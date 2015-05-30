#ifndef STRONGLY_CONNECTED_CONFIGURATION_HPP
#define STRONGLY_CONNECTED_CONFIGURATION_HPP

#include "LDRPrinter.h"
#include "RectilinearBrick.h"
#include "ConnectionPoint.h"
#include "Brick.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <assert.h> 
#include <set> 
#include <limits.h>

#define NO_INDEX ULONG_MAX

template <unsigned int SIZE>
class StronglyConnectedConfiguration : public LDRPrinter {
public:
  RectilinearBrick otherBricks[SIZE-1]; // first brick 0,0, vertical at lv. 0. Bricks sorted.

  bool verify() const {
    const RectilinearBrick origin;
    for(int i = 0; i < SIZE-1; ++i) {
      if(otherBricks[i] < origin ||
        otherBricks[i].x < -15 || otherBricks[i].x > 15 || otherBricks[i].y < -15 || otherBricks[i].y > 15) {
          std::cerr << "Verification failed for " << *this << ": Following brick is illegal: " << otherBricks[i] << std::endl;
          return false;
      }
      if(origin.intersects(otherBricks[i])) {
        std::cerr << "Verification failed for " << *this << ": Origin intersected!" << std::endl;
        return false;
      }
    }
    return true;
  }

  /*
  Return true if changed.
  */
  bool ensureOriginIsSmallest() {
    int minI = -1;
    RectilinearBrick min; // origin.
    for(int i = 0; i < SIZE-1; ++i) {
      if(otherBricks[i] < min) {
        minI = i;
        min = otherBricks[i];
      }
    }

    if(minI == -1) 
      return false;

    // Move all according to the new origin:
    for(int i = 0; i < SIZE-1; ++i) {
      otherBricks[i].x -= min.x;
      otherBricks[i].y -= min.y;
    }

    // Re-introduce old origin at the brick now representing the new origin:
    otherBricks[minI].x -= min.x;
    otherBricks[minI].y -= min.y;
    std::sort(otherBricks, &otherBricks[SIZE-1]);
    return true;
  }

  StronglyConnectedConfiguration() {}
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
  1,2 -> 2,-1 -> -1,-2
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
    for(int i = 1; i < SIZE-1; ++i) {
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

    // Find the new origin if necessary. If no change: sort.:
    if(!ensureOriginIsSmallest())
      std::sort(otherBricks, &(otherBricks[SIZE-1]));

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

  std::pair<int,int> rotationBrickPosition() const {
    if(SIZE == 1)
      return std::make_pair(0,0);
    RectilinearBrick furthest;
    for(int i = 0; i < SIZE-1; ++i) {
      if(otherBricks[i].level() > 0)
        break;
      if(otherBricks[i].horizontal())
        continue; // horizontal first - ignore these!
      furthest = otherBricks[i];
    }
    return std::make_pair(furthest.x, furthest.y);
  }

  bool operator<(const StronglyConnectedConfiguration<SIZE> &c) const {
    for(int i = 0; i < SIZE-1; ++i) {
      if(otherBricks[i] != c.otherBricks[i])
        return otherBricks[i] < c.otherBricks[i];
    }
    return false;
  }

  bool operator==(const StronglyConnectedConfiguration<SIZE> &c) const {
    for(int i = 0; i < SIZE-1; ++i) {
      if(otherBricks[i] != c.otherBricks[i])
        return false;
    }
    return true;
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
  bool intersects(const RectilinearBrick &brick) const {
    RectilinearBrick b;
    for(int i = 0; i < SIZE; b = otherBricks[i++]) {
      if(b.intersects(brick))
        return true;
    }
    return false;
  }

  void toLDR(std::ofstream &os, int x, int y, int ldrColor) const {
    RectilinearBrick b;
    for(int i = 0; i < SIZE; b = otherBricks[i++]) {
      b.toLDR(os, x, y, ldrColor);
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

  unsigned long layerHash() const {
    // Initialize layers:
    unsigned int hLayerSizes[SIZE] = {};
    unsigned int vLayerSizes[SIZE] = {};

    // count:
    RectilinearBrick b;
    for(int i = 0; i < SIZE; b = otherBricks[i++]) {
      if(b.horizontal())
        ++hLayerSizes[b.level()];
      else
        ++vLayerSizes[b.level()];
    }

    // encode:
    unsigned long res = 0;
    for(int i = 0; i < SIZE; ++i) {
      res *= 10;
      if(vLayerSizes[i] == hLayerSizes[i]) {
        res += hLayerSizes[i]+vLayerSizes[i]+5;	
      }
      else {
        res += hLayerSizes[i]+vLayerSizes[i];
      }
    }
    return res;
  }

  bool inSet(const std::set<StronglyConnectedConfiguration<SIZE> > &s) {
    if(s.find(*this) != s.end()) {
      return true;
    }
    if(this->canTurn90()) {
      for(int i = 0; i < 3; ++i) {
        this->turn90();
        if(s.find(*this) != s.end()) {
          return true;
        }
      }
      return false;
    }
    else {
      this->turn180();
      return s.find(*this) != s.end();
    }
  }
};

template <unsigned int SIZE>
std::ostream& operator<<(std::ostream& os, const StronglyConnectedConfiguration<SIZE>& c)
{
  for(int i = 0; i < SIZE-1; ++i)
    os << c.otherBricks[i];
  return os;
}

class FatSCC {
public:
  int size;
  unsigned long index;
  RectilinearBrick otherBricks[4];
  bool isRotationallySymmetric;
  std::pair<int,int> rotationBrickPosition;

  FatSCC() {}
  FatSCC(const FatSCC &f) : size(f.size), index(f.index), isRotationallySymmetric(f.isRotationallySymmetric), rotationBrickPosition(f.rotationBrickPosition) {
    for(int i = 0; i < size-1; ++i) {
      otherBricks[i] = f.otherBricks[i];
    }
  }

  template <unsigned int SIZE>
  FatSCC(const StronglyConnectedConfiguration<SIZE>& scc, unsigned long index) : size(SIZE), index(index) {
    for(int i = 0; i < SIZE-1; ++i) {
      otherBricks[i] = scc.otherBricks[i];
    }
    StronglyConnectedConfiguration<SIZE> turned(scc);
    turned.turn180();
    isRotationallySymmetric = (turned == scc);
    rotationBrickPosition = scc.rotationBrickPosition();
  }

  FatSCC(std::vector<Brick> v) : size((int)v.size()), index(NO_INDEX), isRotationallySymmetric(false), rotationBrickPosition(std::make_pair(0,0)) {
    //std::cout << "CONVERTING!" << std::endl;
    // Ensure level starts at 0:
    int8_t minLv = 99;
    for(std::vector<Brick>::iterator it = v.begin(); it != v.end(); ++it) {
      if(it->level < minLv)
        minLv = it->level;
    }
    for(std::vector<Brick>::iterator it = v.begin(); it != v.end(); ++it) {
      it->level-=minLv;
    }
    //std::cout << " min level: " << minLv << std::endl;

    // Turn into RectilinearBricks and find min:
    std::vector<RectilinearBrick> rbricks;
    RectilinearBrick min = v.begin()->toRectilinearBrick();
    for(std::vector<Brick>::iterator it = v.begin(); it != v.end(); ++it) {
      RectilinearBrick rb = it->toRectilinearBrick();
      if(rb < min)
        min = rb;
      rbricks.push_back(rb);
      //std::cout << "  - " << *it << " -> " << rb << std::endl;
    }
    //std::cout << " min: " << min << std::endl;

    // Turn 90 degrees if min is vertical:
    if(min.horizontal()) {
      //std::cout << " Turning 90 degrees." << std::endl;
      for(std::vector<RectilinearBrick>::iterator it = rbricks.begin(); it != rbricks.end(); ++it) {
        int8_t oldX = it->x;
        it->x = it->y;
        it->y = -oldX;
        it->flipHorizontal();
      }

      // Find the origin:
      min = *rbricks.begin();
      for(std::vector<RectilinearBrick>::iterator it = rbricks.begin(); it != rbricks.end(); ++it) {
        if(*it < min) {
          min = *it;
        }
      }
    }

    // Move all according to the origin:
    int8_t moveX = min.x;
    int8_t moveY = min.y;
    for(std::vector<RectilinearBrick>::iterator it = rbricks.begin(); it != rbricks.end(); ++it) {
      it->x -= moveX;
      it->y -= moveY;
    }

    // Sort and save bricks:
    std::sort(rbricks.begin(), rbricks.end());
    int i = -1;
    for(std::vector<RectilinearBrick>::iterator it = rbricks.begin(); it != rbricks.end(); ++it, ++i) {
      if(i == -1)
        continue; // don't save origin.
      otherBricks[i] = *it;
    }
    /*std::cout << " Resulting rbs: " << std::endl;
    for(i = 0; i < size-1; ++i) {
    std::cout << "  - " << otherBricks[i] << std::endl;
    }//*/
  }

  template <unsigned int SIZE>
  StronglyConnectedConfiguration<SIZE> toSCC() const {
    StronglyConnectedConfiguration<SIZE> ret;
    for(int i = 0; i < SIZE-1; ++i) {
      ret.otherBricks[i] = otherBricks[i];
    }
    return ret;
  }

  bool angleLocked(const ConnectionPoint &p) const {
    RectilinearBrick b;
    for(int i = 0; i < size; b=otherBricks[i++]) {
      if(b.angleLocks(p))
        return true;
    }
    return false;
  }
  bool blocked(const ConnectionPoint &p) const {
    RectilinearBrick b;
    for(int i = 0; i < size; b=otherBricks[i++]) {
      if(b.blocks(p))
        return true;
    }
    return false;
  }

  void getConnectionPoints(std::set<ConnectionPoint> &above, std::set<ConnectionPoint> &below) const {
    //std::cout << "Connection points for " << size << "," << index << ":" << std::endl;
    // Find bricks that might block every level:
    std::set<RectilinearBrick> blockers[6];
    { // using block so that b can be re-used below.
      RectilinearBrick b;
      for(int i = 0; i < size; b=otherBricks[i++]) {
        blockers[b.level()].insert(b);
      }
    }

    // Get all possible connection points and add them to sets (unless blocked).
    ConnectionPoint tmp[4];
    RectilinearBrick b;
    for(int i = 0; i < size; b=otherBricks[i++]) {
      b.getConnectionPointsAbove(tmp, i);
      for(int j = 0; j < 4; ++j) {
        if(!blocked(tmp[j])) {
          above.insert(tmp[j]);
          //std::cout << " - " << tmp[j] << std::endl;
        }
      }
      b.getConnectionPointsBelow(tmp, i);
      for(int j = 0; j < 4; ++j) {
        if(!blocked(tmp[j])) {
          below.insert(tmp[j]);
          //std::cout << " - " << tmp[j] << std::endl;
        }
      }
    }    
  }

  bool isRotationallyMinimal(const std::vector<ConnectionPoint> &pointsForSccs) const {
    if(!isRotationallySymmetric)
      return true; // not applicaple.
    std::vector<ConnectionPoint> rotatedPoints;
    for(std::vector<ConnectionPoint>::const_iterator it = pointsForSccs.begin(); it != pointsForSccs.end(); ++it) {
      ConnectionPoint rotatedPoint(*it, rotationBrickPosition);
      rotatedPoints.push_back(rotatedPoint);
    }
    std::sort(rotatedPoints.begin(), rotatedPoints.end());

    for(std::vector<ConnectionPoint>::const_iterator it1 = pointsForSccs.begin(), it2 = rotatedPoints.begin(); it1 != pointsForSccs.end(); ++it1, ++it2) {
      if(*it2 < *it1) {
        //std::cout << "Not rotationally minimal because " << *it2 << "<" << *it1 << " when rotated on " << rotationBrickPosition.X << "," << rotationBrickPosition.Y << std::endl;
        return false;
      }
      if(*it1 < *it2) {
        return true;
      }
    }
    return true;
  }

  void getRotationalInformation(const std::vector<ConnectionPoint> &pointsForSccs, bool &minimal, bool &rotSym) const {
    minimal = true;
    if(!isRotationallySymmetric) {
      rotSym = false;
      return;
    }
    std::vector<ConnectionPoint> rotatedPoints;
    for(std::vector<ConnectionPoint>::const_iterator it = pointsForSccs.begin(); it != pointsForSccs.end(); ++it) {
      ConnectionPoint rotatedPoint(*it, rotationBrickPosition);
      rotatedPoints.push_back(rotatedPoint);
    }
    std::sort(rotatedPoints.begin(), rotatedPoints.end());
    rotSym = true;

    for(std::vector<ConnectionPoint>::const_iterator it1 = pointsForSccs.begin(), it2 = rotatedPoints.begin(); it1 != pointsForSccs.end(); ++it1, ++it2) {
      if(*it2 < *it1) {
        //std::cout << "Not rotationally minimal because " << *it2 << "<" << *it1 << " when rotated on " << rotationBrickPosition.X << "," << rotationBrickPosition.Y << std::endl;
	rotSym = false;
	minimal = false;
      }
      if(*it1 < *it2) {
	rotSym = false;
      }
    }
  }

  bool isRotationallyIdentical(const std::vector<ConnectionPoint> &pointsForSccs) const {
    if(!isRotationallySymmetric)
      return false; // not applicaple.
    std::vector<ConnectionPoint> rotatedPoints;
    for(std::vector<ConnectionPoint>::const_iterator it = pointsForSccs.begin(); it != pointsForSccs.end(); ++it) {
      ConnectionPoint rotatedPoint(*it, rotationBrickPosition);
      rotatedPoints.push_back(rotatedPoint);
    }
    std::sort(rotatedPoints.begin(), rotatedPoints.end());

    for(std::vector<ConnectionPoint>::const_iterator it1 = pointsForSccs.begin(), it2 = rotatedPoints.begin(); it1 != pointsForSccs.end(); ++it1, ++it2) {
      if(*it2 < *it1) {
        return false;
      }
      if(*it1 < *it2) {
        return false;
      }
    }
    return true;
  }

  bool isRotationallyMinimal(const ConnectionPoint &p) const {
    if(!isRotationallySymmetric)
      return true; // not applicaple.
    ConnectionPoint rotatedPoint(p, rotationBrickPosition);
    return p < rotatedPoint;
  }

  RectilinearBrick operator[](int i) const {
    assert(i < size);
    assert(i >= 0);
    if(i == 0) {
      return RectilinearBrick();
    }
    return otherBricks[i-1];
  }
  bool operator<(const FatSCC &c) const {
    if(index == NO_INDEX) { // Proper comparison:
      for(int i = 0; i < size-1; ++i) {
        if(otherBricks[i] != c.otherBricks[i])
          return otherBricks[i] < c.otherBricks[i];
      }
      return false;
    }

    if(size != c.size)
      return size > c.size; // Big first.
    return index < c.index;
  }
  bool operator==(const FatSCC &c) const {
    return size == c.size && index == c.index;
  }
  bool check(const FatSCC &c) const {
    if(size != c.size)
      return true;
    bool indicesSame = index == c.index;
    bool otherBricksSame = true;
    for(int i = 0; i < size-1; ++i) {
      if(otherBricks[i] != c.otherBricks[i]) {
        otherBricksSame = false;
        break;
      }
    }
    return indicesSame == otherBricksSame;
  }

  bool ensureOriginIsSmallest() {
    int minI = -1;
    RectilinearBrick min; // origin.
    for(int i = 0; i < size-1; ++i) {
      if(otherBricks[i] < min) {
        minI = i;
        min = otherBricks[i];
      }
    }

    if(minI == -1) 
      return false;

    // Move all according to the new origin:
    for(int i = 0; i < size-1; ++i) {
      otherBricks[i].x -= min.x;
      otherBricks[i].y -= min.y;
    }

    // Re-introduce old origin at the brick now representing the new origin:
    otherBricks[minI].x -= min.x;
    otherBricks[minI].y -= min.y;
    std::sort(otherBricks, &otherBricks[size-1]);
    return true;
  }

  /*
  Turns this scc 90 degrees
  Might cause reordering. 
  Causes new brick to be "origin".
  It is assumed that it is possible to turn 90 degrees.
  1,2 -> 2,-1 -> -1,-2
  */
  void turn90() {
    // First turn 90:
    for(int i = 0; i < size-1; ++i) {
      int8_t oldX = otherBricks[i].x;
      otherBricks[i].x = otherBricks[i].y;
      otherBricks[i].y = -oldX;
      otherBricks[i].flipHorizontal();
    }

    // Find the new origin:
    int minI = 0;
    for(int i = 1; i < size-1; ++i) {
      if(otherBricks[i] < otherBricks[minI])
        minI = i;
    }

    // Move all according to the new origin:
    int8_t moveX = otherBricks[minI].x;
    int8_t moveY = otherBricks[minI].y;
    for(int i = 0; i < size-1; ++i) {
      otherBricks[i].x -= moveX;
      otherBricks[i].y -= moveY;
    }

    // Re-introduce the turned old origin:
    otherBricks[minI].setHorizontalTrue(); 
    otherBricks[minI].x -= moveX;
    otherBricks[minI].y -= moveY;

    // Sort bricks:
    std::sort(otherBricks, &otherBricks[size-1]);
  }

  void turn180() {
    // First turn all bricks 180:
    for(int i = 0; i < size-1; ++i) {
      otherBricks[i].x = -otherBricks[i].x;
      otherBricks[i].y = -otherBricks[i].y;
    }
    if(size == 2)
      return;

    // Find the new origin if necessary. If no change: sort.:
    if(!ensureOriginIsSmallest())
      std::sort(otherBricks, &(otherBricks[size-1]));
  }

  bool canTurn90() const {
    if(size <= 2) {
      return false;
    }

    for(int i = 0; i < size-1; ++i) {
      if(otherBricks[i].level() > 0)
        return false;
      if(otherBricks[i].horizontal())
        return true;
    }
    return false;
  }

  FatSCC rotateToMin() const {
    FatSCC min(*this);
    FatSCC candidate(*this);
    if(canTurn90()) {
      for(int i = 0; i < 3; ++i) {
        candidate.turn90();
        if(candidate < min) {
          min = candidate;
        }
      }
    }
    else {
      candidate.turn180();
      if(candidate < min) {
        min = candidate;
      }
    }
    return min;
  }
  
  int getBrickIndex(const RectilinearBrick &b) const {
    if(b.isBase())
      return 0;
    for(int i = 0; i < size-1; ++i)
      if(b == otherBricks[i])
	return i+1;
    assert(false);
    return -1;
  }
};

inline std::ostream& operator<<(std::ostream& os, const FatSCC& c) {
  os << "FatSCC[size=" << c.size;
  if(c.index != NO_INDEX)
    os << ",index=" << c.index;
  if(c.isRotationallySymmetric) {
    os << ",symmetric@" << c.rotationBrickPosition.X << "," << c.rotationBrickPosition.Y;
  }
  for(int i = 0; i < c.size-1; ++i)
    os << c.otherBricks[i];
  os << "]";
  return os;
}

#endif // STRONGLY_CONNECTED_CONFIGURATION_HPP
