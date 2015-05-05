#ifndef SINGLE_CONFIGURATION_MANAGER_H
#define SINGLE_CONFIGURATION_MANAGER_H

#include "Configuration.hpp"
#include "ConnectionPoint.h"
#include "StronglyConnectedConfiguration.hpp"

#include <vector>
#include <set>

typedef unsigned long long counter;

class SingleConfigurationManager {
  unsigned int combinationSize;
  FatSCC combination[6];
  std::set<ConnectionList> foundConnectionLists;

  // Experimental lookup:
  std::set<uint64_t> foundConnectionsEncoded;
  ConfigurationEncoder encoder;
  //std::set<StronglyConnectedConfiguration<4> > &foundSCCs; // For debugging only!
  //std::set<StronglyConnectedConfiguration<4> > &correct; // For debuggin only!
  std::set<ConnectionPoint> above[6];
  std::set<ConnectionPoint> below[6]; 
  bool prevMustBeChosen[6];

public:
  SingleConfigurationManager(const std::vector<FatSCC> &combination);

  void run(std::vector<Connection> &l, const std::vector<IConnectionPoint> &abovePool, const std::vector<IConnectionPoint> &belowPool, bool *remaining, int remainingSize);
  void run();
  void printLDRFile() const;
  bool isRotationallyMinimal(const ConnectionList &l) const;

  counter attempts, rectilinear, nonRectilinearConnectionLists, models, problematic;
};

#endif // SINGLE_CONFIGURATION_MANAGER_H
