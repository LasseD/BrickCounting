#ifndef CONFIGURATION_MANAGER_H
#define CONFIGURATION_MANAGER_H

#include "StronglyConnectedConfiguration.hpp"
#include "SingleConfigurationManager.h"

class ConfigurationManager {
private:
  FatSCC* sccs[5];
  unsigned long sccsSize[5];

  counter attempts, rectilinear, nonRectilinearConnectionLists, models, problematic;

  void runForCombination(const std::vector<FatSCC> &combination, const std::vector<int> &combinationType, int prevSCCIndex);

  void runForCombinationType(const std::vector<int> &combinationType);
  void runForCombinationType(const std::vector<int> &combinationType, int remaining);
public:
  ConfigurationManager();
  void runForSize(int size);

  void test() const;
};

#endif // CONFIGURATION_MANAGER_H
