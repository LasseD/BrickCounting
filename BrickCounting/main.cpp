#include "RectilinearBrick.h"
#include "StronglyConnectedConfiguration.hpp"
#include "StronglyConnectedConfigurationManager.h"
#include "ConfigurationManager.h"
#include <iostream>
#include <fstream>
#include <assert.h>

/*
* MAIN ENTRY POINT!
*/
int main(int cargs, char** args) {  
  // Create SCC data files:
  //StronglyConnectedConfigurationManager mgr;
  //mgr.create();

  // Rund RC:
  ConfigurationManager mgr;
  //mgr.runForSize(2);
  //mgr.runForSize(3);
  //mgr.runForSize(4);
  //mgr.runForSize(5);
  mgr.runForSize(6);
  return 0;
}
