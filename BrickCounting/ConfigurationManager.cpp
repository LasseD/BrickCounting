#include "ConfigurationManager.h"
#include "StronglyConnectedConfigurationManager.h"
#include "SingleConfigurationManager.h"

#include <iostream>
#include <algorithm>
#include <time.h>

void ConfigurationManager::runForCombination(const std::vector<FatSCC> &combination, const std::vector<int> &combinationType, int prevSCCIndex, std::ofstream &os) {
  if(combination.size() == combinationType.size()) {
    SingleConfigurationManager mgr(combination, os);
    mgr.run();

    attempts+=mgr.attempts;
    rectilinear+=mgr.foundRectilinearConfigurationsEncoded.size();
    nonRectilinearConfigurations+=mgr.foundNonRectilinearConfigurationsEncoded.size();
    models+=mgr.models;
    problematic+=mgr.problematic;
    for(int i = 0; i < BOOST_STAGES; ++i) {
      angleMappingBoosts[i] += mgr.angleMappingBoosts[i];
    }
#ifdef _COMPARE_ALGORITHMS
    std::cout << "Removing " << mgr.foundSCCs.size() << " from " << correct.size() << std::endl;
    for(std::map<FatSCC,uint64_t>::const_iterator it = mgr.foundSCCs.begin(); it != mgr.foundSCCs.end(); ++it) {
      const FatSCC &scc = it->first;
      if(correct.find(scc) == correct.end()) {
        std::cout << "Incorrectly found: " << scc << std::endl;
        MPDPrinter h;
        h.add("IncorrectlyFound", new Configuration(scc)); // new OK as we are done.
        h.print("IncorrectlyFound");
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
  std::cout << " Non-rectilinear corner connected SCCs:    " << nonRectilinearConfigurations << std::endl;
  std::cout << " Models:                                   " << models << std::endl;
  std::cout << " Models requiring manual confirmation:     " << problematic << std::endl;
  std::cout << " Program execution time (seconds):         " << seconds << std::endl;
#ifdef _COMPARE_ALGORITHMS
  std::cout << " Boosts performed in AngleMappings:" << std::endl;
  for(int i = 0; i < BOOST_STAGES; ++i) {
    std::cout << "  BOOST LEVEL " << (i+1) << ": " << angleMappingBoosts[i] << std::endl;
  }
  std::cout << " Remaining Rectilinear SCCs to find:       " << correct.size() << std::endl;
#endif
  std::cout << std::endl;
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

  time(&endTime);
  double seconds = difftime(endTime,startTime);

  // Output results:
  std::cout << "Results for size " << size << ":" << std::endl;
  std::cout << " Attempts:                                 " << attempts << std::endl;
  std::cout << " Rectilinear corner connected SCCs:        " << rectilinear << std::endl;
  std::cout << " Non-rectilinear corner connected SCCs:    " << nonRectilinearConfigurations << std::endl;
  std::cout << " Models:                                   " << models << std::endl;
  std::cout << " Models requiring manual confirmation:     " << problematic << std::endl;
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
  MPDPrinter d;
  int j = 0;
  for(std::set<FatSCC>::const_iterator it = correct.begin(); it != correct.end(); ++it) {
    std::stringstream ss;
    ss << "missing_" << j++;
    d.add(ss.str(), new Configuration(*it)); // 'new' is OK as we are done... and don't give a damn anymore.
  }
  d.print("missing");
#endif

  std::cout << std::endl;
}

ConfigurationManager::ConfigurationManager(int maxSccSize) : attempts(0), rectilinear(0), nonRectilinearConfigurations(0), models(0), problematic(0) {
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
  runForSize(4);

/*
  std::vector<int> v;
  v.push_back(2);
  v.push_back(2);

  //runForCombinationType(v, 3);
  
  std::vector<FatSCC> v2;
  v2.push_back(sccs[1][1]);
  v2.push_back(sccs[1][4]);

  std::ofstream os;
  os.open("temp.txt", std::ios::out);
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
