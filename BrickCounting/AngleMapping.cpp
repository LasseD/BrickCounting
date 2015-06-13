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
  //std::cout << "SML size " << sizeMappings;
  S = new bool[sizeMappings];
  M = new bool[sizeMappings];
  L = new bool[sizeMappings];
  //std::cout << " OK" << std::endl;
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

uint64_t AngleMapping::smlIndex(unsigned short const * const angleStep) const {
  uint64_t ret = angleStep[0];

  for(unsigned int i = 1; i < numAngles; ++i) {
    int push = 2*angleSteps[i] + 1;
    ret = (ret * push) + angleStep[i];
  }
  return ret;
}

bool AngleMapping::isRectilinear(unsigned short *angleStep) const {
  for(unsigned int i = 0; i < numAngles; ++i) {
    if(angleStep[i] != 0) {
      return false;
    }
  }
  return true;
}

Configuration AngleMapping::getConfiguration(unsigned short const * const angleStep) const {
  Configuration c(sccs[0]);
  for(unsigned int i = 0; i < numAngles; ++i) {
    const IConnectionPoint &ip1 = points[2*i];
    const IConnectionPoint &ip2 = points[2*i+1];
    const Angle angle((short)angleStep[i]-(short)angleSteps[i], angleSteps[i]);
    const Connection cc(ip1, ip2, angle);
    assert(ip2.P1.configurationSCCI != 0);
    c.add(sccs[ip2.P1.configurationSCCI], ip2.P1.configurationSCCI, cc);
  }
  return c;
}

std::vector<Connection> AngleMapping::getConfigurationConnections(unsigned short const * const angleStep) const {
  std::vector<Connection> res;
  for(unsigned int i = 0; i < numAngles; ++i) {
    const IConnectionPoint &ip1 = points[2*i];
    const IConnectionPoint &ip2 = points[2*i+1];
    const Angle angle((short)angleStep[i]-(short)angleSteps[i], angleSteps[i]);
    res.push_back(Connection(ip1, ip2, angle));
  }
  return res;
}

void AngleMapping::evalSML(unsigned int angleI, unsigned short *angleStep) {
  if(angleI < numAngles) {
    unsigned short steps = 2*angleSteps[angleI]+1;
    for(unsigned short i = 0; i < steps; ++i) {
      angleStep[angleI] = i;
      evalSML(angleI+1, angleStep);
    }
    return;
  }

  Configuration c = getConfiguration(angleStep);


  // Investigate!
  uint64_t i = smlIndex(angleStep);
  assert(i < sizeMappings);

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

void AngleMapping::findIslands(std::multimap<Encoding, SIsland> &sIslands, std::set<Encoding> &keys) {
  // Add all S-islands:
  for(unsigned int i = 0; i < ufS->numReducedUnions; ++i) {
    unsigned short rep[5];
    uint16_t unionI = ufS->reducedUnions[i];
    ufS->getRepresentative(unionI, rep);
    assert(ufS->get(rep) == unionI);
    assert(S[ufS->indexOf(rep)]);

    Configuration c = getConfiguration(rep);
    std::vector<IConnectionPair> found;
    c.isRealizable<0,0>(found);
    Encoding encoding = encoder.encode(found);
    sIslands.insert(std::make_pair(encoding, SIsland(this, unionI, rep)));
    if(keys.find(encoding) == keys.end())
      keys.insert(encoding);
  }
}

void AngleMapping::findExtremeConfigurations(unsigned int angleI, unsigned short *angleStep, bool allZero, std::set<Encoding> &rect, std::set<Encoding> &nonRect, std::vector<std::vector<Connection> > &toLdr) {
  if(angleI < numAngles) {
    angleStep[angleI] = 0;
    findExtremeConfigurations(angleI+1, angleStep, false, rect, nonRect, toLdr);

    angleStep[angleI] = 2*angleSteps[angleI];
    findExtremeConfigurations(angleI+1, angleStep, false, rect, nonRect, toLdr);

    // Also try no angle - just in case:
    angleStep[angleI] = angleSteps[angleI]-1;
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
    toLdr.push_back(getConfigurationConnections(angleStep)); // cyclic!
}

void AngleMapping::findNewExtremeConfigurations(std::set<Encoding> &rect, std::set<Encoding> &nonRect, std::vector<std::vector<Connection> > &toLdr) {
  unsigned short angleStep[5] = {0,0,0,0,0};

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

void AngleMapping::reportProblematic(unsigned short const * const angleStep, int mIslandI, int mIslandTotal, int lIslandTotal, std::vector<std::vector<Connection> > &toLdr) const {
  Configuration c = getConfiguration(angleStep);
  std::vector<Connection> cc = getConfigurationConnections(angleStep);
  toLdr.push_back(cc);
  // Report
  std::cout << " Configuration requires manual verification!" <<std::endl;
  std::cout << "  File: ";
  encoder.writeFileName(std::cout, cc);
  std::cout << std::endl;
  if(mIslandTotal > 0)
    std::cout << "  This configuration represents M-island " << (mIslandI+1) << "/" << mIslandTotal << ". There are " << lIslandTotal << " L-islands in this M-island" << std::endl;
  else
    std::cout << "  S islands without M-islands inside!" << std::endl;        
  std::cout << "  Configuration: " << c << std::endl;
  std::cout << std::endl;
}

void AngleMapping::findNewConfigurations(std::set<Encoding> &rect, std::set<Encoding> &nonRect, 
                                         std::vector<std::vector<Connection> > &toLdr, std::vector<Configuration> &nrcToPrint, std::vector<Configuration> &modelsToPrint, 
                                         counter &models, counter &problematic) {
  if(numAngles > 3) { // Can't really do this for 4+ angles... sorry.
    findNewExtremeConfigurations(rect, nonRect, toLdr);
    return;
  }

  // Evaluate SML:
  unsigned short angleStep[5] = {0,0,0,0,0};
  rectilinearIndex = smlIndex(angleStep);

  //std::cout << " smlIndex OK" << std::endl;

  evalSML(0, angleStep);

  //std::cout << " creating sufs" << std::endl;
  // Evaluate union-find:
  unsigned short sizes[5];
  for(unsigned int i = 0; i < numAngles; ++i) {
    sizes[i] = 2*angleSteps[i]+1;
  }
  ufS = new SimpleUnionFind(numAngles, sizes, S);
  ufM = new SimpleUnionFind(numAngles, sizes, M);
  ufL = new SimpleUnionFind(numAngles, sizes, L);

  //std::cout << " finding islands" << std::endl;
  // Find islands:
  std::multimap<Encoding, SIsland> sIslands;
  std::set<Encoding> sIslandKeys;
  findIslands(sIslands, sIslandKeys);

  /* Perform analysis:
  Walk through S. Once a 1 is found in S, expand to whole region and report M and L islands.
  - If no M island: Report problematic. Don't count.  
  - If more than one M-island, output problematic. Count all islands.
  - For each M island: Walk through island to find L-islands.
   - If no L-island. Report problematic. Count 1.
   - If more than one L-island: Report problematic. Still only count 1.
  */
  if(sIslandKeys.size() == 0) {
    //std::cout << "No islands!" << std::endl;
    return;
  }

  for(std::set<Encoding>::const_iterator itKeys = sIslandKeys.begin(); itKeys != sIslandKeys.end(); ++itKeys) {
    Encoding encoding = *itKeys;
    if(rect.find(encoding) != rect.end() || nonRect.find(encoding) != nonRect.end()) {
      //std::cout << "Already known encoding: " << encoding.first << std::endl;
      continue; // Already found!
    }

    std::pair<std::multimap<Encoding, SIsland>::const_iterator,std::multimap<Encoding, SIsland>::const_iterator> r = sIslands.equal_range(encoding);
    bool rangeIsRect = false;
    std::vector<Configuration> allNrcInRange;

    //std::cout << " Investigating islands" << std::endl;
    for(std::multimap<Encoding, SIsland>::const_iterator itS = r.first; itS != r.second; ++itS) {
      const SIsland &sIsland = itS->second;
      //std::cout << " S island at " << smlIndex(sIsland.representative) << std::endl;

      if(sIsland.mIslands.size() == 0) { // No M-islands inside => problematic. No count.
        reportProblematic(sIsland.representative, 0, 0, 0, toLdr);
        ++problematic;
        continue;
      }

      int mIslandI = 0;
      for(std::vector<MIsland>::const_iterator itM = sIsland.mIslands.begin(); itM != sIsland.mIslands.end(); ++itM, ++mIslandI) {
        const MIsland &mIsland = *itM;

        Configuration c = getConfiguration(mIsland.representative);
        if(mIsland.rectilinear) {
          rangeIsRect = true;
        }
        else {
          allNrcInRange.push_back(c);
        }
        //std::cout << " Investigating M islands. Rectilinear: " << mIsland.rectilinear << std::endl;

        // Multiple M-islands inside => problematic. Count only this M-island.
        // No L-islands => problematic, but still count.
        if(sIsland.mIslands.size() != 1 || mIsland.lIslands.size() != 1) { 
          ++problematic;
          reportProblematic(mIsland.representative, mIslandI, (int)sIsland.mIslands.size(), (int)mIsland.lIslands.size(), toLdr);
        }
      }
    }

    // Update output:
    if(!allNrcInRange.empty()) {
      nrcToPrint.push_back(*allNrcInRange.begin());
      for(std::vector<Configuration>::const_iterator itR = allNrcInRange.begin(); itR != allNrcInRange.end(); ++itR) {
        modelsToPrint.push_back(*itR);
      }
      models+=allNrcInRange.size();
    }

    if(rangeIsRect)
      rect.insert(encoding);
    else
      nonRect.insert(encoding);
  }

  // Cleanup:
  delete ufS;
  delete ufM;
  delete ufL;
}
