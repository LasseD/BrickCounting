#ifndef CONFIGURATION_HPP
#define CONFIGURATION_HPP

#include "LDRPrinter.h"
#include "Brick.h"
#include "RectilinearBrick.h"
#include "ConnectionPoint.h"
#include "Block.hpp"

#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <set>
#include <map>
#include <queue>
#include <assert.h> 
#include <iomanip>

#define MAX_ANGLE_RADIANS 0.664054277

typedef std::pair<IConnectionPoint,IConnectionPoint> IConnectionPair;
inline std::ostream& operator<<(std::ostream &os, const IConnectionPair& c) {
  os << c.first << "<>" << c.second;
  return os;
}

struct StepAngle {
  short n; // nominator
  unsigned short d; // denominator

  StepAngle() : n(0), d(1) {}
  StepAngle(double n) : n((short)geometry::round(n*10000/MAX_ANGLE_RADIANS)), d(10000) {}
  StepAngle(short n, unsigned short d) : n(n), d(d) {
    assert(d != 0);
    assert(-d <= n);
    assert(n <= d);
  }
  StepAngle(const StepAngle &a) : n(a.n), d(a.d) {
    assert(d != 0);
    assert(-d <= n);
    assert(n <= d);
  }
  StepAngle(ConnectionPoint p1, Brick b1, ConnectionPoint p2, Brick b2, unsigned short d) : d(d) {
    if(!p1.above) {
      std::swap(p1,p2); // ensure px.above=true is first.
      std::swap(b1,b2);
    }

    // Compute the angle difference while taking the conection points into account:
    double angleDiff = geometry::normalizeAngle(-b1.angle + M_PI/2*(p2.type-p1.type-2) + b2.angle);

    // Transform to n:
    assert(angleDiff >= -MAX_ANGLE_RADIANS);
    assert(angleDiff <= MAX_ANGLE_RADIANS);
    n = (short)geometry::round((angleDiff/MAX_ANGLE_RADIANS)*d);
  }

  double toRadians() const {
    return n * MAX_ANGLE_RADIANS / d;
  }
  double negate() {
    return n = -n;
  }

  bool operator<(const StepAngle &c) const {
    if(d != c.d)
      return d < c.d;
    return n < c.n;
  }
  bool operator!=(const StepAngle &c) const {
    return n != c.n || d != c.d;
  }
  bool operator==(const StepAngle &c) const {
    return n == c.n && d == c.d;
  }
};
inline std::ostream& operator<<(std::ostream &os, const StepAngle& c) {
  os << c.n << "/" << c.d;
  return os;
}

struct Connection {
  StepAngle angle;
  IConnectionPoint p1, p2; // Invariant: p1.first.configurationSCCI < p2.second.configurationSCCI
  Connection(){}
  Connection(const IConnectionPair &c, const StepAngle &angle) : angle(angle), p1(c.first), p2(c.second) { }
  Connection(const Connection &c) : angle(c.angle), p1(c.p1), p2(c.p2) {}
  Connection(const IConnectionPoint &p1, const IConnectionPoint &p2, const StepAngle &angle) : angle(angle), p1(p1), p2(p2) { } 
  double angleToRadians() const {
    return angle.toRadians();
  }
  bool operator<(const Connection &c) const {
    if(angle != c.angle)      
      return angle < c.angle;
    if(p1 != c.p1)
      return p1 < c.p1;
    return p2 < c.p2;
  }
  bool operator!=(const Connection &c) const {
    return angle != c.angle || p1 != c.p1 || p2 != c.p2;
  }
  bool operator==(const Connection &c) const {
    return angle == c.angle && p1 == c.p1 && p2 == c.p2;
  }
};
inline std::ostream& operator<<(std::ostream &os, const Connection& c) {
  os << "Connection[" << c.p1 << "-" << c.p2 << ",a=" << c.angleToRadians() << "]";
  return os;
}

struct IConnectionPairSet {
private:
  std::set<IConnectionPair> v;
public:
  IConnectionPairSet() {}
  IConnectionPairSet(const IConnectionPairSet &l) : v(l.v) {}

  template <unsigned int SIZE>
  IConnectionPairSet(const util::TinyVector<IConnectionPair, SIZE> &l) {
    for(const IConnectionPair* it = l.begin(); it != l.end(); ++it)
      insert(*it);
  }

  bool operator<(const IConnectionPairSet &l) const {
    if(v.size() != l.v.size())      
      return v.size() < l.v.size();
    for(std::set<IConnectionPair>::const_iterator it1 = v.begin(), it2 = l.v.begin(); it1 != v.end(); ++it1, ++it2) {
      const IConnectionPair &c1 = *it1;
      const IConnectionPair &c2 = *it2;
      if(c1 != c2)
        return c1 < c2;
    }
    return false;
  }
  void insert(IConnectionPair toInsert) {
    if(toInsert.first.first.configurationSCCI > toInsert.second.first.configurationSCCI) {
      std::swap(toInsert.first, toInsert.second);
    }
    v.insert(toInsert);
  }
  void insert(const Connection &c) {
    IConnectionPair toInsert(c.p1, c.p2);
    insert(toInsert);
  }
  std::set<IConnectionPair>::const_iterator begin() const {
    return v.begin();
  }
  std::set<IConnectionPair>::const_iterator end() const {
    return v.end();
  }
  size_t size() const {
    return v.size();
  }
};
inline std::ostream& operator<<(std::ostream &os, const IConnectionPairSet& l) {
  for(std::set<IConnectionPair>::const_iterator it = l.begin(); it != l.end(); ++it) {
    os << it->first << "/" << it->second << " ";
  }
  return os;
}

struct IBrick {
  RectilinearBrick rb;
  Brick b;
  BrickIdentifier bi;
  IBrick(const RectilinearBrick &rb, const Brick &b, const BrickIdentifier &bi) : rb(rb), b(b), bi(bi) {}
  IBrick() {}
  IBrick(const IBrick &ib) : rb(ib.rb), b(ib.b), bi(ib.bi) {}
};

struct Configuration : public LDRPrintable {
  //private:
  Brick origBricks[6];

  //public:
  int bricksSize;
  IBrick bricks[6];

  Configuration() : bricksSize(0) {}
  Configuration(const Configuration &c) : bricksSize(c.bricksSize) {
    for(int i = 0; i < bricksSize; ++i) {
      const IBrick &ib = c.bricks[i];
      bricks[i] = ib;
      origBricks[ib.bi.configurationSCCI] = c.origBricks[ib.bi.configurationSCCI];
    }
  }

  template <int ADD_XY>
  bool isRealizable(util::TinyVector<IConnectionPair, 8> &found) const {
    for(int i = 0; i < bricksSize; ++i) {
      const IBrick &ib = bricks[i];
      for(int j = i+1; j < bricksSize; ++j) {
        const IBrick &jb = bricks[j];
        if(ib.bi.configurationSCCI == jb.bi.configurationSCCI) {
          continue; // from same scc.
        }
        bool connected;
        ConnectionPoint pi, pj;
        if(ib.b.intersects<ADD_XY>(jb.b, jb.rb, connected, pj, pi, ib.rb)) {
          if(!connected) {
            return false;
          }
          pi.brickI = ib.bi.brickIndexInScc;
          pj.brickI = jb.bi.brickIndexInScc;
          IConnectionPair c(IConnectionPoint(ib.bi, pi), IConnectionPoint(jb.bi, pj));
          found.push_back(c);
        }
      }
    }
    return true;
  }

  template <int ADD_XY>
  bool isRealizable(const util::TinyVector<int, 5> &possibleCollisions, int end) const {
    for(unsigned int i = 0; i < possibleCollisions.size(); ++i) {
      const IBrick &ib = bricks[possibleCollisions[i]];
      for(int j = 0; j < end; ++j) {
        const IBrick &jb = bricks[j+bricksSize-end];
        assert(ib.bi.configurationSCCI != jb.bi.configurationSCCI);
        bool connected;
        ConnectionPoint pi, pj;
        if(ib.b.intersects<ADD_XY>(jb.b, jb.rb, connected, pj, pi, ib.rb) && !connected) {
          return false;
        }
      }
    }
    return true;
  }

  void getPossibleCollisions(const FatSCC &scc, const IConnectionPair &connectionPair, util::TinyVector<int, 5> &result) const {
    int prevBrickI = connectionPair.P1.first.configurationSCCI;
    const Brick &prevOrigBrick = origBricks[prevBrickI];
    const ConnectionPoint &prevPoint = connectionPair.P1.second;
    const ConnectionPoint &currPoint = connectionPair.P2.second;
    const int8_t level = prevOrigBrick.level + prevPoint.brick.level() + (prevPoint.above ? 1 : -1) - currPoint.brick.level();

    for(int i = 0; i < bricksSize; ++i) {
      const IBrick ib = bricks[i];
      if(scc.size == 1 && connectionPair.P1.first.configurationSCCI == ib.bi.configurationSCCI && connectionPair.P1.first.brickIndexInScc == ib.bi.brickIndexInScc)
        continue; // Exclude brick we are connecting to.
      const Brick b = ib.b;
      const int8_t levelI = b.level;
      RectilinearBrick sccBrick;
      for(int j = 0; j < scc.size; sccBrick = scc.otherBricks[j], ++j) {
        const int8_t levelJ = level + sccBrick.level();
        if(levelI == levelJ || levelI+1 == levelJ || levelJ+1 == levelI) {
          result.push_back(i);
          //std::cout << "Possible collision: " << i << ": " << b << std::endl;
          break;
        }
      }
    }
  }

  void initSCC(const FatSCC &scc) {
    assert(bricksSize == 0);
#ifdef _TRACE
    std::cout << "Configuration::init(" << scc << ")" << std::endl;
#endif
    RectilinearBrick b;
    origBricks[0] = Brick(b);
    for(int i = 0; i < scc.size; b=scc.otherBricks[i++]) {
      Brick brick(b);
      IBrick ib(b, brick, BrickIdentifier(scc.index, i, 0));
      bricks[bricksSize++] = ib;
    }
  }

  Configuration(const FatSCC &scc) : bricksSize(0) {
    initSCC(scc);
  }

  void add(const FatSCC &scc, int configurationSCCI, Connection c) {
    // Get objects of interest:
    if(configurationSCCI == c.p1.first.configurationSCCI) {
      std::swap(c.p1,c.p2);
    }
    int prevBrickI = c.p1.first.configurationSCCI;
    int currBrickI = c.p2.first.configurationSCCI;
    Brick &prevOrigBrick = origBricks[prevBrickI];
    const ConnectionPoint &prevPoint = c.p1.second;
    const ConnectionPoint &currPoint = c.p2.second;
    Brick prevBrick(prevOrigBrick, c.p1.second.brick);
    geometry::Point prevStud = prevBrick.getStudPosition(prevPoint.type);

    // Compute position of bricks:
    double angle = prevBrick.angle + M_PI/2*(currPoint.type-prevPoint.type-2) + c.angleToRadians();
    int8_t level = prevOrigBrick.level + prevPoint.brick.level() + (prevPoint.above ? 1 : -1);

    RectilinearBrick rb;
    origBricks[currBrickI] = Brick(rb, currPoint, prevStud, angle, level);
    for(int i = 0; i < scc.size; rb=scc.otherBricks[i++]) {
      Brick brick(rb, currPoint, prevStud, angle, level);

      IBrick ib(rb, brick, BrickIdentifier(scc.index, i, currBrickI));
      bricks[bricksSize++] = ib;
    }
  }

  Configuration(FatSCC const * const sccs, const util::TinyVector<Connection, 5> &cs) : bricksSize(0) {
    initSCC(sccs[0]);
    std::set<Connection> remainingConnections;
    for(const Connection* it = cs.begin(); it != cs.end(); ++it) {
      remainingConnections.insert(*it);
    }

    bool addedSccs[6] = {true,false,false,false,false,false};

    while(!remainingConnections.empty()) {
      for(std::set<Connection>::const_iterator it = remainingConnections.begin(); it != remainingConnections.end(); ++it) {
        const Connection &c = *it;
        int i1 = c.p1.first.configurationSCCI;
        int i2 = c.p2.first.configurationSCCI;
        if(addedSccs[i1] && addedSccs[i2]) {
          remainingConnections.erase(it);
          break;
        }
        if(!addedSccs[i1] && !addedSccs[i2]) {
          continue;
        }

        if(addedSccs[i1]) {
          add(sccs[i2], i2, c);
          addedSccs[i2] = true;
#ifdef _TRACE
          std::cout << " Adding " << i2 << ":" << sccs[i2] << " using " << c << std::endl;
#endif
        }
        else {
          add(sccs[i1], i1, c);
          addedSccs[i1] = true;
#ifdef _TRACE
          std::cout << " Adding " << i1 << ":" << sccs[i1] << " using " << c << std::endl;
#endif
        }
        remainingConnections.erase(it);
        break;
      }
    }
  }

  void toLDR(std::ofstream &os, int) const {    
    int colors[6] = {LDR_COLOR_RED, LDR_COLOR_YELLOW, LDR_COLOR_BLUE, LDR_COLOR_GREEN, LDR_COLOR_ORANGE, LDR_COLOR_PURPLE};

    for(int i = 0; i < bricksSize; ++i) {
      const Brick &b = bricks[i].b;
      b.toLDR(os, colors[bricks[i].bi.configurationSCCI]);
    }
  }

  FatSCC toMinSCC() const {
    util::TinyVector<Brick, 6> v;
    for(int i = 0; i < bricksSize; ++i)
      v.push_back(bricks[i].b);
    FatSCC ret(v);
    ret = ret.rotateToMin();
    return ret;
  }
};
inline std::ostream& operator<<(std::ostream& os, const Configuration& c) {
  os << "CONF[size=" << c.bricksSize << ",";
  for(int i = 0; i < c.bricksSize; ++i)
    os << c.bricks[i].b;
  os << "]";
  return os;
}

#endif // CONFIGURATION_HPP
