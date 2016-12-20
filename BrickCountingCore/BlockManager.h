#ifndef STRONGLY_CONNECTED_CONFIGURATION_MANAGER_H
#define STRONGLY_CONNECTED_CONFIGURATION_MANAGER_H

#include <set>
#include <iostream>
#include <fstream>
#include "Block.hpp"
#include "BlockList.hpp"

class RectilinearConfigurationManager {
public:
  RectilinearConfigurationManager();
  ~RectilinearConfigurationManager();
  void create(int maxSize);
  void createOld();
  void loadAllFromDisk();

  void **lists;  

  FatSCC* loadFromFile(int i, unsigned long &size, bool oldSccFile) const;
  //FatSCC* loadFromFile(std::vector<int> combinationType, unsigned long &size) const;
  void loadFromFile(std::set<FatSCC> &s, std::vector<int> combinationType) const;

private:
  FatSCC* loadFromFile(std::string fileName, int i, unsigned long &size) const;

  RectilinearConfigurationList<1> l1;
  RectilinearConfigurationList<2> l2;
  RectilinearConfigurationList<3> l3;
  RectilinearConfigurationList<4> l4;
  RectilinearConfigurationList<5> l5;
  RectilinearConfigurationList<6> l6;

  void writeToFile(int i, bool old = false);
};

#endif
