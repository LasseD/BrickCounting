#ifndef RECTILINEAR_CONFIGURATION_MANAGER_H
#define RECTILINEAR_CONFIGURATION_MANAGER_H

#include "RectilinearConfiguration.hpp"

#include <vector>

class RectilinearConfigurationManager {
public:
  RectilinearConfigurationManager();

  void create();

  void printLDRFile(const std::vector<RectilinearConfiguration> &s);
};

#endif // RECTILINEAR_CONFIGURATION_MANAGER_H
