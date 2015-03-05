#include "RectilinearBrick.h"
#include "StronglyConnectedConfiguration.hpp"
#include "StronglyConnectedConfigurationManager.h"
#include <iostream>
#include <fstream>
#include <assert.h>

void runSimpleSerializationTestRectilinearBrick() {
  std::cout << "Running simple serialization test for RectilinearBricks" << std::endl;
  RectilinearBrick a(1,2,3,true);
  RectilinearBrick b(-1,-2,-3,false);

  std::cout << "a and b before serialization: " << a << b << std::endl;

  std::ofstream outfile;
  outfile.open("test.dat", std::ios::binary | std::ios::out);
  a.serialize(outfile);
  b.serialize(outfile);
  outfile.close();

  std::ifstream infile;
  infile.open("test.dat", std::ios::binary | std::ios::in);
  RectilinearBrick c,d;
  c.deserialize(infile);
  d.deserialize(infile);
  infile.close();

  std::cout << "c and d  after serialization: " << c << d << std::endl;  
}

/*
* MAIN ENTRY POINT!
*/
int main(int cargs, char** args) {
  std::cout << "Running strongly connected configurations." << std::endl;

  StronglyConnectedConfigurationManager mgr;
  mgr.create();

  //runSimpleSerializationTestRectilinearBrick();
  return 0;
}
