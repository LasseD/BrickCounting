#ifndef STRONGLY_CONNECTED_CONFIGURATION_HPP
#define STRONGLY_CONNECTED_CONFIGURATION_HPP

#include <iostream>
#include <fstream>
#include "RectilinearBrick.h"

template <unsigned int SIZE>
class StronglyConnectedConfiguration {
public:
  RectilinearBrick otherBricks[SIZE-1]; // first brick 0,0, vertical at lv. 0. Bricks sorted.

  StronglyConnectedConfiguration(){}

  bool operator<(const StronglyConnectedConfiguration<SIZE> &c) const {
    for(int i = 0; i < SIZE-1; ++i) {
      if(otherBricks[i] != c.otherBricks[i])
	return otherBricks[i] < c.otherBricks[i];
    }
    return false;
  }

  void serialize(std::ofstream &os) const {
    for(int i = 0; i < SIZE-1; ++i) {
      otherBricks[i].serialize(os);
    }
  }

  void deserialize(std::ifstream &is) {
    for(int i = 0; i < SIZE-1; ++i)
      otherBricks[i].deserialize(is);
  }

  bool intersects(const RectilinearBrick &b) {
    // brick at 0,0,vertical:
    if(b.intersects(RectilinearBrick()))
      return true;
    // All other bricks:
    for(int i = 0; i < SIZE-1; ++i) {
      if(otherBricks[i].intersects(b))
	return true;
    }  
    return false;
  }
};

template <unsigned int SIZE>
std::ostream& operator<<(std::ostream& os, const StronglyConnectedConfiguration<SIZE>& c)
{
  for(int i = 0; i < SIZE-1; ++i)
    os << c.otherBricks[i];
  return os;
}

#endif
