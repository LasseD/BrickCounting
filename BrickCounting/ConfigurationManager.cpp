#include "ConfigurationManager.h"
#include "StronglyConnectedConfigurationManager.h"
#include "SingleConfigurationManager.h"

#include <iostream>
#include <algorithm>

void ConfigurationManager::runForCombination(const std::vector<FatSCC> &combination, const std::vector<int> &combinationType, int prevSCCIndex) {
  if(combination.size() == combinationType.size()) {
    SingleConfigurationManager mgr(combination);
    mgr.run();
    attempts+=mgr.attempts;
    rectilinear+=mgr.foundRectilinearConfigurationsEncoded.size();
    nonRectilinearConfigurations+=mgr.foundNonRectilinearConfigurationsEncoded.size();
    models+=mgr.models;
    problematic+=mgr.problematic;
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
    runForCombination(v, combinationType, i);
  }  
}

void ConfigurationManager::runForCombinationType(const std::vector<int> &combinationType) {
  std::cout << "ConfigurationManager::runForCombinationType(";
  for(std::vector<int>::const_iterator it = combinationType.begin(); it != combinationType.end(); ++it)
    std::cout << *it << " ";
  std::cout << ") started!" << std::endl;

  int firstSCCSize = combinationType[0];
  for(unsigned int i = 0; i < sccsSize[firstSCCSize-1]; ++i) {
    std::vector<FatSCC> v;
    v.push_back(sccs[firstSCCSize-1][i]);
    runForCombination(v, combinationType, i);
  }

  std::cout << "Current counts: " << std::endl;
  std::cout << " " << rectilinear << " rectilinear combinations." << std::endl;
  std::cout << " " << attempts << " attempts." << std::endl;
  std::cout << " " << nonRectilinearConfigurations << " corner connected SCCs." << std::endl;

  // TODO: Thread here!
}

void ConfigurationManager::runForCombinationType(const std::vector<int> &combinationType, int remaining, int prevSize) {
  if(remaining == 0) {
    runForCombinationType(combinationType);
    return;
  }
    
  for(int i = MIN(prevSize,remaining); i > 0; --i) {
    std::vector<int> v(combinationType);
    v.push_back(i);
    runForCombinationType(v, remaining-i, i);
  }
}

void ConfigurationManager::runForSize(int size) {
  // For base being 1..size-1:
  for(int base = size-1; base > 0; --base) {
    std::vector<int> combination;
    combination.push_back(base);
    runForCombinationType(combination, size-base, base);
  }

  // Output results:
  std::cout << "Results for size " << size << ":" << std::endl;
  std::cout << " Attempts:                                 " << attempts << std::endl;
  std::cout << " Strongly connected configurations (SCCs): " << sccsSize[size-1] << std::endl;
  std::cout << " Rectilinear corner connected SCCs:        " << rectilinear << std::endl;
  std::cout << " Non-rectilinear corner connected SCCs:    " << nonRectilinearConfigurations << std::endl;
  std::cout << " Models:                                   " << models << std::endl;
  std::cout << " Models requiring manual confirmation:     " << problematic << std::endl;
  std::cout << std::endl;
}

ConfigurationManager::ConfigurationManager() : attempts(0), rectilinear(0), nonRectilinearConfigurations(0), models(0), problematic(0) {
  StronglyConnectedConfigurationManager sccMgr;
  int upTo = 5;
#ifdef _DEBUG
  upTo = 4; // Decreases file loading time - never run for full input in bebug mode!
#endif
  for(int i = 0; i < upTo; ++i) {
    sccs[i] = sccMgr.loadFromFile(i, sccsSize[i]);
  }
}

void ConfigurationManager::test() {
  std::vector<int> v;
  v.push_back(2);
  v.push_back(1);
  v.push_back(1);

  runForCombinationType(v);
  /*
  std::vector<FatSCC> v2;
  v2.push_back(sccs[0][0]);
  v2.push_back(sccs[0][0]);
  v2.push_back(sccs[0][0]);
  v2.push_back(sccs[0][0]);

  mgr.runForCombination(v2, v, -1);//*/

  //SingleConfigurationManager::test1();
  //SingleConfigurationManager::test2();
}
