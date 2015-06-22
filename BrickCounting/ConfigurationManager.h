#ifndef CONFIGURATION_MANAGER_H
#define CONFIGURATION_MANAGER_H

#include "StronglyConnectedConfiguration.hpp"
#include "SingleConfigurationManager.h"

class ConfigurationManager {
private:
  FatSCC* sccs[5];
  unsigned long sccsSize[5];

  counter attempts, rectilinear, nonRectilinearConfigurations, models, problematic;

  void runForCombination(const std::vector<FatSCC> &combination, const std::vector<int> &combinationType, int prevSCCIndex, std::ofstream &os);

  void runForCombinationType(const std::vector<int> &combinationType, int combinedSize);
  void runForCombinationType(const std::vector<int> &combinationType, int remaining, int prevSize, int combinedSize);
public:
  //std::set<StronglyConnectedConfiguration<4> > foundSCCs; // For debugging only!
  //std::set<StronglyConnectedConfiguration<4> > &correct; // For debugging only!
  ConfigurationManager();
  void runForSize(int size);

  void test();
};

#endif // CONFIGURATION_MANAGER_H
