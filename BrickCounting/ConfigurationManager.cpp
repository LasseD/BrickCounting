#include "ConfigurationManager.h"
#include "StronglyConnectedConfigurationManager.h"
#include "SingleConfigurationManager.h"

#include <iostream>
#include <algorithm>
#include <time.h>

void ConfigurationManager::runForCombination(const std::vector<FatSCC> &combination, const std::vector<int> &combinationType, int prevSCCIndex, std::ofstream &os) {
  if(combination.size() == combinationType.size()) {
    SingleConfigurationManager mgr(combination, os, findExtremeAnglesOnly);
    mgr.run();

    attempts+=mgr.attempts;
    rectilinear+=mgr.rectilinear;
    models+=mgr.models;
    problematic+=mgr.manual.size();
    for(int i = 0; i < BOOST_STAGES; ++i) {
      angleMappingBoosts[i] += mgr.angleMappingBoosts[i];
    }
#ifdef _COMPARE_ALGORITHMS
    std::cout << "Removing " << mgr.foundSCCs.size() << " from " << correct.size() << std::endl;
    for(std::map<FatSCC,uint64_t>::const_iterator it = mgr.foundSCCs.begin(); it != mgr.foundSCCs.end(); ++it) {
      const FatSCC &scc = it->first;
      if(correct.find(scc) == correct.end()) {
        std::cout << "Incorrectly found: " << scc << std::endl;
        MPDPrinter h, d;
        h.add("IncorrectlyFound", new Configuration(scc)); // new OK as we are done.
        h.print("IncorrectlyFound");

        for(std::map<FatSCC,uint64_t>::const_iterator it2 = mgr.foundSCCs.begin(); it2 != mgr.foundSCCs.end(); ++it2) {
          const FatSCC &scc2 = it2->first;
          std::stringstream ss;
          ss << "all" << it2->second;
          d.add(ss.str(), new Configuration(scc2)); // new OK as we are done.
        }
        d.print("AllInIncorrectBatch");
      }
      assert(correct.find(scc) != correct.end());
      correct.erase(scc);
    }
#endif

    return;
  }

  int sccSize = combinationType[combination.size()];
  int prevSccSize = combinationType[combination.size()-1];
  unsigned int i = prevSccSize == sccSize ? prevSCCIndex : 0;
  for(; i < sccsSize[sccSize-1]; ++i) {
    std::vector<FatSCC> v(combination);
    FatSCC &fatSCC = sccs[sccSize-1][i];
    if(prevSccSize == sccSize && v.back().index != i)
      assert(v.back().check(fatSCC));
    v.push_back(fatSCC);
    runForCombination(v, combinationType, i, os);
  }
}

void ConfigurationManager::runForCombinationType(const std::vector<int> &combinationType, int combinedSize) {
  if(findExtremeAnglesOnly && combinationType.size() <= 2)
    return; // Don't run extreme angles for combinatoins that can be handled using the normal algorithm.
  time_t startTime, endTime;
  time(&startTime);

  std::ofstream os;
  std::stringstream ss;

  ss << "manual\\manual_size_" << combinedSize << "_sccsizes";

  std::cout << "Computing all models for combination type ";
  for(std::vector<int>::const_iterator it = combinationType.begin(); it != combinationType.end(); ++it) {
    if(it != combinationType.begin())
      std::cout << "/";
    std::cout << *it;
    ss << "_" << *it;
  }
  ss << ".txt";
  os.open(ss.str().c_str(), std::ios::out);
  std::cout << "." << std::endl;

  // Store counters:
  counter storeAttempts = attempts;
  counter storeRectilinear = rectilinear;
  counter storeModels = models;
  counter storeProblematic = problematic;
  attempts = rectilinear = models = problematic = 0;

  int firstSCCSize = combinationType[0];
  for(unsigned int i = 0; i < sccsSize[firstSCCSize-1]; ++i) {
    std::vector<FatSCC> v;
    v.push_back(sccs[firstSCCSize-1][i]);
    runForCombination(v, combinationType, i, os);
  }

  os.flush();
  os.close();

  time(&endTime);
  double seconds = difftime(endTime,startTime);

  printResults(combinationType, seconds);  

  std::cout << std::endl;
  std::cout << "Results for combination type ";
  for(std::vector<int>::const_iterator it = combinationType.begin(); it != combinationType.end(); ++it) {
    if(it != combinationType.begin())
      std::cout << "/";
    std::cout << *it;
  }
  std::cout << " (Accumulated totals):" << std::endl;
  std::cout << " Attempts:                                 " << attempts << std::endl;
  std::cout << " Rectilinear corner connected SCCs:        " << rectilinear << std::endl;
  if(findExtremeAnglesOnly && combinationType.size() > 2) {
    std::cout << " NRCs:                                     " << models << std::endl;
  }
  else {
    std::cout << " Models:                                   " << models << std::endl;
    std::cout << " Models requiring manual confirmation:     " << problematic << std::endl;
  }
  std::cout << " Program execution time (seconds):         " << seconds << std::endl;
#ifdef _COMPARE_ALGORITHMS
  std::cout << " Boosts performed in AngleMappings:" << std::endl;
  for(int i = 0; i < BOOST_STAGES; ++i) {
    std::cout << "  BOOST LEVEL " << (i+1) << ": " << angleMappingBoosts[i] << std::endl;
  }
  std::cout << " Remaining Rectilinear SCCs to find:       " << correct.size() << std::endl;
#endif
  std::cout << std::endl;

  // Restore counters
  attempts+=storeAttempts;
  rectilinear+=storeRectilinear;
  models+=storeModels;
  problematic+=storeProblematic;
}

void ConfigurationManager::runForCombinationType(const std::vector<int> &combinationType, int remaining, int prevSize, int combinedSize) {
  if(remaining == 0) {
    runForCombinationType(combinationType, combinedSize);
    return;
  }
    
  for(int i = MIN(prevSize,remaining); i > 0; --i) {
    std::vector<int> v(combinationType);
    v.push_back(i);
    runForCombinationType(v, remaining-i, i, combinedSize);
  }
}

void ConfigurationManager::runForSize(int size) {
  time_t startTime, endTime;
  time(&startTime);

  // For base being 1..size-1:
  for(int base = size-1; base > 0; --base) {
    std::vector<int> combination;
    combination.push_back(base);
    runForCombinationType(combination, size-base, base, size);
  }

  // Output results:
  if(!findExtremeAnglesOnly) {
    time(&endTime);
    double seconds = difftime(endTime,startTime);

    std::vector<int> fakeCombination;
    fakeCombination.push_back(size);
    printResults(fakeCombination, seconds);

    std::cout << "Results for size " << size << ":" << std::endl;
    std::cout << " Attempts:                                 " << attempts << std::endl;
    std::cout << " Rectilinear corner connected SCCs:        " << rectilinear << std::endl;
    if(findExtremeAnglesOnly) {
      std::cout << " NRCs/models:                              " << models << std::endl;
    }
    else {
      std::cout << " Models:                                   " << models << std::endl;
      std::cout << " Models requiring manual confirmation:     " << problematic << std::endl;
    }
    std::cout << " Program execution time (seconds):         " << seconds << std::endl;
#ifdef _DEBUG
    std::cout << " Boosts performed in AngleMappings:" << std::endl;
    for(int i = 0; i < BOOST_STAGES; ++i) {
      std::cout << "  BOOST LEVEL " << (i+1) << ": " << angleMappingBoosts[i] << std::endl;
    }
#endif
#ifdef _COMPARE_ALGORITHMS
    std::cout << " Remaining Rectilinear SCCs to find:       " << correct.size() << std::endl;
    // Print unseen:
    if(!correct.empty()) {
      MPDPrinter d;
      int j = 0;
      for(std::set<FatSCC>::const_iterator it = correct.begin(); it != correct.end(); ++it) {
        std::stringstream ss;
        ss << "missing_" << j++;
        d.add(ss.str(), new Configuration(*it)); // 'new' is OK as we are done... and don't give a damn anymore.
      }
      d.print("missing");
    }
#endif
    std::cout << std::endl;
  }
}

ConfigurationManager::ConfigurationManager(int maxSccSize, bool findExtremeAnglesOnly) : attempts(0), rectilinear(0), models(0), problematic(0), findExtremeAnglesOnly(findExtremeAnglesOnly) {
  for(int i = 0; i < BOOST_STAGES; ++i) {
    angleMappingBoosts[i] = 0;
  }
  StronglyConnectedConfigurationManager sccMgr;
  for(int i = 0; i < maxSccSize; ++i) {
    sccs[i] = sccMgr.loadFromFile(i, sccsSize[i], false);
  }
#ifdef _COMPARE_ALGORITHMS
  unsigned long correctSccsSize;;
  FatSCC* correctSccs = sccMgr.loadFromFile(maxSccSize-1, correctSccsSize, true);
  for(unsigned long i = 0; i < correctSccsSize; ++i) {
    FatSCC scc(correctSccs[i]);
    scc.index = NO_INDEX;
    scc = scc.rotateToMin();
    assert(correct.find(scc) == correct.end());
    correct.insert(scc);
  }
  // remove all SCCs:
  for(unsigned long i = 0; i < sccsSize[maxSccSize-1]; ++i) {
    FatSCC scc(sccs[maxSccSize-1][i]);
    scc.index = NO_INDEX;
    scc = scc.rotateToMin();
    if(correct.find(scc) == correct.end()) {
      std::cout << "Incorrectly removed: " << scc << std::endl;
      MPDPrinter h;
      h.add("IncorrectlyRemoved", new Configuration(scc)); // new OK as we are done.
      h.print("IncorrectlyRemoved");
    }
    assert(correct.find(scc) != correct.end());
    correct.erase(scc);
  }
  std::cout << "RCs to be found: " << correct.size() << std::endl;
  delete[] correctSccs;
#endif
}

void ConfigurationManager::test() {
  std::vector<int> v;
  v.push_back(1);
  v.push_back(1);
  v.push_back(1);
  v.push_back(1);
  v.push_back(1);
  v.push_back(1);

  runForCombinationType(v, 5);
/*  
  std::vector<FatSCC> v2;
  v2.push_back(sccs[1][0]);
  v2.push_back(sccs[1][4]);
  v2.push_back(sccs[1][10]); // maybe 9, 10, 11, ...
  //ConfigurationEncoder encoder(v2);

  std::ofstream os;
  os.open("temp.txt", std::ios::out);
  runForCombination(v2, v, -1, os);//*/
  /*
  std::vector<IConnectionPair> pairs;
  RectilinearBrick b0;
  RectilinearBrick &b1 = sccs[1][3].otherBricks[0];

  IConnectionPoint icp1(BrickIdentifier(3,1,0),ConnectionPoint(NW,b1,false,1));
  IConnectionPoint icp2(BrickIdentifier(0,0,1),ConnectionPoint(NW,b0,true ,0));
  IConnectionPoint icp3(BrickIdentifier(0,0,1),ConnectionPoint(NE,b0,false,0));
  IConnectionPoint icp4(BrickIdentifier(0,0,2),ConnectionPoint(NE,b0,true ,0));

  pairs.push_back(IConnectionPair(icp1,icp2));
  pairs.push_back(IConnectionPair(icp3,icp4));

  SingleConfigurationManager sm(v2, os);
  std::vector<IConnectionPoint> pools;
  sm.run(pairs, pools, pools, NULL, 0);



  /*
  uint64_t encoded1 = 1082724547;
  uint64_t encoded2 = 1111896211;
  IConnectionPairList list1, list2;
  std::vector<IConnectionPair> list1x, list2x;
  encoder.decode(encoded1, list1);
  encoder.decode(encoded2, list2);

  std::cout << "1082724547: " << std::endl;
  std::vector<Connection> cs1, cs2;
  for(std::set<IConnectionPair>::const_iterator it2 = list1.begin(); it2 != list1.end(); ++it2) {
    cs1.push_back(Connection(*it2, StepAngle()));
    list1x.push_back(*it2);
    std::cout << *it2 << std::endl;
  }
  std::cout << "1111896211: " << std::endl;
  for(std::set<IConnectionPair>::const_iterator it2 = list2.begin(); it2 != list2.end(); ++it2) {
    cs2.push_back(Connection(*it2, StepAngle()));
    list2x.push_back(*it2);
    std::cout << *it2 << std::endl;
  }
  Configuration c1(&(v2[0]), cs1);
  Configuration c2(&(v2[0]), cs2);
  FatSCC min = c1.toMinSCC();

  std::vector<IConnectionPair> found1, found2;
  c1.isRealizable<-MOLDING_TOLERANCE_MULTIPLIER>(found1);
  uint64_t encoding1 = encoder.encode(found1).first;
  std::cout << "Encoding 1: " << encoding1 << std::endl;
  c2.isRealizable<-MOLDING_TOLERANCE_MULTIPLIER>(found2);
  uint64_t encoding2 = encoder.encode(found2).first;
  std::cout << "Encoding 2: " << encoding2 << std::endl;

  std::ofstream os;
  os.open("temp.txt", std::ios::out);
  std::vector<IConnectionPoint> pool;
  SingleConfigurationManager singleMgr(v2, os);
  singleMgr.run(list1x, pool, pool, NULL, 0);
  singleMgr.run(list2x, pool, pool, NULL, 0);

  MPDPrinter h;
  Configuration cf(min);
  h.add("abe1", &cf);
  h.print("abe1");

  /*
  runForCombination(v2, v, -1, os);//*/
  /*
  SingleConfigurationManager sm(v2, os);
  std::vector<IConnectionPoint> pools;

  std::vector<IConnectionPair> pairs;
  RectilinearBrick b0;
  RectilinearBrick &b1 = sccs[1][2].otherBricks[0];

  IConnectionPoint icp1(BrickIdentifier(2,0,0),ConnectionPoint(NE,b0,false,0));
  IConnectionPoint icp2(BrickIdentifier(0,0,1),ConnectionPoint(SW,b0,true ,0));
  IConnectionPoint icp3(BrickIdentifier(0,0,1),ConnectionPoint(NW,b0,true ,0));
  IConnectionPoint icp4(BrickIdentifier(0,0,2),ConnectionPoint(NE,b0,false,0));
  // ICP[BI[scc=3,1,0],ASW]<>ICP[BI[scc=0,0,1],BSW] ICP[BI[scc=0,0,1],BNW]<>ICP[BI[scc=0,0,2],ANW]

  pairs.push_back(IConnectionPair(icp1,icp2));
  pairs.push_back(IConnectionPair(icp3,icp4));

  sm.run(pairs, pools, pools, NULL, 0);//*/
}

void ConfigurationManager::printResults(const std::vector<int> &combinationType, double seconds) const {
  std::stringstream ssFileName;
  ssFileName << "results";
  for(std::vector<int>::const_iterator it = combinationType.begin(); it != combinationType.end(); ++it) {
    ssFileName << "_" << *it;
  }
  ssFileName << ".txt";
  std::ofstream ss;
  ss.open(ssFileName.str().c_str(), std::ios::out);

  ss << "Results for combination type ";
  for(std::vector<int>::const_iterator it = combinationType.begin(); it != combinationType.end(); ++it) {
    if(it != combinationType.begin())
      ss << "/";
    ss << *it;
  }
  ss << ":" << std::endl;
  ss << " Attempts:                                 " << attempts << std::endl;
  ss << " Rectilinear corner connected SCCs:        " << rectilinear << std::endl;
  if(findExtremeAnglesOnly && combinationType.size() > 2) {
    ss << " NRCs:                                     " << models << std::endl;
  }
  else {
    ss << " Models:                                   " << models << std::endl;
    ss << " Models requiring manual confirmation:     " << problematic << std::endl;
  }
  ss << " Program execution time (seconds):         " << seconds << std::endl;
  ss.flush();
  ss.close();  
}
