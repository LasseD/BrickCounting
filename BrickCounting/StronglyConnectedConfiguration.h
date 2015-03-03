#ifndef STRONGLY_CONNECTED_CONFIGURATION_H
#define STRONGLY_CONNECTED_CONFIGURATION_H

#include <iostream>
#include "RectilinearBrick.h"

template <unsigned int SIZE>
class StronglyConnectedConfiguration
{
public:
  RectilinearBrick otherBricks[SIZE-1]; // first brick 0,0, vertical(angle 0) at lv. 0. Bricks sorted.
  StronglyConnectedConfiguration(){}
};

template <unsigned int SIZE>
std::ostream& operator<<(std::ostream& os, const StronglyConnectedConfiguration<SIZE>& s)
{
  for(int i = 0; i < SIZE-1; ++i)
    os << s.otherBricks[i];
  return os;
}

template <unsigned int SIZE>
std::istream& operator>>(std::istream& is, StronglyConnectedConfiguration<SIZE>& s)
{
  for(int i = 0; i < SIZE-1; ++i)
    is >> s.otherBricks[i];
  return is;
}

#endif
