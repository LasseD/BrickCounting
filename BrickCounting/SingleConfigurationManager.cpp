#include "SingleConfigurationManager.h"

SingleConfigurationManager::SingleConfigurationManager(){}

/*
void RectilinearConfigurationManager::test() const {
  // Setup:
  RectilinearBrick b1;
  StronglyConnectedConfiguration<1> scc1;
  RectilinearBrick b2(3, -1, 0, true); // true for horizontal
  StronglyConnectedConfiguration<2> scc2(scc1, b2);
  RectilinearBrick b3(0, 4, 0, false);
  StronglyConnectedConfiguration<3> scc3(scc2, b3);//*/

  // NW: Axle, NE: Hose, SE: PIN
/*
  RectilinearConfiguration rc(scc3);
  std::cout << "Constructing and adding yellow brick: " << std::endl;
  rc.add(scc1, IConnection(0, Connection(ConnectionPoint(NW, b3, true), ConnectionPoint(SE, b1, false))));
  /*std::cout << "Constructing and adding black brick: " << std::endl;
  rc.add(scc1, IConnection(1, Connection(ConnectionPoint(SW, b1, true), ConnectionPoint(SW, b1, false))));
  /*std::cout << "Constructing and adding blue brick: " << std::endl;
  rc.add(scc1, IConnection(0, Connection(ConnectionPoint(SE, b1, true), ConnectionPoint(NW, b1, false))));
  std::cout << "Constructing and adding green brick: " << std::endl;
  rc.add(scc1, IConnection(0, Connection(ConnectionPoint(SW, b1, true), ConnectionPoint(NE, b1, false))));
  //*/
/*
  // Print:
  std::vector<RectilinearConfiguration> s;
  s.push_back(rc);
  printLDRFile(s);
}

void RectilinearConfigurationManager::create() {
  std::cout << "Running create on StronglyConnectedConfigurationManager" << std::endl;

}

void RectilinearConfigurationManager::printLDRFile(const std::vector<RectilinearConfiguration> &s) const {
  LDRPrinterHandler h;
  
  std::vector<RectilinearConfiguration>::const_iterator it = s.begin();
  for(int i = 0; it != s.end(); ++it, ++i) {
    LDRPrinter const * p = &(*it);
    h.add(p);
  }
  
  h.print("rc/Test");
}//*/
