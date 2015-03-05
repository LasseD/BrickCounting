#include "StronglyConnectedConfigurationManager.h"
#include <sstream>

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
  // 1:
  StronglyConnectedConfiguration<1> baseConfiguration;
  l1.s.insert(baseConfiguration);
  std::cout << l1 << std::endl;
  writeToFile(0);

  // 2:
  l2.addAllFor(baseConfiguration);
  std::cout << l2 << std::endl;
  writeToFile(1);

  // 3:
  l3.addAllFor(l2);
  std::cout << l3 << std::endl;
  writeToFile(2);
  //system("pause");

  // 4:
  l4.addAllFor(l3);
  std::cout << "Outputting l4:" << std::endl;
  std::cout << l4 << std::endl;
  writeToFile(3);

  // TODO!
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
