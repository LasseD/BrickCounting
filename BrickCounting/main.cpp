#include "RectilinearBrick.h"
#include "StronglyConnectedConfiguration.hpp"
#include "StronglyConnectedConfigurationManager.h"
#include "ConfigurationManager.h"
#include <iostream>
#include <fstream>
#include <assert.h>

void runTestA() {
  /*
  //StronglyConnectedConfigurationManager mgr;
  
  StronglyConnectedConfigurationList<4> list;
  std::set<StronglyConnectedConfiguration<4> > s;
  std::ifstream infile;
  infile.open("old_rc\\4.dat", std::ios::binary | std::ios::in);
  list.deserialize(infile, s);
  std::cout << "Read " << s.size() << " configurations of size 4" << std::endl;
  ConfigurationManager mgr(s);
  mgr.runForSize(4);
  
  //mgr.createOld();
  //*/
}

/*
* MAIN ENTRY POINT!
*/
int main(int cargs, char** args) {  
  ConfigurationManager mgr;
  //mgr.test();
  //mgr.runForSize(2);
  mgr.runForSize(3);
  //mgr.runForSize(4);
  //mgr.runForSize(5);
  //*/
  return 0;
}
