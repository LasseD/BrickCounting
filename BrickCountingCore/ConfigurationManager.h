#ifndef CONFIGURATION_MANAGER_H
#define CONFIGURATION_MANAGER_H

#include "Block.hpp"
#include "SingleConfigurationManager.h"

class ConfigurationManager {
private:
  FatSCC* sccs[5];
  unsigned long sccsSize[5];

  counter attempts, rectilinear, models, problematic;
  counter angleMappingBoosts[BOOST_STAGES];
  bool findExtremeAnglesOnly;
  util::ProgressWriter pw;

  void runForCombination(const util::TinyVector<FatSCC, 6> &combination, const util::TinyVector<int, 6> &combinationType, int prevSCCIndex, std::ofstream &os);

  void runForCombinationType(const util::TinyVector<int, 6> &combinationType, int remaining, int prevSize, int combinedSize);
public:
  void runForCombinationType(const util::TinyVector<int, 6> &combinationType, int combinedSize);
#ifdef _COMPARE_ALGORITHMS
  std::set<FatSCC> correct; // For debugging only!
#endif

  ConfigurationManager(int maxSccSize, bool findExtremeAnglesOnly);
  void runForSize(int size);

  void test();
  void printResults(const util::TinyVector<int, 6> &combinationType, double seconds) const;
};

#endif // CONFIGURATION_MANAGER_H
