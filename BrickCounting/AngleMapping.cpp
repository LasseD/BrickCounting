#include "AngleMapping.h"

#include "Brick.h"
#include "Configuration.hpp"
#include "TurningSingleBrick.h"
#include <time.h>

AngleMapping::AngleMapping(FatSCC const * const sccs, int numScc, const std::vector<IConnectionPair> &cs, const ConfigurationEncoder &encoder, std::ofstream &os) : numAngles(numScc-1), numBricks(0), encoder(encoder), os(os) {
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
    case 0:
      angleSteps[i] = STEPS_0;
      break;
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

  // Finally. Set angle types of locked connections to type 0:
  // First set the ones locked directly on the SCC:
  for(unsigned int i = 0; i < 2*numAngles; ++i) {
    int angleI = i/2;
    int sccI = points[i].first.configurationSCCI;
    const ConnectionPoint &p = points[i].second;
    if(sccs[sccI].angleLocked(p)) {
      angleTypes[angleI] = 0;
      //std::cout << "Locked angle on " << sccs[sccI] << " vs " << p << std::endl;
    }
  }
  // Secondly, lock all touching connections:
  for(unsigned int i = 0; i < 2*numAngles; ++i) {
    int angleI = i/2;
    int sccI = points[i].first.configurationSCCI;
    const ConnectionPoint &pi = points[i].second;

    for(unsigned int j = i+1; j < 2*numAngles; ++j) {
      int angleJ = j/2;
      int sccJ = points[j].first.configurationSCCI;
      if(sccI != sccJ)
	continue;
      const ConnectionPoint &pj = points[j].second;
      //std::cout << "Investigating " << pi << " & " << pj << std::endl;
      if(pi.angleLocks(pj)) {
	angleTypes[angleI] = 0;
	angleTypes[angleJ] = 0;	
	//std::cout << "Locked angles " << pi << " & " << pj << std::endl;
      }
    }
  }
}

/*
Deprecated: Too slow.
uint64_t AngleMapping::smlIndex(const Position &p) const {
  uint64_t ret = p.p[0];

  for(unsigned int i = 1; i < numAngles; ++i) {
    const int push = 2*angleSteps[i] + 1;
    ret = (ret * push) + p.p[i];
  }
  return ret;
}*/

Configuration AngleMapping::getConfiguration(const Position &p) const {
  Configuration c(sccs[0]);
  for(unsigned int i = 0; i < numAngles; ++i) {
    const IConnectionPoint &ip1 = points[2*i];
    const IConnectionPoint &ip2 = points[2*i+1];
    const Angle angle((short)p.p[i]-(short)angleSteps[i], angleSteps[i] == 0 ? 1 : angleSteps[i]);
    const Connection cc(ip1, ip2, angle);
    assert(ip2.P1.configurationSCCI != 0);
    c.add(sccs[ip2.P1.configurationSCCI], ip2.P1.configurationSCCI, cc);
  }
  return c;
}

Configuration AngleMapping::getConfiguration(const Configuration &baseConfiguration, int angleI, unsigned short angleStep) const {
  Configuration c(baseConfiguration);

  const IConnectionPoint &ip1 = points[2*angleI];
  const IConnectionPoint &ip2 = points[2*angleI+1];
  const Angle angle((short)angleStep-(short)angleSteps[angleI], angleSteps[angleI] == 0 ? 1 : angleSteps[angleI]);
  const Connection cc(ip1, ip2, angle);
  assert(ip2.P1.configurationSCCI != 0);
  c.add(sccs[ip2.P1.configurationSCCI], ip2.first.configurationSCCI, cc);

  return c;
}

std::vector<Connection> AngleMapping::getConfigurationConnections(const Position &p) const {
  std::vector<Connection> res;
  for(unsigned int i = 0; i < numAngles; ++i) {
    const IConnectionPoint &ip1 = points[2*i];
    const IConnectionPoint &ip2 = points[2*i+1];
    const Angle angle((short)p.p[i]-(short)angleSteps[i], angleSteps[i]);
    res.push_back(Connection(ip1, ip2, angle));
  }
  return res;
}

/*
Evaluate the SML matrix.
Parameters:
- angleI: Used as recursion index in angles. Initialize with 0. 
- smlI: Index into SML now being build dynamically, rather than computed using smlIndex(position). Initially 0.
- c: Configuration being constructed dynamically. Initially containing sccs[0].
- possibleCollisions: possible connections pre-computed to be used in next step. Remember to initialize!
 */
void AngleMapping::evalSML(unsigned int angleI, uint64_t smlI, const Configuration &c, bool noS, bool noM, bool noL) {
  // Update smlI:
  assert(smlI < sizeMappings);
  const unsigned short steps = 2*angleSteps[angleI]+1;
  smlI *= steps;

  // Find possible collisions:
  const IConnectionPoint &ip1 = points[2*angleI];
  const IConnectionPoint &ip2 = points[2*angleI+1];
  unsigned int ip2I = ip2.first.configurationSCCI;
  std::vector<int> possibleCollisions = c.getPossibleCollisions(sccs[ip2I], ip1.first);

  // Recursion:
  if(angleI < numAngles-1) {
    for(unsigned short i = 0; i < steps; ++i) {
      Configuration c2 = getConfiguration(c, angleI, i);
      
      noS = noS && c2.isRealizable<-1>(possibleCollisions, sccs[ip2I].size);
      noM = noM && c2.isRealizable< 0>(possibleCollisions, sccs[ip2I].size);
      noL = noL && c2.isRealizable< 1>(possibleCollisions, sccs[ip2I].size);

      evalSML(angleI+1, smlI + i, c2, noS, noM, noL);
    }
    return;
  }

  // End of recursion:
  assert(angleI == numAngles-1);

  // Speed up for noSML:
  if(noS) for(unsigned int i = 0; i < steps; ++i) S[smlI+i] = false;
  if(noM) for(unsigned int i = 0; i < steps; ++i) M[smlI+i] = false;
  if(noL) for(unsigned int i = 0; i < steps; ++i) L[smlI+i] = false;
  bool sDone = noS;
  bool mDone = noM;
  bool lDone = noL;
  if(sDone && mDone && lDone)
    return;

  // Speed up using TSB:
  if(sccs[numAngles].size == 1 && angleTypes[angleI] == 1) {
    assert(steps == 407);
#ifdef _TRACE
    std::cout << "Using TSB to check quickly." << std::endl;
    std::cout << "Angle types:" << std::endl;
    for(unsigned int i = 0; i < numAngles; ++i)
      std::cout << " " << i << ": " << angleTypes[i] << std::endl;      
#endif
    const IConnectionPair icp(ip1,ip2);
      
    TurningSingleBrickInvestigator tsbInvestigator(c, ip2I, icp);

    // First check quick clear:
    if(!sDone && tsbInvestigator.isClear<-1>(possibleCollisions)) {
      for(unsigned int i = 0; i < steps; ++i) 
	S[smlI+i] = true;
      for(unsigned short i = 0; i < steps; ++i) {
	Configuration c2 = getConfiguration(c, angleI, i);
	if(S[smlI+i] != c2.isRealizable<-1>(possibleCollisions, sccs[numAngles].size)) {
	  std::cerr << "Assertion error on S[" << i << "]=" << S[smlI+i] << " configuration: " << c2 << std::endl;	 
	  LDRPrinterHandler h;
	  h.add(&c2);
	  h.print("assertion_fail");
	  assert(false);
	}
      }//*/
      sDone = true;
    }
    if(!mDone && tsbInvestigator.isClear<0>(possibleCollisions)) {
      for(unsigned int i = 0; i < steps; ++i) 
	M[smlI+i] = true;
      for(unsigned short i = 0; i < steps; ++i) {
	Configuration c2 = getConfiguration(c, angleI, i);
	assert(M[smlI+i] == c2.isRealizable<0>(possibleCollisions, sccs[numAngles].size));
      }//*/
      mDone = true;
    }
    if(!lDone && tsbInvestigator.isClear<1>(possibleCollisions)) {
      for(unsigned int i = 0; i < steps; ++i) 
	L[smlI+i] = true;
      for(unsigned short i = 0; i < steps; ++i) {
	Configuration c2 = getConfiguration(c, angleI, i);
	assert(L[smlI+i] == c2.isRealizable<1>(possibleCollisions, sccs[numAngles].size));
      }//*/
      lDone = true;
    }
    if(sDone && mDone && lDone)
      return;

    // Check using TSB:
    IntervalList l;
    if(!sDone && tsbInvestigator.allowableAnglesForBricks<-1>(possibleCollisions, l)) {
      math::intervalToArray(Interval(-MAX_ANGLE_RADIANS,MAX_ANGLE_RADIANS), l, &S[smlI], steps);
      std::cout << "Investigating S-mapping vs " << l << std::endl;
      for(unsigned short i = 0; i < steps; ++i) {
	Configuration c2 = getConfiguration(c, angleI, i);
	if(S[smlI+i] != c2.isRealizable<-1>(possibleCollisions, sccs[numAngles].size)) {
	  std::cerr << "Assertion error on S[" << i << "]=" << S[smlI+i] << " vs allowableAnglesForBricks. Configuration: " << c2 << std::endl;	 
	  LDRPrinterHandler h;
	  h.add(&c2);
	  h.print("assertion_fail");

	  for(unsigned short j = 0; j < steps; ++j) {
	    std::cout << (S[smlI+i] ? "X" : "O");
	  }
	  std::cout << std::endl;
	  for(unsigned short j = 0; j < steps; ++j) {
	    Configuration c3 = getConfiguration(c, angleI, j);
	    std::cout << (c3.isRealizable<-1>(possibleCollisions, sccs[numAngles].size) ? "X" : "O");
	  }
	  std::cout << std::endl;
	  assert(false);
	}
	//assert(S[smlI+i] == c2.isRealizable<-1>(possibleCollisions, sccs[numAngles].size));
      }//*/
      sDone = true;
    }
    l.clear();
    if(!mDone && tsbInvestigator.allowableAnglesForBricks<0>(possibleCollisions, l)) {
      math::intervalToArray(Interval(-MAX_ANGLE_RADIANS,MAX_ANGLE_RADIANS), l, &M[smlI], steps);
      std::cout << "Investigating M-mapping vs " << l << std::endl;
      for(unsigned short i = 0; i < steps; ++i) {
	Configuration c2 = getConfiguration(c, angleI, i);
	assert(M[smlI+i] == c2.isRealizable<0>(possibleCollisions, sccs[numAngles].size));
      }//*/
      mDone = true;
    }
    l.clear();
    if(!lDone && tsbInvestigator.allowableAnglesForBricks<1>(possibleCollisions, l)) {
      math::intervalToArray(Interval(-MAX_ANGLE_RADIANS,MAX_ANGLE_RADIANS), l, &L[smlI], steps);
      std::cout << "Investigating L-mapping vs " << l << std::endl;
      for(unsigned short i = 0; i < steps; ++i) {
	Configuration c2 = getConfiguration(c, angleI, i);
	assert(L[smlI+i] == c2.isRealizable<1>(possibleCollisions, sccs[numAngles].size));
      }//*/
      lDone = true;
    }
    if(sDone && mDone && lDone)
      return;
  }
  
  /* // Uncomment for quicker processing time.
  for(unsigned int i = 0; i < steps; ++i) S[smlI+i] = false;
  for(unsigned int i = 0; i < steps; ++i) M[smlI+i] = false;
  for(unsigned int i = 0; i < steps; ++i) L[smlI+i] = false;
  return;//*/

  for(unsigned short i = 0; i < steps; ++i) {
    Configuration c2 = getConfiguration(c, angleI, i);
    if(!sDone)
      S[smlI+i] = c2.isRealizable<-1>(possibleCollisions, sccs[numAngles].size);
    if(!mDone)
      M[smlI+i] = c2.isRealizable< 0>(possibleCollisions, sccs[numAngles].size);
    if(!lDone)
      L[smlI+i] = c2.isRealizable< 1>(possibleCollisions, sccs[numAngles].size);
  }
}

void AngleMapping::findIslands(std::multimap<Encoding, SIsland> &sIslands, std::set<Encoding> &keys) {
  // Add all S-islands:
  for(unsigned int i = 0; i < ufS->numReducedUnions; ++i) {
    Position rep;
    const uint32_t unionI = ufS->reducedUnions[i];
    ufS->getRepresentative(unionI, rep);
    assert(ufS->get(rep) == unionI);
    assert(S[ufS->indexOf(rep)]);

    Configuration c = getConfiguration(rep);
    std::vector<IConnectionPair> found;
    c.isRealizable<-1>(found);
    Encoding encoding = encoder.encode(found);
    sIslands.insert(std::make_pair(encoding, SIsland(this, unionI, rep)));
    if(keys.find(encoding) == keys.end())
      keys.insert(encoding);
  }
}

void AngleMapping::findExtremeConfigurations(unsigned int angleI, Position &p, bool allZero, std::set<Encoding> &rect, std::set<Encoding> &nonRect, std::vector<std::vector<Connection> > &toLdr) {
  if(angleI < numAngles) {
    p.p[angleI] = 0;
    findExtremeConfigurations(angleI+1, p, false, rect, nonRect, toLdr);

    p.p[angleI] = 2*angleSteps[angleI];
    findExtremeConfigurations(angleI+1, p, false, rect, nonRect, toLdr);

    // Also try no angle - just in case:
    p.p[angleI] = angleSteps[angleI]-1;
    findExtremeConfigurations(angleI+1, p, allZero, rect, nonRect, toLdr);

    return;
  }
  if(allZero)
    return;

  Configuration c = getConfiguration(p);
  std::vector<IConnectionPair> found;
  if(!c.isRealizable<0>(found))
    return; // Not possible to build.
  Encoding encoded = encoder.encode(found);
  if(rect.find(encoded) != rect.end())
    return; // Already known as rect.
  if(nonRect.find(encoded) != nonRect.end())
    return; // Already known as nonRect.
  nonRect.insert(encoded);
  if(found.size() != numAngles)
    toLdr.push_back(getConfigurationConnections(p)); // cyclic!
}

void AngleMapping::findNewExtremeConfigurations(std::set<Encoding> &rect, std::set<Encoding> &nonRect, std::vector<std::vector<Connection> > &toLdr) {
  Position p;
  p.init();

  //Initially try rectilinear:
  Configuration rectilinear = getConfiguration(p);
  std::vector<IConnectionPair> found; // Currently ignored.
  if(rectilinear.isRealizable<0>(found)) { // Rectilinear:
    assert(found.size() >= numAngles);
    Encoding encoded = encoder.encode(found);
    if(rect.find(encoded) == rect.end()) {
      rect.insert(encoded);
    }
  }
  findExtremeConfigurations(0, p, true, rect, nonRect, toLdr);
}

void AngleMapping::reportProblematic(const Position &p, int mIslandI, int mIslandTotal, int lIslandTotal, std::vector<std::vector<Connection> > &toLdr) const {
  Configuration c = getConfiguration(p);
  std::vector<Connection> cc = getConfigurationConnections(p);
  
  toLdr.push_back(cc);
  // Report
  os << " Configuration requires manual verification!" <<std::endl;

  // Special case: If this configuration contains no L-islands, but does contain a loop, then it is most likely OK:
  if(mIslandTotal == 1 && lIslandTotal == 0 && cc.size() > numAngles) {
    os << "  Special case for manual verification: Single M-island in S-island. With a loop, but without an L-island." << std::endl;
  }

  os << "  File: ";
  encoder.writeFileName(os, cc);
  os << std::endl;
  os << "  Angles: " << std::endl;
  for(unsigned int i = 0; i < numAngles; ++i) {
    double radian = Angle(p.p[i]-angleSteps[i], angleSteps[i]).toRadians();
    os << "   " << (i+1) << ": step " << p.p[i] << "/" << (2*angleSteps[i]+1) << ", fraction " << (p.p[i]-angleSteps[i]) << "/" << angleSteps[i] << ", radian " << radian << std::endl;
  }
  if(mIslandTotal > 0)
    os << "  This configuration represents M-island " << (mIslandI+1) << "/" << mIslandTotal << ". There are " << lIslandTotal << " L-islands in this M-island" << std::endl;
  else
    os << "  S islands without M-islands inside!" << std::endl;        
  os << "  Configuration: " << c << std::endl;
  os << std::endl;
}

void AngleMapping::findNewConfigurations(std::set<Encoding> &rect, std::set<Encoding> &nonRect, 
                                         std::vector<std::vector<Connection> > &toLdr, std::vector<Configuration> &nrcToPrint, std::vector<Configuration> &modelsToPrint, 
                                         counter &models, counter &problematic) {
  time_t startTime, endTime;
  time(&startTime);

  // Compute rectilinear index:
  rectilinearIndex = angleSteps[0];
  for(unsigned int i = 1; i < numAngles; ++i) {
    const int push = 2*angleSteps[i] + 1;
    rectilinearIndex = (rectilinearIndex * push) + angleSteps[i];
  }

  // Evaluate SML:
  Configuration c(sccs[0]);
  evalSML(0, 0, c, false, false, false);

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
    return;
  }

  for(std::set<Encoding>::const_iterator itKeys = sIslandKeys.begin(); itKeys != sIslandKeys.end(); ++itKeys) {
    Encoding encoding = *itKeys;
    if(rect.find(encoding) != rect.end() || nonRect.find(encoding) != nonRect.end()) {
      continue; // Already found!
    }

    std::pair<std::multimap<Encoding, SIsland>::const_iterator,std::multimap<Encoding, SIsland>::const_iterator> r = sIslands.equal_range(encoding);
    bool rangeIsRect = false;
    std::vector<Configuration> allNrcInRange;

    for(std::multimap<Encoding, SIsland>::const_iterator itS = r.first; itS != r.second; ++itS) {
      const SIsland &sIsland = itS->second;

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
	  assert(!rangeIsRect);
          rangeIsRect = true;
        }
        else {
          allNrcInRange.push_back(c);
        }

        // Multiple M-islands inside => problematic. Count only this M-island.
        // No L-islands => problematic, but still count.
        if(sIsland.mIslands.size() != 1 || mIsland.lIslands > 1) { 
          ++problematic;
          reportProblematic(mIsland.representative, mIslandI, (int)sIsland.mIslands.size(), mIsland.lIslands, toLdr);
        }
      }
    }

    // Update output:
    if(!allNrcInRange.empty()) {
      nrcToPrint.push_back(*allNrcInRange.begin());
      if(allNrcInRange.size() > 1)
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

  time(&endTime);
  double seconds = difftime(endTime,startTime);
  if(seconds > 2)
    std::cout << "Angle map finding performed in " << seconds << " seconds." << std::endl;
}
