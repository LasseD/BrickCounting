#ifndef STRONGLY_CONNECTED_CONFIGURATION_HPP
#define STRONGLY_CONNECTED_CONFIGURATION_HPP

#include "LDRPrinter.h"
#include "RectilinearBrick.h"
#include "Brick.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <assert.h> 
#include <set> 

class ISCC {
public:
  virtual void getConnectionPoints(std::set<ConnectionPoint> &above, std::set<ConnectionPoint> &below) const = 0;
};

template <unsigned int SIZE>
class StronglyConnectedConfiguration : public LDRPrinter, public ISCC {
public:
  RectilinearBrick otherBricks[SIZE-1]; // first brick 0,0, vertical at lv. 0. Bricks sorted.

  bool verify() const {
    const RectilinearBrick origin;
    //bool originIntersected = false;
    for(int i = 0; i < SIZE-1; ++i) {
      if(otherBricks[i] < origin ||
	 otherBricks[i].x < -15 || otherBricks[i].x > 15 || otherBricks[i].y < -15 || otherBricks[i].y > 15) {
	std::cerr << "Verification failed for " << *this << ": Following brick is illegal: " << otherBricks[i] << std::endl;
        return false;
      }
      //if(origin.intersects(otherBricks[i]))
      //originIntersected = true;
    }
    //if(!originIntersected)
    //std::cerr << "Verification failed for " << *this << ": Origin not intersected!" << std::endl;
    return true;
  }

  void ensureOriginIsSmallest() {
    int minI = -1;
    RectilinearBrick min; // origin.
    for(int i = 0; i < SIZE-1; ++i) {
      if(otherBricks[i] < min) {
        minI = i;
        min = otherBricks[i];
      }
    }

    if(minI != -1) {
      // Move all according to the new origin:
      for(int i = 0; i < SIZE-1; ++i) {
        otherBricks[i].x -= min.x;
        otherBricks[i].y -= min.y;
      }

      // Re-introduce old origin at the brick now representing the new origin:
      otherBricks[minI].x -= min.x;
      otherBricks[minI].y -= min.y;
      std::sort(otherBricks, &otherBricks[SIZE-1]);
    }
  }

  StronglyConnectedConfiguration(){}
  StronglyConnectedConfiguration(const StronglyConnectedConfiguration& c) {
    for(int i = 0; i < SIZE-1; ++i) {
      otherBricks[i] = c.otherBricks[i];
    }
  }
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
      if(i == ithis && b < c.otherBricks[i])
        otherBricks[ithis++] = b;
      otherBricks[ithis++] = c.otherBricks[i];
    }
    if(ithis == SIZE-2)
      otherBricks[SIZE-2] = b;

    ensureOriginIsSmallest();
    assert(verify());
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
      int8_t oldX = otherBricks[i].x;
      otherBricks[i].x = otherBricks[i].y;
      otherBricks[i].y = -oldX;
      otherBricks[i].flipHorizontal();
    }

    // Find the new origin:
    int minI = 0;
    for(int i = 0; i < SIZE-1; ++i) {
      if(otherBricks[i] < otherBricks[minI])
        minI = i;
    }

    // Move all according to the new origin:
    int8_t moveX = otherBricks[minI].x;
    int8_t moveY = otherBricks[minI].y;
    for(int i = 0; i < SIZE-1; ++i) {
      otherBricks[i].x -= moveX;
      otherBricks[i].y -= moveY;
    }

    // Re-introduce the turned old origin:
    otherBricks[minI].setHorizontalTrue(); 
    otherBricks[minI].x -= moveX;
    otherBricks[minI].y -= moveY;

    // Sort bricks:
    std::sort(otherBricks, &otherBricks[SIZE-1]);
    assert(verify());
  }

  void turn180() {
    // First turn all bricks 180:
    for(int i = 0; i < SIZE-1; ++i) {
      otherBricks[i].x = -otherBricks[i].x;
      otherBricks[i].y = -otherBricks[i].y;
    }
    if(SIZE == 2)
      return;

    // Find the new origin:
    ensureOriginIsSmallest();
    //std::cout << "ensured " << *this << std::endl;      

    // Sort bricks:
    std::sort(otherBricks, &(otherBricks[SIZE-1]));
    //std::cout << "end check " << *this << std::endl;      
    assert(verify());
  }

  bool canTurn90() const {
    if(SIZE <= 2) {
      return false;
    }

    for(int i = 0; i < SIZE-1; ++i) {
      if(otherBricks[i].level() > 0)
        return false;
      if(otherBricks[i].horizontal())
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

  // O(SIZE) algorithm.
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

  void toLDR(std::ofstream &os, int x, int y, int ldrColor) const {
    RectilinearBrick().toLDR(os, x, y, ldrColor);
    if(SIZE < 2)
      return;
    for(int i = 0; i < SIZE-1; ++i) {
      otherBricks[i].toLDR(os, x, y, ldrColor);
    }
  }

  int height() const {
    int res = 1;
    for(int i = 0; i < SIZE-1; ++i) {
      if(otherBricks[i].level() > res)
	res = otherBricks[i].level();
    }
    return res+1;
  }

  unsigned int layerHash() const {
    // Initialize layers:
    unsigned int layerSizes[SIZE] = {};

    // count:
    for(int i = 0; i < SIZE-1; ++i)
      ++layerSizes[otherBricks[i].level()];

    // encode:
    unsigned int res = 0;
    for(int i = 0; i < SIZE; ++i) {
      res *= 10;
      res += layerSizes[i];
    }
    return res;
  }

  bool angleLocked(const ConnectionPoint &p) const {
    RectilinearBrick b;
    for(int i = 0; i < SIZE; b=otherBricks[i++]) {
      if(b.angleLocks(p))
	return true;
    }
    return false;    
  }
  bool blocked(const ConnectionPoint &p) const {
    RectilinearBrick b;
    for(int i = 0; i < SIZE; b=otherBricks[i++]) {
      if(b.blocks(p))
	return true;
    }
    return false;
  }

  void getConnectionPoints(std::set<ConnectionPoint> &above, std::set<ConnectionPoint> &below) const {
    // Find bricks that might block every level:
    std::set<RectilinearBrick> blockers[SIZE];
    {
      RectilinearBrick b; // using block so that b can be re-used below.
      for(int i = 0; i < SIZE; b=otherBricks[i++]) {
	blockers[b.level()].insert(b);
      }
    }

    // Get all possible connection points and add them to sets (unless blocked).
    ConnectionPoint tmp[SIZE];
    RectilinearBrick b;
    for(int i = 0; i < SIZE; b=otherBricks[i++]) {
      int four = 0;
      b.getConnectionPointsAbove(tmp, four);
      for(int j = 0; j < four; ++j) {
	if(!blocked(tmp[j]))
	  above.insert(tmp[j]);
      }
      four = 0;
      b.getConnectionPointsBelow(tmp, four);
      for(int j = 0; j < four; ++j) {
	if(above.find(tmp[j]) == above.end() && !blocked(tmp[j]))
	  below.insert(tmp[j]);
      }
    }    
  }
};

template <>
class StronglyConnectedConfiguration<0> {
};

template <unsigned int SIZE>
std::ostream& operator<<(std::ostream& os, const StronglyConnectedConfiguration<SIZE>& c)
{
  for(int i = 0; i < SIZE-1; ++i)
    os << c.otherBricks[i];
  return os;
}

#endif
