#ifndef STRONGLY_CONNECTED_CONFIGURATION_MANAGER_H
#define STRONGLY_CONNECTED_CONFIGURATION_MANAGER_H

#include <set>
#include <iostream>
#include "StronglyConnectedConfiguration.h"

template <unsigned int ELEMENT_SIZE>
class StronglyConnectedConfigurationList {
 public:
  void writeToDisk();
  void readFromDisk();
  StronglyConnectedConfiguration<ELEMENT_SIZE> *v; // used sorted for retrieval.
  std::set<StronglyConnectedConfiguration<ELEMENT_SIZE> > s; // only used for construction.
};

template <unsigned int ELEMENT_SIZE>
std::ostream& operator<<(std::ostream& os, const StronglyConnectedConfigurationList<ELEMENT_SIZE>& l)
{
  os << (uint64_t)l.s.size();
  for(typename std::set<StronglyConnectedConfiguration<ELEMENT_SIZE> >::const_iterator it = l.s.begin(); it != l.s.end(); ++it)
    os << *it;
  return os;
}
template <unsigned int ELEMENT_SIZE>
std::istream& operator>>(std::istream& is, StronglyConnectedConfigurationList<ELEMENT_SIZE>& l)
{
  uint64_t size;
  is >> size;
  l.v = new StronglyConnectedConfiguration<ELEMENT_SIZE>[size];
  for(int i = 0; i < size; ++i) {
    is >> l.v[i];
  }
  
  return is;
}

class StronglyConnectedConfigurationManager {
 public:
  StronglyConnectedConfigurationManager();
  void create();

  void **lists;  

 private:
  StronglyConnectedConfigurationList<1> l1;
  StronglyConnectedConfigurationList<2> l2;
  StronglyConnectedConfigurationList<3> l3;
  StronglyConnectedConfigurationList<4> l4;
  StronglyConnectedConfigurationList<5> l5;
  StronglyConnectedConfigurationList<6> l6;
};

#endif
