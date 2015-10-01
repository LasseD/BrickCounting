#ifndef STRONGLY_CONNECTED_CONFIGURATION_MANAGER_H
#define STRONGLY_CONNECTED_CONFIGURATION_MANAGER_H

#include <set>
#include <iostream>
#include <fstream>
#include "StronglyConnectedConfiguration.hpp"
#include "StronglyConnectedConfigurationList.hpp"

class RectilinearConfigurationManager {
public:
  RectilinearConfigurationManager();
  void create(int maxSize);
  void createOld();
  void loadAllFromDisk();

  void **lists;  

  FatSCC* loadFromFile(int i, unsigned long &size, bool oldSccFile) const;

private:
  RectilinearConfigurationList<1> l1;
  RectilinearConfigurationList<2> l2;
  RectilinearConfigurationList<3> l3;
  RectilinearConfigurationList<4> l4;
  RectilinearConfigurationList<5> l5;
  RectilinearConfigurationList<6> l6;

  void writeToFile(int i, bool old = false);
};

#endif
