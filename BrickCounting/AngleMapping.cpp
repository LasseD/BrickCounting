#include "AngleMapping.h"

#include "Brick.h"
#include "Configuration.hpp"

AngleMapping::AngleMapping(FatSCC const * const sccs, int numScc, const std::vector<IConnectionPair> &cs, const ConfigurationEncoder &encoder) : numAngles(numScc-1), numBricks(0), encoder(encoder) {
  // Simple copying:
  for(int i = 0; i < numScc; ++i) {
    this->sccs[i] = sccs[i];
    numBricks += sccs[i].size;
  }
  // Connection info:
  unsigned int i = 0;
  for(std::vector<IConnectionPair>::const_iterator it = cs.begin(); it != cs.end(); ++it) {
    const IConnectionPair &c = *it;
    const IConnectionPoint &icp1 = c.first;
    const IConnectionPoint &icp2 = c.second;
    points[i++] = icp1;
    points[i++] = icp2;
  }
  
  setupAngleTypes();
  // Set up angle steps:
  for(i = 0; i < numAngles; ++i) {
    switch(angleTypes[i]) {
    case 1:
      angleSteps[i] = STEPS_1;
      break;
    case 2:
      angleSteps[i] = STEPS_2;
      break;
    case 3:
      angleSteps[i] = STEPS_3;
      break;
    }
#ifdef _ANGLE
    std::cout << "Angle type for angle " << i << ": " << angleTypes[i] << " => steps=" << angleSteps[i] << std::endl;
#endif
  }

  if(numAngles > 3) {
    std::cout << "NOT CREATING SML MAPPING FOR NUMANGLES > 3!" << std::endl;
    return;
  }
  
  // sizeMappings:
  sizeMappings = 1;
  for(i = 0; i < numAngles; ++i) {
    sizeMappings *= (2*angleSteps[i]+1);
  }
#ifdef _TRACE
  std::cout << "SML size " << sizeMappings << std::endl;
#endif

  // S, M & L:
  S = new bool[sizeMappings];
  M = new bool[sizeMappings];
  L = new bool[sizeMappings];
}

AngleMapping::~AngleMapping() {
  if(numAngles > 3)
    return;
  delete[] S;
  delete[] M;
  delete[] L;
}

void AngleMapping::setupAngleTypes() {
  // First set up a counter for number of connections to sccs:
  unsigned int fatSccConnectionCounts[6] = {0,0,0,0,0,0}; // scc -> connections to it.
  unsigned int lastConnectedTo[6]; // scc -> last seen connected scc index.
  for(unsigned int i = 0; i < numAngles; ++i) {
    int i1 = points[2*i].first.configurationSCCI;
    int i2 = points[2*i+1].first.configurationSCCI;
    fatSccConnectionCounts[i1]++;
    fatSccConnectionCounts[i2]++;
    lastConnectedTo[i1] = i2;
    lastConnectedTo[i2] = i1;
  }

  // Set up sizes (to be reduced later):
  unsigned int fatSccSizes[6] = {0,0,0,0,0,0}; // scc -> size of it.
  for(unsigned int i = 0; i < numAngles+1; ++i) {
    fatSccSizes[i] = sccs[i].size;
  }

#ifdef _ANGLE
  std::cout << "Angle mappings of size " << numAngles << std::endl;
  std::cout << " Connection counts: " << std::endl;
  for(unsigned int i = 0; i < numAngles+1; ++i) {
    std::cout << "  " << i << "->" << fatSccConnectionCounts[i] << std::endl;
  }
  std::cout << " Last connected to: " << std::endl;
  for(unsigned int i = 0; i < numAngles+1; ++i) {
    std::cout << "  " << i << "->" << lastConnectedTo[i] << std::endl;
  }
  std::cout << " Initial sizes (no merging): " << std::endl;
  for(unsigned int i = 0; i < numAngles+1; ++i) {
    std::cout << "  " << i << "->" << fatSccSizes[i] << std::endl;
  }
#endif

  // Angle types computed based on size of components on side of turn point:
  for(unsigned int i = 0; i < numAngles; ++i)
    angleTypes[i] = 3;
  // Minimize angles connecting leaves:
  for(unsigned int i = 0; i < numAngles*2; ++i) {
    int angleI = i/2;
    int sccI = points[i].first.configurationSCCI;
    if(fatSccConnectionCounts[sccI] == 1)
      angleTypes[angleI] = MIN(fatSccSizes[sccI], numBricks-fatSccSizes[sccI]);
  }
  // Reduce graph by merging leaves into parents:
  for(unsigned int i = 0; i < numAngles+1; ++i) {
    if(fatSccConnectionCounts[i] == 1) {
      int otherI = lastConnectedTo[i];
      fatSccSizes[otherI] += fatSccSizes[i];
      fatSccSizes[i] = 0;
      --fatSccConnectionCounts[otherI];
      fatSccConnectionCounts[i] = 0; // same as --
    }
  }

#ifdef _ANGLE
  std::cout << "After first round of minimizing " << std::endl;
  std::cout << " angleTypes (initially all 3): " << std::endl;
  for(unsigned int i = 0; i < numAngles; ++i) {
    std::cout << "  " << i << "->" << angleTypes[i] << std::endl;
  }
  std::cout << " Current sizes: " << std::endl;
  for(unsigned int i = 0; i < numAngles; ++i) {
    std::cout << "  " << i << "->" << fatSccSizes[i] << std::endl;
  }
  std::cout << " Number of connections: " << std::endl;
  for(unsigned int i = 0; i < numAngles+1; ++i) {
    std::cout << "  " << i << "->" << fatSccConnectionCounts[i] << std::endl;
  }
#endif

  // Perform initial minimizing again:
  for(unsigned int i = 0; i < numAngles*2; ++i) {
    int angleI = i/2;
    int sccI = points[i].first.configurationSCCI;
    if(fatSccConnectionCounts[sccI] == 1)
      angleTypes[angleI] = MIN(fatSccSizes[sccI], numBricks-fatSccSizes[sccI]);
  }

#ifdef _ANGLE
  std::cout << "After second round" << std::endl;
  std::cout << " angleTypes: " << std::endl;
  for(unsigned int i = 0; i < numAngles; ++i) {
    std::cout << "  " << i << "->" << angleTypes[i] << std::endl;
  }
#endif
}

uint64_t AngleMapping::smlIndex(short const * const angleStep) const {
  uint64_t ret = angleStep[0] + angleSteps[0];

  for(unsigned int i = 1; i < numAngles; ++i) {
    int push = 2*angleSteps[i-1]+1;
    ret = (ret * push) + angleStep[i] + angleSteps[i];
  }
  return ret;
}

bool AngleMapping::isRectilinear(short *angleStep) const {
  for(unsigned int i = 0; i < numAngles; ++i) {
    if(angleStep[i] != 0) {
      return false;
    }
  }
  return true;
}

Configuration AngleMapping::getConfiguration(short const * const angleStep) const {
  Configuration c(sccs[0]);
  for(unsigned int i = 0; i < numAngles; ++i) {
    const IConnectionPoint &ip1 = points[2*i];
    const IConnectionPoint &ip2 = points[2*i+1];
    const Angle angle(angleStep[i], angleSteps[i]);
    const Connection cc(ip1, ip2, angle);
    assert(ip2.P1.configurationSCCI != 0);
    c.add(sccs[ip2.P1.configurationSCCI], ip2.P1.configurationSCCI, cc);
  }
  return c;
}

void AngleMapping::evalSML(unsigned int angleI, short *angleStep) {
  if(angleI < numAngles) {
    short steps = angleSteps[angleI];
    for(short i = -steps; i <= steps; ++i) {
      angleStep[angleI] = i;
      evalSML(angleI+1, angleStep);
    }
    return;
  }

  uint64_t i = smlIndex(angleStep);
  Configuration c = getConfiguration(angleStep);

  // Investigate!
  std::vector<IConnectionPair> found; // Currently ignored.
  S[i] = c.isRealizable<-1,-1>(found);
  M[i] = c.isRealizable<0,0>(found);
  L[i] = c.isRealizable<1,1>(found);

#ifdef _TRACE
  std::cout << "EVAL SML(" << angleStep[0];
  for(unsigned int j = 1; j < numAngles; ++j) {
    std::cout << "," << angleStep[j];
  }
  std::cout << ") = " << S[i] <<" "<< M[i] <<" "<< L[i] << std::endl;
#endif
}

void AngleMapping::findIslands(unsigned int angleI, short *angleStep, std::vector<SIsland> &sIslands) {
  if(angleI < numAngles) {
    short steps = angleSteps[angleI];
    for(short i = -steps; i <= steps; ++i) {
      angleStep[angleI] = i;
      findIslands(angleI+1, angleStep, sIslands);
    }
    return;
  }
  uint64_t index = smlIndex(angleStep);
  if(S[index])
    sIslands.push_back(SIsland(this, angleStep));
}

void AngleMapping::findExtremeConfigurations(unsigned int angleI, short *angleStep, bool allZero, std::set<Encoding> &rect, std::set<Encoding> &nonRect, std::vector<Configuration> &toLdr) {
  if(angleI < numAngles) {
    angleStep[angleI] = -angleSteps[angleI];
    findExtremeConfigurations(angleI+1, angleStep, false, rect, nonRect, toLdr);

    angleStep[angleI] = angleSteps[angleI];
    findExtremeConfigurations(angleI+1, angleStep, false, rect, nonRect, toLdr);

    // Also try no angle - just in case:
    angleStep[angleI] = 0;
    findExtremeConfigurations(angleI+1, angleStep, allZero, rect, nonRect, toLdr);

    return;
  }
  if(allZero)
    return;

  Configuration c = getConfiguration(angleStep);
  std::vector<IConnectionPair> found;
  if(!c.isRealizable<0,0>(found))
    return; // Not possible to build.
  Encoding encoded = encoder.encode(found);
  if(rect.find(encoded) != rect.end())
    return; // Already known as rect.
  if(nonRect.find(encoded) != nonRect.end())
    return; // Already known as nonRect.
  nonRect.insert(encoded);
  if(found.size() != numAngles)
    toLdr.push_back(c); // cyclic!
}

void AngleMapping::findNewExtremeConfigurations(std::set<Encoding> &rect, std::set<Encoding> &nonRect, std::vector<Configuration> &toLdr) {
  short angleStep[5] = {0,0,0,0,0};

  //Initially try rectilinear:
  Configuration rectilinear = getConfiguration(angleStep);
  std::vector<IConnectionPair> found; // Currently ignored.
  if(rectilinear.isRealizable<0,0>(found)) { // Rectilinear:
    assert(found.size() >= numAngles);
    Encoding encoded = encoder.encode(found);
    if(rect.find(encoded) == rect.end()) {
      rect.insert(encoded);
    }
  }
  findExtremeConfigurations(0, angleStep, true, rect, nonRect, toLdr);
}

void AngleMapping::findNewConfigurations(std::set<Encoding> &rect, std::set<Encoding> &nonRect, std::vector<Configuration> &toLdr, counter &problematic) {
  if(numAngles > 3) { // Can't really do this for 4+ angles... sorry.
    findNewExtremeConfigurations(rect, nonRect, toLdr);
    return;
  }

  // Evaluate SML:
  short angleStep[5] = {0,0,0,0,0};
  rectilinearIndex = smlIndex(angleStep);
  evalSML(0, angleStep);

  // Find islands:
  //std::cout << "Finding islands" << std::endl;
  std::vector<SIsland> sIslands;
  findIslands(0, angleStep, sIslands);

  /* Perform analysis:
  Walk through S. Once a 1 is found in S, expand to whole region and report M and L islands.
  - If no M island: Report problematic. Don't count.  
  - If more than one M-island, output problematic. Count all islands.
  - For each M island: Walk through island to find L-islands.
   - If no L-island. Report problematic. Count 1.
   - If more than one L-island: Report problematic. Still only count 1.
  */
  if(sIslands.size() == 0)
    return;

  //std::cout << "Investigating islands" << std::endl;
  for(std::vector<SIsland>::const_iterator itS = sIslands.begin(); itS != sIslands.end(); ++itS) {
    const SIsland &sIsland = *itS;
    //std::cout << " S island at " << smlIndex(sIsland.representative) << std::endl;

    if(sIsland.mIslands.size() == 0) { // No M-islands inside => problematic. No count.
      std::cout << " Problematic: No M islands! " << smlIndex(sIsland.representative) << std::endl;
      ++problematic;
      Configuration c = getConfiguration(sIsland.representative);
      toLdr.push_back(c);
      continue;
    }
    bool multipleMIslands = sIsland.mIslands.size() > 1;
    for(std::vector<MIsland>::const_iterator itM = sIsland.mIslands.begin(); itM != sIsland.mIslands.end(); ++itM) {
      const MIsland &mIsland = *itM;
      //std::cout << " Investigating M islands. Rectilinear: " << mIsland.rectilinear << std::endl;
      Configuration c = getConfiguration(mIsland.representative);
      std::vector<IConnectionPair> found; // Currently ignored.
      c.isRealizable<0,0>(found);
        Encoding encoded = encoder.encode(found);

      // Multiple M-islands inside => problematic. Count.
      // No L-islands => problematic, but still count.
      if(multipleMIslands || mIsland.lIslands.size() != 1) { 
        std::cout << " Problematic: Multiple M islands or |islands|!=1: " << smlIndex(mIsland.representative) << std::endl;
        for(std::vector<LIsland>::const_iterator itL = mIsland.lIslands.begin(); itL != mIsland.lIslands.end(); ++itL) {
          const LIsland &lIsland = *itL;
          std::cout << "  L island: " << smlIndex(lIsland.representative) << std::endl;
        }
        ++problematic;
        toLdr.push_back(c);
      }

      if(mIsland.rectilinear) {
        if(rect.find(encoded) == rect.end()) {
          rect.insert(encoded);
        }
      }
      else {
        if(nonRect.find(encoded) == nonRect.end()) {
          nonRect.insert(encoded);
        }
      }
    }
  }
}
