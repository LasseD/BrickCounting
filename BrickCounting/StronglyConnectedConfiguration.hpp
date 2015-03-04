#ifndef STRONGLY_CONNECTED_CONFIGURATION_HPP
#define STRONGLY_CONNECTED_CONFIGURATION_HPP

#include <iostream>
#include <fstream>
#include <algorithm>
#include "RectilinearBrick.h"

template <unsigned int SIZE>
class StronglyConnectedConfiguration {
public:
  RectilinearBrick otherBricks[SIZE-1]; // first brick 0,0, vertical at lv. 0. Bricks sorted.

  StronglyConnectedConfiguration(){}

  /*
    Constructor for constructing a scc from a smaller scc c and a brick b.
    Assumptions about the input: b does not intersect c. b and c are strongly connected.
   */
  StronglyConnectedConfiguration(const StronglyConnectedConfiguration<SIZE-1> &c, const RectilinearBrick &b) {
    if(SIZE == 2) {
      otherBricks[0] = b;
      return;
    }

    int ithis = 0;
    for(int i = 0; i < SIZE-2; ++i) {
      if(b < c.otherBricks[i])
	otherBricks[ithis++] = b;
      otherBricks[ithis++] = c.otherBricks[i];
    }
    if(ithis != SIZE-1)
      otherBricks[SIZE-2] = b;
  }

  /*
    Turns this scc 90 degrees
    Might cause reordering. 
    Causes new brick to be "origin".
    It is assumed that it is possible to turn 90 degrees.
   */
  void turn90() {
    // First turn 90:
    for(int i = 0; i < SIZE-1; ++i) {
      if(otherBricks[i].horizontal) {
	otherBricks[i].horizontal = false;
	int oldX = otherBricks[i].x;
	otherBricks[i].x = otherBricks[i].y;
	otherBricks[i].y = -oldX-2;
      }
      else {
	otherBricks[i].horizontal = true;
	int oldY = otherBricks[i].y;
	otherBricks[i].y = -otherBricks[i].x;
	otherBricks[i].x = oldY;
      }
    }
    
    // Find the new origin:
    int minI = 0;
    for(int i = 1; i < SIZE-1; ++i) {
      if(otherBricks[i] < otherBricks[minI])
	minI = i;
    }

    // Move all according to the new origin:
    int moveX = otherBricks[minI].x;
    int moveY = otherBricks[minI].y;
    for(int i = 1; i < SIZE-1; ++i) {
      otherBricks[i].x -= moveX;
      otherBricks[i].y -= moveY;
    }

    // Re-introduce the turned old origin:
    otherBricks[minI].horizontal = true; 
    otherBricks[minI].x -= moveX;
    otherBricks[minI].y -= moveY;

    // Sort bricks:
    std::sort(otherBricks, &otherBricks[SIZE]);
  }

  void turn180() {
    // First turn all bricks 180:
    for(int i = 0; i < SIZE-1; ++i) {
      if(otherBricks[i].horizontal) {
	otherBricks[i].x = -otherBricks[i].x-2;
	otherBricks[i].y = -otherBricks[i].y+2;
      }
      else {
	otherBricks[i].x = -otherBricks[i].x;
	otherBricks[i].y = -otherBricks[i].y;
      }
    }
    if(SIZE == 2)
      return;
    
    // Find the new origin:
    int minI = 0;
    for(int i = 1; i < SIZE-1; ++i) {
      if(otherBricks[i] < otherBricks[minI])
	minI = i;
    }

    // Move all according to the new origin:
    int moveX = otherBricks[minI].x;
    int moveY = otherBricks[minI].y;
    for(int i = 1; i < SIZE-1; ++i) {
      otherBricks[i].x -= moveX;
      otherBricks[i].y -= moveY;
    }

    // Re-introduce old origin at the brick now representing the new origin:
    otherBricks[minI].x -= moveX;
    otherBricks[minI].y -= moveY;

    // Sort bricks:
    std::sort(otherBricks, &otherBricks[SIZE]);
  }

  bool canTurn90() const {
    if(SIZE <= 2) {
      return false;
    }

    for(int i = 0; i < SIZE-1; ++i) {
      if(otherBricks[i].level > 0)
	return false;
      if(otherBricks[i].horizontal)
	return true;
    }
    return false;
  }

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

  bool intersects(const RectilinearBrick &b) const {
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
