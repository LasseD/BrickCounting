#ifndef SINGLE_CONFIGURATION_MANAGER_H
#define SINGLE_CONFIGURATION_MANAGER_H

#include "Configuration.hpp"
#include "ConnectionPoint.h"
#include "StronglyConnectedConfiguration.hpp"
#include "AngleMapping.h"
#include "ConfigurationEncoder.h"

#include <vector>
#include <set>
#include <map>

class SingleConfigurationManager {
public:
  std::set<Encoding> foundRectilinearConfigurationsEncoded; // Both with and without cycles
  std::set<Encoding> foundNonRectilinearConfigurationsEncoded; // Both with and without cycles
private:
  std::vector<Configuration> configurationsToWriteToLdr;
  std::set<uint64_t> investigatedConnectionPairListsEncoded;
  unsigned int combinationSize;
  FatSCC combination[6];
  ConfigurationEncoder encoder;
  
  //std::set<IConnectionPairList> investigatedIConnectionPairLists; // For debugging only!
  //std::set<FatSCC> foundSCCs; // For debugging only!
  //std::map<uint64_t,FatSCC> foundSCCsMap; // For debugging only!
  //std::set<StronglyConnectedConfiguration<3> > &correct; // For debuggin only!

  // Used only for construction:
  std::set<ConnectionPoint> above[6], below[6]; 
  bool prevMustBeChosen[6];

public:
  SingleConfigurationManager(const std::vector<FatSCC> &combination);

  void run(std::vector<IConnectionPair> &l, const std::vector<IConnectionPoint> &abovePool, const std::vector<IConnectionPoint> &belowPool, bool *remaining, int remainingSize);
  void run();
  void printLDRFile() const;
  bool isRotationallyMinimal(const IConnectionPairList &l) const;

  counter attempts, models, problematic; // rectilinear, nonRectilinearConfigurations, 
};

#endif // SINGLE_CONFIGURATION_MANAGER_H
