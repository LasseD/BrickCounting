#ifndef STRONGLY_CONNECTED_CONFIGURATION_LIST_HPP
#define STRONGLY_CONNECTED_CONFIGURATION_LIST_HPP

#include <set>
#include <iostream>
#include <fstream>
#include "StronglyConnectedConfiguration.hpp"

template <unsigned int ELEMENT_SIZE>
class StronglyConnectedConfigurationList {
 public:
  StronglyConnectedConfiguration<ELEMENT_SIZE> *v; // used sorted for retrieval.
  std::set<StronglyConnectedConfiguration<ELEMENT_SIZE> > s; // only used for construction.

  void serialize(std::ofstream &os) {
    unsigned long size = s.size();
    os.write(reinterpret_cast<const char *>(&size), sizeof(unsigned long));

    typename std::set<StronglyConnectedConfiguration<ELEMENT_SIZE> >::const_iterator it;
    for(it = s.begin(); it != s.end(); ++it) {
      it->serialize(os);
    }
  }
  void deserialize(std::ifstream &is) {
    unsigned long size;
    is.read((char*)&size, sizeof(unsigned long));
    v = new StronglyConnectedConfiguration<ELEMENT_SIZE>[size];
    for(int i = 0; i < size; ++i) {
      v[i].deserialize(is);
    }
  }
};

template <unsigned int ELEMENT_SIZE>
std::ostream& operator<<(std::ostream& os, const StronglyConnectedConfigurationList<ELEMENT_SIZE>& l) {
  os << (uint64_t)l.s.size();

  typename std::set<StronglyConnectedConfiguration<ELEMENT_SIZE> >::const_iterator it = l.s.begin();
  for(; it != l.s.end(); ++it)
    os << *it;
  return os;
};

#endif
