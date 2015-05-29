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
    rectilinear+=mgr.rectilinear;
    nonRectilinearIConnectionPairLists+=mgr.nonRectilinearIConnectionPairLists;
    models+=mgr.models;
    problematic+=mgr.problematic;
    //std::cout << "Found " << mgr.rectilinear << " rectilinear combinations in " << mgr.attempts << " attempts." << std::endl;
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
  std::cout << ") started!" << std::endl;//*/

  int firstSCCSize = combinationType[0];
  for(unsigned int i = 0; i < sccsSize[firstSCCSize-1]; ++i) {
    std::vector<FatSCC> v;
    v.push_back(sccs[firstSCCSize-1][i]);
    runForCombination(v, combinationType, i);
  }

  std::cout << "Found " << rectilinear << " rectilinear combinations in " << attempts << " attempts." << std::endl;


  // TODO: Thread here!
}

void ConfigurationManager::runForCombinationType(const std::vector<int> &combinationType, int remaining, int prevSize) {
  //std::cout << "ConfigurationManager::runForCombinationType(remaining=" << remaining << ") started!" << std::endl;
  if(remaining == 0) {
    runForCombinationType(combinationType);
    return;
  }
    
  for(int i = min(prevSize,remaining); i > 0; --i) {
    std::vector<int> v(combinationType);
    v.push_back(i);
    runForCombinationType(v, remaining-i, i);
  }
}

void ConfigurationManager::runForSize(int size) {
  std::cout << "ConfigurationManager::runForSize(" << size << ") started!" << std::endl;
  if(size < 2) {
    std::cerr << "Error: ConfigurationManager::run must be for at least size 2!" << std::endl;
    return;
  }

  // For base being 1..size-1:
  for(int base = size-1; base > 0; --base) {
    std::vector<int> combination;
    combination.push_back(base);
    runForCombinationType(combination, size-base, base);
  }

  // Output results:
  std::cout << "Results for size " << size << ":" << std::endl;
  std::cout << " Strongly connected configurations (SCC): " << sccsSize[size-1] << std::endl;
  std::cout << " Attempts: " << attempts << std::endl;
  std::cout << " Rectilinear configurations: " << sccsSize[size-1] << " + " << rectilinear << " = " << (sccsSize[size-1]+rectilinear) << std::endl;
  std::cout << " Ways to connect SCCs resulting in new models: " << nonRectilinearIConnectionPairLists << std::endl;
  std::cout << " Models: " << models << std::endl;
  std::cout << " Models requiring manual confirmation: " << problematic << std::endl;
  std::cout << std::endl;
}

ConfigurationManager::ConfigurationManager() : attempts(0), rectilinear(0), nonRectilinearIConnectionPairLists(0), models(0), problematic(0) {
  StronglyConnectedConfigurationManager sccMgr;
  for(int i = 0; i < 3; ++i) {
    sccs[i] = sccMgr.loadFromFile(i, sccsSize[i]);
    // TODO: Run to 5!
  }
}

void ConfigurationManager::test() const {
  /*ConfigurationManager mgr;
  std::vector<int> v;
  v.push_back(1);
  v.push_back(1);
  v.push_back(1);
  v.push_back(1);

  //mgr.runForCombinationType(v);
  
  std::vector<FatSCC> v2;
  v2.push_back(sccs[0][0]);
  v2.push_back(sccs[0][0]);
  v2.push_back(sccs[0][0]);
  v2.push_back(sccs[0][0]);

  mgr.runForCombination(v2, v, -1);//*/

  //SingleConfigurationManager::test1();
  //SingleConfigurationManager::test2();
}
