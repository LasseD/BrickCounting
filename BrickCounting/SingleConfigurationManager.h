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
  std::vector<std::vector<Connection> > manual;
  std::vector<Configuration> nrcToPrint; // Used when there are non-rectilinear models, but not multiple non-rectilinear models for a given connection set.
  std::vector<Configuration> modelsToPrint; // Used when there are multiple models for a given connection set then include all models in it - including the RC!
  std::set<uint64_t> investigatedConnectionPairListsEncoded;
  unsigned int combinationSize;
  FatSCC combination[6];
  ConfigurationEncoder encoder;
  std::ofstream &os;
  
  //std::set<IConnectionPairList> investigatedIConnectionPairLists; // For debugging only!
  //std::set<FatSCC> foundSCCs; // For debugging only!
  //std::map<uint64_t,FatSCC> foundSCCsMap; // For debugging only!
  //std::set<StronglyConnectedConfiguration<3> > &correct; // For debuggin only!

  // Used only for construction:
  std::set<ConnectionPoint> above[6], below[6]; 
  bool prevMustBeChosen[6];

public:
  SingleConfigurationManager(const std::vector<FatSCC> &combination, std::ofstream &os);
  SingleConfigurationManager& operator=(const SingleConfigurationManager &tmp) {
    assert(false); // Assignment operator should not be used.
    std::vector<FatSCC> c;
    SingleConfigurationManager *ret = new SingleConfigurationManager(c, tmp.os);
    return *ret;
  }

  void run(std::vector<IConnectionPair> &l, const std::vector<IConnectionPoint> &abovePool, const std::vector<IConnectionPoint> &belowPool, bool *remaining, int remainingSize);
  void run();
  void printLDRFile(bool selectNrc) const;
  void printManualLDRFiles() const;
  void printManualLDRFile(const std::vector<std::pair<std::string,Configuration> > &v, const std::string &fileName) const;
  bool isRotationallyMinimal(const IConnectionPairList &l) const;

  counter attempts, models, problematic; // rectilinear, nonRectilinearConfigurations, 
  counter angleMappingBoosts[BOOST_STAGES];
};

#endif // SINGLE_CONFIGURATION_MANAGER_H
