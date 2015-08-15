#include "AngleMapping.h"

#include "Brick.h"
#include "Configuration.hpp"
#include "TurningSingleBrick.h"
#include <time.h>
#include <sstream>

namespace math {
  void intervalToArray(const IntervalList &l, bool *array, unsigned int sizeArray) {
    IntervalList::const_iterator it = l.begin();
    for(unsigned int i = 0; i < sizeArray; ++i) {
      double v = -MAX_ANGLE_RADIANS + ((MAX_ANGLE_RADIANS+MAX_ANGLE_RADIANS)*i)/((double)sizeArray-1);
      if(it == l.end() || v < it->first) {
        array[i] = false; // sizeArray-1-i
      }
      else if(v > it->second) {
        array[i] = false; // sizeArray-1-i
        ++it;
      }
      else
        array[i] = true; // sizeArray-1-i
    }
  }
}

AngleMapping::AngleMapping(FatSCC const * const sccs, int numScc, const std::vector<IConnectionPair> &cs, const ConfigurationEncoder &encoder, std::ofstream &os) : numAngles(numScc-1), numBricks(0), encoder(encoder), os(os) {
  // Boosts:
  for(int i = 0; i < BOOST_STAGES; ++i)
    boosts[i] = 0;
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
  }

  // sizeMappings:
  sizeMappings = 1;
  for(i = 0; i < numAngles-1; ++i) {
    sizeMappings *= (2*angleSteps[i]+1);
  }
#ifdef _TRACE
  std::cout << "SML size " << sizeMappings << std::endl;
#endif

  // S, M & L:
  SS = new math::IntervalListVector(sizeMappings, MAX_LOAD_FACTOR);
  MM = new math::IntervalListVector(sizeMappings, MAX_LOAD_FACTOR);
  LL = new math::IntervalListVector(sizeMappings, MAX_LOAD_FACTOR);
}

AngleMapping::~AngleMapping() {
  delete SS;
  delete MM;
  delete LL;
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

  // Perform initial minimizing again:
  for(unsigned int i = 0; i < numAngles*2; ++i) {
    int angleI = i/2;
    int sccI = points[i].first.configurationSCCI;
    if(fatSccConnectionCounts[sccI] == 1)
      angleTypes[angleI] = MIN(fatSccSizes[sccI], numBricks-fatSccSizes[sccI]);
  }

  // Finally. Set angle types of locked connections to type 0:
  // First set the ones locked directly on the SCC:
  for(unsigned int i = 0; i < 2*numAngles; ++i) {
    int angleI = i/2;
    int sccI = points[i].first.configurationSCCI;
    const ConnectionPoint &p = points[i].second;
    if(sccs[sccI].angleLocked(p)) {
      angleTypes[angleI] = 0;
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
      if(pi.angleLocks(pj)) {
        angleTypes[angleI] = 0;
        angleTypes[angleJ] = 0;	
      }
    }
  }
}

/*
Configuration AngleMapping::getConfiguration(const Position &p) const {
  Configuration c(sccs[0]);
  for(unsigned int i = 0; i < numAngles; ++i) {
    const IConnectionPoint &ip1 = points[2*i];
    const IConnectionPoint &ip2 = points[2*i+1];
    const StepAngle angle((short)p.p[i]-(short)angleSteps[i], angleSteps[i] == 0 ? 1 : angleSteps[i]);
    const Connection cc(ip1, ip2, angle);
    assert(ip2.P1.configurationSCCI != 0);
    c.add(sccs[ip2.P1.configurationSCCI], ip2.P1.configurationSCCI, cc);
  }
  return c;
}*/

Configuration AngleMapping::getConfiguration(const MixedPosition &p) const {
  Configuration c(sccs[0]);
  for(unsigned int i = 0; i < numAngles-1; ++i) {
    const IConnectionPoint &ip1 = points[2*i];
    const IConnectionPoint &ip2 = points[2*i+1];
    const StepAngle angle((short)p.p[i]-(short)angleSteps[i], angleSteps[i] == 0 ? 1 : angleSteps[i]);
    const Connection cc(ip1, ip2, angle);
    assert(ip2.P1.configurationSCCI != 0);
    c.add(sccs[ip2.P1.configurationSCCI], ip2.P1.configurationSCCI, cc);
  }
  // Add last angle:
  const IConnectionPoint &ip1 = points[2*(numAngles-1)];
  const IConnectionPoint &ip2 = points[2*(numAngles-1)+1];
  const StepAngle angle(p.lastAngle);
  const Connection cc(ip1, ip2, angle);
  assert(ip2.P1.configurationSCCI != 0);
  c.add(sccs[ip2.P1.configurationSCCI], ip2.P1.configurationSCCI, cc);

  return c;
}

Configuration AngleMapping::getConfiguration(const Configuration &baseConfiguration, int angleI, unsigned short angleStep) const {
  Configuration c(baseConfiguration);

  const IConnectionPoint &ip1 = points[2*angleI];
  const IConnectionPoint &ip2 = points[2*angleI+1];
  const StepAngle angle((short)angleStep-(short)angleSteps[angleI], angleSteps[angleI] == 0 ? 1 : angleSteps[angleI]);
  const Connection cc(ip1, ip2, angle);
  assert(ip2.P1.configurationSCCI != 0);
  c.add(sccs[ip2.P1.configurationSCCI], ip2.first.configurationSCCI, cc);

  return c;
}

std::vector<Connection> AngleMapping::getConfigurationConnections(const MixedPosition &p) const {
  std::vector<Connection> res;
  for(unsigned int i = 0; i < numAngles-1; ++i) {
    const IConnectionPoint &ip1 = points[2*i];
    const IConnectionPoint &ip2 = points[2*i+1];
    const StepAngle angle((short)p.p[i]-(short)angleSteps[i], angleSteps[i]);
    res.push_back(Connection(ip1, ip2, angle));
  }

  const IConnectionPoint &ip1 = points[2*(numAngles-1)];
  const IConnectionPoint &ip2 = points[2*(numAngles-1)+1];
  const StepAngle angle(p.lastAngle);
  res.push_back(Connection(ip1, ip2, angle));

  return res;
}

/*
Evaluate the SML matrix.
Parameters:
- angleI: Used as recursion index in angles. Initialize with 0. 
- smlI: Index into SML. Initially 0.
- c: Configuration being constructed dynamically. Initially containing sccs[0].
- possibleCollisions: possible connections pre-computed to be used in next step. Remember to initialize!
*/
void AngleMapping::evalSML(unsigned int angleI, uint32_t smlI, const Configuration &c, bool noS, bool noM, bool noL) {
  // Find possible collisions:
  const IConnectionPoint &ip1 = points[2*angleI];
  const IConnectionPoint &ip2 = points[2*angleI+1];
  unsigned int ip2I = ip2.first.configurationSCCI;
  std::vector<int> possibleCollisions = c.getPossibleCollisions(sccs[ip2I], IConnectionPair(ip1, ip2));

  // Recursion:
  if(angleI < numAngles-1) {
    // Update smlI:
    assert(smlI < sizeMappings);
    const unsigned short steps = 2*angleSteps[angleI]+1;
    smlI *= steps;

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
  if(noS) SS->insertEmpty(smlI);
  if(noM) MM->insertEmpty(smlI);
  if(noL) LL->insertEmpty(smlI);
  bool sDone = noS;
  bool mDone = noM;
  bool lDone = noL;
  if(sDone && mDone && lDone) {
    ++boosts[0];
    return;
  }
#ifdef _TRACE
  std::cout << "NO SML statuses: " << noS << ", " << noM << ", " << noL << std::endl;
#endif

  if(angleTypes[angleI] == 0) {
    Configuration c2 = getConfiguration(c, angleI, 0);
    IntervalList full;
    full.push_back(Interval(-EPSILON,EPSILON));

    if(!sDone) {
      if(c2.isRealizable<-1>(possibleCollisions, sccs[numAngles].size))
        SS->insert(smlI, full);
      else
        SS->insertEmpty(smlI);
    }
    if(!mDone) {
      if(c2.isRealizable<0>(possibleCollisions, sccs[numAngles].size))
        MM->insert(smlI, full);
      else
        MM->insertEmpty(smlI);
    }
    if(!lDone) {
      if(c2.isRealizable<1>(possibleCollisions, sccs[numAngles].size))
        LL->insert(smlI, full);
      else
        LL->insertEmpty(smlI);
    }
    ++boosts[1];
    return;
  }

  const IConnectionPair icp(ip1,ip2);
  TurningSCCInvestigator tsbInvestigator(c, sccs[numAngles], ip2I, icp);

  // First check quick clear:
  if(!sDone && tsbInvestigator.isClear<-1>(possibleCollisions)) {
    IntervalList full;
    full.push_back(Interval(-MAX_ANGLE_RADIANS,MAX_ANGLE_RADIANS));
    SS->insert(smlI, full);
    sDone = true;
  }
  if(!mDone && tsbInvestigator.isClear<0>(possibleCollisions)) {
    IntervalList full;
    full.push_back(Interval(-MAX_ANGLE_RADIANS,MAX_ANGLE_RADIANS));
    MM->insert(smlI, full);
    mDone = true;
  }
  if(!lDone && tsbInvestigator.isClear<1>(possibleCollisions)) {
    IntervalList full;
    full.push_back(Interval(-MAX_ANGLE_RADIANS,MAX_ANGLE_RADIANS));
    LL->insert(smlI, full);
    lDone = true;
  }
  if(sDone && mDone && lDone) {
    ++boosts[2];
    return;
  }

  // Check using TSB:
  if(!sDone) {
    IntervalList l;
    tsbInvestigator.allowableAnglesForBricks<-1>(possibleCollisions, l);
    SS->insert(smlI, l);
    
#ifdef _DEBUG
    const unsigned short steps = 2*angleSteps[angleI]+1;
    bool *S = new bool[steps];
    math::intervalToArray(l, S, steps);

    int numDisagreements = 0;
    for(unsigned short i = 0; i < steps; ++i) {
      Configuration c2 = getConfiguration(c, angleI, i);
      if(S[i] != c2.isRealizable<-1>(possibleCollisions, sccs[numAngles].size)) {
        ++numDisagreements;
      }
    }
    if(numDisagreements > 0) {
      //std::cout << "Assertion warning on S vs allowableAnglesForBricks. Number of disagreements: " << numDisagreements << std::endl;	 
    }
    if(numDisagreements > 24) {
      std::cout << "Assertion error on S vs allowableAnglesForBricks." << std::endl;	 
      LDRPrinterHandler h, d;

      std::cout << " Connection points: " << std::endl;
      for(unsigned int i = 0; i <= angleI; ++i) {
        const IConnectionPoint &ip1 = points[2*i];
        const IConnectionPoint &ip2 = points[2*i+1];
        std::cout << "  " << ip1 << "-->" << ip2 << std::endl;
      }

      std::cout << "Content of S:" << std::endl;	 
      for(unsigned short j = 0; j < steps; ++j) {
        std::cout << (S[j] ? "X" : "-");
      }
      std::cout << std::endl;
      std::cout << "Using isRealizable on all angles:" << std::endl;	 
      bool first = true;
      for(unsigned short j = 0; j < steps; ++j) {
        Configuration c3 = getConfiguration(c, angleI, j);
        bool realizable = c3.isRealizable<-1>(possibleCollisions, sccs[numAngles].size);
        if(realizable != S[j]) {
          d.add(new Configuration(c3)); // OK Be cause we are about to die.
          if(first) {
            h.add(&c3);
            h.print("assertion_fail");
            first = false;
          }
        }
        std::cout << (realizable ? "X" : "-");
      }
      first = true;
      std::cout << std::endl;
      d.print("disagreements");
      std::cout << "Number of disagreements: " << numDisagreements << ":" << std::endl;
      for(unsigned short j = 0; j < steps; ++j) {
        Configuration c3 = getConfiguration(c, angleI, j);
        bool realizable = c3.isRealizable<-1>(possibleCollisions, sccs[numAngles].size);
        if(realizable != S[j]) {
          if(first) {
            std::cout << "Configuration of first disagreement: " << c3 << std::endl;
            first = false;
          }
          const StepAngle angle((short)j-(short)angleSteps[angleI], angleSteps[angleI] == 0 ? 1 : angleSteps[angleI]);
          std::cout << " Angle step: " << j << ", radians: " << angle.toRadians() << (realizable ? ", IsRealizable OK, intervals not." : ", Intervals OK, isRealizable not") << std::endl;
        }
      }
      assert(false); int* kill = NULL; kill[0] = 0;
    }
    delete[] S;
#endif
//*/
  }
  if(!mDone) {
    IntervalList l;
    tsbInvestigator.allowableAnglesForBricks<0>(possibleCollisions, l);
    MM->insert(smlI, l);
  }
  if(!lDone) {
    IntervalList l;
    tsbInvestigator.allowableAnglesForBricks<1>(possibleCollisions, l);
    LL->insert(smlI, l);
  }
  ++boosts[3];
}

void AngleMapping::findIslands(std::multimap<Encoding, SIsland> &sIslands, std::set<Encoding> &keys) {
  // Add all S-islands:
  for(std::vector<uint32_t>::const_iterator it = ufS->rootsBegin(); it != ufS->rootsEnd(); ++it) {
    const uint32_t unionI = *it;
    MixedPosition rep;
    ufS->getRepresentativeOfUnion(unionI, rep);

#ifdef _DEBUG
    if(ufS->getRootForPosition(rep) != unionI) {
      std::cout << ufS->getRootForPosition(rep) <<"!="<< unionI << std::endl;
    }
#endif
    assert(ufS->getRootForPosition(rep) == unionI);

    Configuration c = getConfiguration(rep);
    std::vector<IConnectionPair> found;
    c.isRealizable<-1>(found);
    Encoding encoding = encoder.encode(found);
    sIslands.insert(std::make_pair(encoding, SIsland(this, unionI, rep)));
    if(keys.find(encoding) == keys.end())
      keys.insert(encoding);
  }
}

/*
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
}*/

void AngleMapping::reportProblematic(const MixedPosition &p, int mIslandI, int mIslandTotal, int lIslandTotal, std::vector<std::vector<Connection> > &toLdr) const {
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
  for(unsigned int i = 0; i < numAngles-1; ++i) {
    double radian = StepAngle(p.p[i]-angleSteps[i], angleSteps[i]).toRadians();
    os << "   " << (i+1) << ": step " << p.p[i] << "/" << (2*angleSteps[i]+1) << ", fraction " << (p.p[i]-angleSteps[i]) << "/" << angleSteps[i] << ", radian " << radian << std::endl;
  }
  os << "   " << numAngles << ": radian " << p.lastAngle << std::endl;
  if(mIslandTotal > 0)
    os << "  This configuration represents M-island " << (mIslandI+1) << "/" << mIslandTotal << ". There are " << lIslandTotal << " L-islands in this M-island" << std::endl;
  else
    os << "  S islands without M-islands inside!" << std::endl;        

  if(numAngles < 3) {
    //Create maping file:
    std::stringstream mappingFileName;
    mappingFileName << "manual\\" << numBricks << "\\";
    encoder.writeFileName(mappingFileName, cc);
    mappingFileName << "_mapping.txt";
    std::ofstream mappingFile;
    mappingFile.open(mappingFileName.str(), std::ios::out);
    mappingFile << numAngles << " ";
    for(unsigned int i = 0; i < numAngles-1; ++i)
      mappingFile << " " << (2*angleSteps[i]+1);
    mappingFile << std::endl;

    // Print SML Mapping!
    if(numAngles == 1) {
      IntervalList s, m , l;
      SS->get(0, s);
      MM->get(0, m);
      LL->get(0, l);
      mappingFile << s << std::endl;
      mappingFile << m << std::endl;
      mappingFile << l << std::endl;
    }
    else {
      for(int i = 0; i < 2*angleSteps[0]+1; ++i) {
        IntervalList s, m , l;
        SS->get(i, s);
        MM->get(i, m);
        LL->get(i, l);
        mappingFile << s << std::endl;
        mappingFile << m << std::endl;
        mappingFile << l << std::endl;
      }
    }
    mappingFile.flush();
    mappingFile.close();
  }
  os << std::endl;
}

void AngleMapping::findNewConfigurations(std::set<Encoding> &rect, std::set<Encoding> &nonRect, std::vector<std::vector<Connection> > &toLdr, std::vector<Configuration> &nrcToPrint, std::vector<Configuration> &modelsToPrint, counter &models, counter &problematic) {
  time_t startTime, endTime;
  time(&startTime);

  // Compute rectilinear index:
  rectilinearIndex = numAngles > 1 ? angleSteps[0] : 0;
  rectilinearPosition.p[0] = angleSteps[0];
  for(unsigned int i = 1; i < numAngles-1; ++i) {
    const int push = 2*angleSteps[i] + 1;
    rectilinearIndex = (rectilinearIndex * push) + angleSteps[i];
    rectilinearPosition.p[i] = angleSteps[i];
  }
  rectilinearPosition.lastAngle = 0;

  // Evaluate SML:
  Configuration c(sccs[0]);
  evalSML(0, 0, c, false, false, false);

  // Evaluate union-find:
  unsigned short sizes[MAX_DIMENSIONS-1];
  for(unsigned int i = 0; i < numAngles-1; ++i) {
    sizes[i] = 2*angleSteps[i]+1;
  }
  ufS = new UnionFind::IntervalUnionFind(numAngles, sizes, *SS);
  ufM = new UnionFind::IntervalUnionFind(numAngles, sizes, *MM);
  ufL = new UnionFind::IntervalUnionFind(numAngles, sizes, *LL);

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
      if(allNrcInRange.size() > 1) {
        for(std::vector<Configuration>::const_iterator itR = allNrcInRange.begin(); itR != allNrcInRange.end(); ++itR) {
          modelsToPrint.push_back(*itR);
        }
        models+=allNrcInRange.size();
      }
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
