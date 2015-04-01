#ifndef STRONGLY_CONNECTED_CONFIGURATION_MANAGER_H
#define STRONGLY_CONNECTED_CONFIGURATION_MANAGER_H

#include <set>
#include <iostream>
#include <fstream>
#include "StronglyConnectedConfiguration.hpp"
#include "StronglyConnectedConfigurationList.hpp"

class StronglyConnectedConfigurationManager {
public:
  StronglyConnectedConfigurationManager();
  void create();
  void loadAllFromDisk();

  void **lists;  

  FatSCC* loadFromFile(int i, unsigned long &size) const;

private:
  StronglyConnectedConfigurationList<1> l1;
  StronglyConnectedConfigurationList<2> l2;
  StronglyConnectedConfigurationList<3> l3;
  StronglyConnectedConfigurationList<4> l4;
  StronglyConnectedConfigurationList<5> l5;
  StronglyConnectedConfigurationList<6> l6;

  void writeToFile(int i);
};

#endif
