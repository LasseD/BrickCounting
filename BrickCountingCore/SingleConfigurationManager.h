#ifndef SINGLE_CONFIGURATION_MANAGER_H
#define SINGLE_CONFIGURATION_MANAGER_H

#include "Configuration.hpp"
#include "ConnectionPoint.h"
#include "Block.hpp"
#include "AngleMapping.h"
#include "ConfigurationEncoder.h"
#include "util/ProgressWriter.hpp"

#include <vector>
#include <set>
#include <map>

class SingleConfigurationManager {
public:
  std::set<uint64_t> nonCyclicConfigurations;
  std::set<Encoding> cyclicConfigurations;

#ifdef _COMPARE_ALGORITHMS
  std::map<FatSCC,uint64_t> foundSCCs; // For debugging only!
#endif

  std::vector<util::TinyVector<AngledConnection, 5> > manual;
private:
  std::vector<Configuration> nrcToPrint; // Used when there are non-rectilinear models, but not multiple non-rectilinear models for a given connection set.
  std::vector<Configuration> modelsToPrint; // Used when there are multiple models for a given connection set then include all models in it - including the RC!
  std::set<uint64_t> investigatedConnectionPairListsEncoded;
  unsigned int combinationSize;
  FatSCC combination[6];
  ConfigurationEncoder encoder;
  std::ofstream &os;
  util::ProgressWriter pw;

  // Used only for construction:
  std::set<ConnectionPoint> above[6], below[6]; 
  bool prevMustBeChosen[6], findExtremeAnglesOnly;

  void run(bool countForPw);
public: // TODO: Only public for testing.
  void run(util::TinyVector<IConnectionPair, 5> &l, const std::vector<IConnectionPoint> &abovePool, const std::vector<IConnectionPoint> &belowPool, bool *remaining, int remainingSize, bool countForPw);

public:
  SingleConfigurationManager(const util::TinyVector<FatSCC, 6> &combination, std::ofstream &os, bool findExtremeAnglesOnly);
  SingleConfigurationManager& operator=(const SingleConfigurationManager &tmp) {
    assert(false); // Assignment operator should not be used.
    util::TinyVector<FatSCC, 6> c;
    SingleConfigurationManager *ret = new SingleConfigurationManager(c, tmp.os, false); // Not deleted - fails on invoce.
    return *ret;
  }

  void run();
  void printMPDFile() const;
  void printManualLDRFiles() const;
  void printManualLDRFile(const std::vector<std::pair<std::string,Configuration> > &v, const std::string &fileName) const;
  bool isRotationallyMinimal(const IConnectionPairSet &l) const;

  counter attempts, models, rectilinear;//, nonRectilinearConfigurations, 
  counter angleMappingBoosts[BOOST_STAGES];
};

#endif // SINGLE_CONFIGURATION_MANAGER_H
