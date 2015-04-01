#include "StronglyConnectedConfigurationManager.h"
#include <sstream>
#include <windows.h>

StronglyConnectedConfigurationManager::StronglyConnectedConfigurationManager() {
  lists = new void*[6];
  lists[0] = &l1;
  lists[1] = &l2;
  lists[2] = &l3;
  lists[3] = &l4;
  lists[4] = &l5;
  lists[5] = &l6;
}

void StronglyConnectedConfigurationManager::create() {
  // First. Ensure output folder exists:
  CreateDirectory("scc", NULL);

  // 1:
  StronglyConnectedConfiguration<1> baseConfiguration;
  l1.s.insert(baseConfiguration);
  std::cout << l1 << std::endl;
  writeToFile(0);

  l1.printLDRFile();

  // 2:
  l2.addAllFor(baseConfiguration);
  //l1.s.clear();
  std::cout << l2 << std::endl;
  writeToFile(1);

  l2.printLDRFile();
  std::cout << "DONE STRONGLY CONNECTED CONFIGURATIONS OF SIZE 2" << std::endl << std::endl;

  // 3:
  l3.addAllFor(l2);
  l2.s.clear();
  std::cout << l3 << std::endl;
  writeToFile(2);

  l3.printLDRFile();
  std::cout << "DONE STRONGLY CONNECTED CONFIGURATIONS OF SIZE 3" << std::endl << std::endl;

  // 4:
  l4.addAllFor(l3);
  l3.s.clear();
  std::cout << l4 << std::endl;
  writeToFile(3);
  std::cout << "DONE STRONGLY CONNECTED CONFIGURATIONS OF SIZE 4" << std::endl << std::endl;

  //l4.countAllFor(l3);
  //return;

  // 5:
  l5.addAllFor(l4);
  l4.s.clear();
  std::cout << l5 << std::endl;
  writeToFile(4);
  std::cout << "DONE STRONGLY CONNECTED CONFIGURATIONS OF SIZE 5" << std::endl << std::endl;
  //system("cmd /C pause");
  //system("pause");
  //l5.countAllFor(l4);

  // 6:
  l6.countAllFor(l5);
}

void StronglyConnectedConfigurationManager::writeToFile(int i) {
  std::ofstream os;
  std::stringstream ss;
  ss << "scc/" << (i+1) << ".dat";
  os.open(ss.str().c_str(), std::ios::binary | std::ios::out);
  std::cout << "Writing file with strongly connected configurations of size " << (i+1) << " to " << ss.str() << std::endl;

  switch(i) {
  case 0:    
    l1.serialize(os);
    break;
  case 1:
    l2.serialize(os);
    break;
  case 2:
    l3.serialize(os);
    break;
  case 3:
    l4.serialize(os);
    break;
  case 4:
    l5.serialize(os);
    break;
  case 5:
    l6.serialize(os);
    break;
  }
  os.close();
}

FatSCC* StronglyConnectedConfigurationManager::loadFromFile(int i, unsigned long &size) const {
  std::ifstream is;
  std::stringstream ss;
  ss << "scc/" << (i+1) << ".dat";
  is.open(ss.str().c_str(), std::ios::binary | std::ios::in);
  std::cout << "Reading file with strongly connected configurations of size " << (i+1) << " from " << ss.str() << std::endl;

  FatSCC* ret;
  switch(i) {
  case 0:    
    ret = l1.deserialize(is, size);
    break;
  case 1:
    ret = l2.deserialize(is, size);
    break;
  case 2:
    ret = l3.deserialize(is, size);
    break;
  case 3:
    ret = l4.deserialize(is, size);
    break;
  case 4:
    ret = l5.deserialize(is, size);
    break;
  default:
    std::cerr << "ERROR: StronglyConnectedConfigurationManager::loadFromFile() called on " << i << std::endl;
    break;
  }
  is.close();
  
  std::cout << "Read " << size << " strongly connected configurations of size " << (i+1) << " from " << ss.str() << std::endl;
  return ret;
}
