#include "ConfigurationManager.h"
#include "StronglyConnectedConfigurationManager.h"
#include "SingleConfigurationManager.h"

#include <iostream>
#include <algorithm>
#include <time.h>

void ConfigurationManager::runForCombination(const util::TinyVector<FatSCC, 6> &combination, const util::TinyVector<int, 6> &combinationType, int prevSCCIndex, std::ofstream &os) {
  if(combination.size() == combinationType.size()) {
    
    pw.reportProgress();
    SingleConfigurationManager mgr(combination, os, findExtremeAnglesOnly);
    mgr.run();

    attempts+=mgr.attempts;
    rectilinear+=mgr.rectilinear;
    models+=mgr.models;
    problematic+=mgr.manual.size();
    for(int i = 0; i < BOOST_STAGES; ++i) {
      angleMappingBoosts[i] += mgr.angleMappingBoosts[i];
    }

#ifdef _DEBUG
//    mgr.printLDRFile();
#endif

#ifdef _COMPARE_ALGORITHMS
    if(correct.empty()) {
      return;
    }
    std::cout << "Removing " << mgr.foundSCCs.size() << " from " << correct.size() << std::endl;
    for(std::map<FatSCC,uint64_t>::const_iterator it = mgr.foundSCCs.begin(); it != mgr.foundSCCs.end(); ++it) {
      const FatSCC &scc = it->first;
      if(correct.find(scc) == correct.end()) {
        std::cout << "Incorrectly found: " << scc << std::endl;
        MPDPrinter h, d, d2;
        h.add("IncorrectlyFound", new Configuration(scc)); // new OK as we are done.
        h.print("IncorrectlyFound");

        for(std::map<FatSCC,uint64_t>::const_iterator it2 = mgr.foundSCCs.begin(); it2 != mgr.foundSCCs.end(); ++it2) {
          const FatSCC &scc2 = it2->first;
          std::stringstream ss;
          ss << "all" << it2->second;
          d.add(ss.str(), new Configuration(scc2)); // new OK as we are done.
        }
        d.print("AllInIncorrectBatch");

        int i = 0;
        for(std::set<FatSCC>::const_iterator it2 = correct.begin(); it2 != correct.end() && i < 9; ++it2, ++i) {
          const FatSCC &scc2 = *it2;
          std::stringstream ss;
          ss << "correct_" << i;
          d2.add(ss.str(), new Configuration(scc2)); // new OK as we are done.
        }
        d2.print("FirstCorrect");

        assert(false);std::cerr << "DIE X429" << std::endl;
        int *die = NULL; die[0] = 42;
      }
      correct.erase(scc);
    }
#endif

    return;
  }

  int sccSize = combinationType[combination.size()];
  int prevSccSize = combinationType[combination.size()-1];
  unsigned int i = prevSccSize == sccSize ? prevSCCIndex : 0;
  for(; i < sccsSize[sccSize-1]; ++i) {
    util::TinyVector<FatSCC, 6> v(combination);
    FatSCC &fatSCC = sccs[sccSize-1][i];
    v.push_back(fatSCC);
    runForCombination(v, combinationType, i, os);
  }
}

void ConfigurationManager::runForCombinationType(const util::TinyVector<int, 6> &combinationType, int combinedSize) {
#ifdef _COMPARE_ALGORITHMS
  RectilinearConfigurationManager sccMgr;
  sccMgr.loadFromFile(correct, combinationType);
  std::cout << "RCs to be found: " << correct.size() << std::endl;
#endif

  if(findExtremeAnglesOnly && combinationType.size() <= 2)
    return; // Don't run extreme angles for combinatoins that can be handled better using the normal algorithm.
  time_t startTime, endTime;
  time(&startTime);

  std::ofstream os;
  std::stringstream ss, ss2;

  ss << "manual\\manual_size_" << combinedSize << "_sccsizes";
  ss2 << " Combination type ";

  for(const int* it = combinationType.begin(); it != combinationType.end(); ++it) {
    if(it != combinationType.begin()) {
      ss2 << "/";
    }
    ss2 << *it;
    ss << "_" << *it;
  }
  std::cout << "Computing all models for" << ss2.str();
  ss << ".txt";
  os.open(ss.str().c_str(), std::ios::out);
  if(findExtremeAnglesOnly)
    std::cout << " (extreme angles only)";
  std::cout << "." << std::endl;
  pw.initReportName(ss2.str());

  unsigned long steps = 1;
  for(const int* it = combinationType.begin(); it != combinationType.end(); ++it) {
    steps *= sccsSize[*it-1];
  }
  pw.initSteps(steps);
  pw.initTime();

  // Store counters:
  counter storeAttempts = attempts;
  counter storeRectilinear = rectilinear;
  counter storeModels = models;
  counter storeProblematic = problematic;
  attempts = rectilinear = models = problematic = 0;

  int firstSCCSize = combinationType[0];
  for(unsigned int i = 0; i < sccsSize[firstSCCSize-1]; ++i) {
    util::TinyVector<FatSCC, 6> v;
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
  for(const int* it = combinationType.begin(); it != combinationType.end(); ++it) {
    if(it != combinationType.begin())
      std::cout << "/";
    std::cout << *it;
  }
  std::cout << " (Accumulated totals):" << std::endl;
  std::cout << " Attempts:                                 " << attempts << std::endl;
  std::cout << " Precision boost multiplier:               " << PRECISION_BOOST_MULTIPLIER << std::endl;
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
  std::cout << std::endl;
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

  // Restore counters
  attempts+=storeAttempts;
  rectilinear+=storeRectilinear;
  models+=storeModels;
  problematic+=storeProblematic;
}

void ConfigurationManager::runForCombinationType(const util::TinyVector<int, 6> &combinationType, int remaining, int prevSize, int combinedSize) {
  if(remaining == 0) {
    runForCombinationType(combinationType, combinedSize);
    return;
  }
    
  for(int i = MIN(prevSize,remaining); i > 0; --i) {
    util::TinyVector<int, 6> v(combinationType);
    v.push_back(i);
    runForCombinationType(v, remaining-i, i, combinedSize);
  }
}

void ConfigurationManager::runForSize(int size) {
  time_t startTime, endTime;
  time(&startTime);

  // For base being 1..size-1:
  for(int base = size-1; base > 0; --base) {
    util::TinyVector<int, 6> combination;
    combination.push_back(base);
    runForCombinationType(combination, size-base, base, size);
  }

  // Output results:
  time(&endTime);
  double seconds = difftime(endTime,startTime);

  util::TinyVector<int, 6> fakeCombination;
  fakeCombination.push_back(size);
  printResults(fakeCombination, seconds);

  std::cout << "Results for size " << size << ":" << std::endl;
  std::cout << " Attempts:                                 " << attempts << std::endl;
  std::cout << " Precision boost multiplier:               " << PRECISION_BOOST_MULTIPLIER << std::endl;
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

ConfigurationManager::ConfigurationManager(int maxSccSize, bool findExtremeAnglesOnly) : attempts(0), rectilinear(0), models(0), problematic(0), findExtremeAnglesOnly(findExtremeAnglesOnly) {
  for(int i = 0; i < BOOST_STAGES; ++i) {
    angleMappingBoosts[i] = 0;
  }
  RectilinearConfigurationManager sccMgr;
  for(int i = 0; i < MIN(maxSccSize,5); ++i) {
    sccs[i] = sccMgr.loadFromFile(i, sccsSize[i], false);
  }
}

void ConfigurationManager::test() {
  util::TinyVector<int, 6> v;
  v.push_back(2);
  v.push_back(1);
  v.push_back(1);
  runForCombinationType(v, 4);
/*
  // Investigate failure #76:
  util::TinyVector<FatSCC, 6> v2;
  v2.push_back(sccs[2][125]);
  v2.push_back(sccs[2][934]);

  std::ofstream os;
  os.open("temp.txt", std::ios::out);//

  //runForCombination(v2, v, -1, os);

  util::TinyVector<IConnectionPair, 5> pairs;
  RectilinearBrick b0;
  RectilinearBrick &b1 = sccs[2][125].otherBricks[0];
  RectilinearBrick &b2 = sccs[2][934].otherBricks[1];

  IConnectionPoint icp1(BrickIdentifier(125,1,0),ConnectionPoint(NE,b1,true,1));
  IConnectionPoint icp2(BrickIdentifier(934,2,1),ConnectionPoint(NW,b2,false,2));

  pairs.push_back(IConnectionPair(icp1,icp2));

  SingleConfigurationManager sm(v2, os, false);
  std::vector<IConnectionPoint> pools;
  sm.run(pairs, pools, pools, NULL, 0);
  sm.printLDRFile();
  sm.printManualLDRFiles();
  */
}

void ConfigurationManager::printResults(const util::TinyVector<int, 6> &combinationType, double seconds) const {
  std::stringstream ssFileName;
  ssFileName << "results";
  for(const int* it = combinationType.begin(); it != combinationType.end(); ++it) {
    ssFileName << "_" << *it;
  }
  ssFileName << ".txt";
  std::ofstream ss;
  ss.open(ssFileName.str().c_str(), std::ios::out);

  ss << "Results for combination type ";
  for(const int* it = combinationType.begin(); it != combinationType.end(); ++it) {
    if(it != combinationType.begin())
      ss << "/";
    ss << *it;
  }
  ss << ":" << std::endl;
  ss << " Attempts:                                 " << attempts << std::endl;
  ss << " Precision boost multiplier:               " << PRECISION_BOOST_MULTIPLIER << std::endl;
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
