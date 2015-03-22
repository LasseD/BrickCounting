#include "Brick.h"
#include "RectilinearConfigurationManager.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>

RectilinearConfigurationManager::RectilinearConfigurationManager(){}

void RectilinearConfigurationManager::create() {
  std::cout << "Running create on StronglyConnectedConfigurationManager" << std::endl;

  // Setup:
  StronglyConnectedConfiguration<1> scc1;
  RectilinearBrick b2(0, 4, 0, false);
  StronglyConnectedConfiguration<2> scc2_1(scc1, b2);

  RectilinearConfiguration rc(scc2_1);

  // Print:
  std::vector<RectilinearConfiguration> s;
  s.push_back(rc);
  printLDRFile(s);
}

void RectilinearConfigurationManager::printLDRFile(const std::vector<RectilinearConfiguration> &s) {
  LDRPrinterHandler h;
  
  std::vector<RectilinearConfiguration>::const_iterator it = s.begin();
  for(int i = 0; it != s.end(); ++it, ++i) {
    LDRPrinter const * p = &(*it);
    h.add(p);
  }
  
  h.print("Test");
}
