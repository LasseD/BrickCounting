#include "StronglyConnectedConfigurationManager.h"

template <unsigned int ELEMENT_SIZE>
void StronglyConnectedConfigurationList<ELEMENT_SIZE>::writeToDisk(){
  // TODO!
}

template <unsigned int ELEMENT_SIZE>
void StronglyConnectedConfigurationList<ELEMENT_SIZE>::readFromDisk() {
  // TODO!
}

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
  // create 1:
  //l1.s.insert(StronglyConnectedConfiguration<1>());
  std::cout << "Created all combinations of size 1: " << l1 << std::endl;

  // TODO!
}

