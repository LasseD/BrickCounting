#ifndef CONFIGURATION_HPP
#define CONFIGURATION_HPP

#include "LDRPrinter.h"
#include "Brick.h"
#include "RectilinearBrick.h"
#include "ConnectionPoint.h"
#include "StronglyConnectedConfiguration.hpp"

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

#define P1 first
#define P2 second
typedef std::pair<IConnectionPoint,IConnectionPoint> IConnectionPair;
inline std::ostream& operator<<(std::ostream &os, const IConnectionPair& c) {
  os << c.first << "<>" << c.second;
  return os;
}

struct Angle {
  short n; // nominator
  unsigned short d; // denominator

  Angle() {}
  Angle(short n, unsigned short d) : n(n), d(d) {
    assert(d != 0);
    assert(-d <= n);
    assert(n <= d);
  }
  Angle(const Angle &a) : n(a.n), d(a.d) {
    assert(d != 0);  
    assert(-d <= n);
    assert(n <= d);
  }
  Angle(ConnectionPoint p1, Brick b1, ConnectionPoint p2, Brick b2, unsigned short d) : d(d) {
    if(!p1.above) {
      std::swap(p1,p2); // ensure px.above=true is first.
      std::swap(b1,b2);
    }

    // Compute the angle difference while taking the conection points into account:
    double angleDiff = -b1.angle + M_PI/2*(p2.type-p1.type-2) + b2.angle;

    // Transform to n:
    while(angleDiff < -M_PI)
      angleDiff += 2*M_PI;
    while(angleDiff > M_PI)
      angleDiff -= 2*M_PI;
    assert(angleDiff >= -MAX_ANGLE_RADIANS);
    assert(angleDiff <= MAX_ANGLE_RADIANS);
    n = (short)round(angleDiff/MAX_ANGLE_RADIANS*d);
  }

  double toRadians() const {
    return n * MAX_ANGLE_RADIANS / d;
  }

  bool operator<(const Angle &c) const {
    if(d != c.d)
      return d < c.d;
    return n < c.n;
  }
  bool operator!=(const Angle &c) const {
    return n != c.n || d != c.d;
  }
  bool operator==(const Angle &c) const {
    return n == c.n && d == c.d;
  }
};
inline std::ostream& operator<<(std::ostream &os, const Angle& c) {
  os << c.n << "/" << c.d;
  return os;
}

struct Connection {
  Angle angle;
  IConnectionPoint p1, p2; // Invariant: p1.first.configurationSCCI < p2.second.configurationSCCI
  Connection(){}
  Connection(const IConnectionPair &c, const Angle &angle) : angle(angle), p1(c.first), p2(c.second) {
    if(p1.first.configurationSCCI > p2.first.configurationSCCI) {
      std::swap(p1, p2); // ensures connections are always
    }
  }
  Connection(const Connection &c) : angle(c.angle), p1(c.p1), p2(c.p2) {}
  Connection(const IConnectionPoint &p1, const IConnectionPoint &p2, const Angle &angle) : angle(angle), p1(p1), p2(p2) {
    if(p1.first.configurationSCCI > p2.first.configurationSCCI) {
      std::swap(this->p1, this->p2); // ensures connections are always with p1->above = true.
    }  
  } 
  double angleToRadians() const {
    return p1.second.above ? angle.toRadians() : -angle.toRadians();
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

struct IConnectionPairList {
private:
  std::set<IConnectionPair> v;
public:
  IConnectionPairList() {}
  IConnectionPairList(const IConnectionPairList &l) : v(l.v) {}
  IConnectionPairList(const std::vector<IConnectionPair> &l) {
    for(std::vector<IConnectionPair>::const_iterator it = l.begin(); it != l.end(); ++it)
      insert(*it);
  }

  bool operator<(const IConnectionPairList &l) const {
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
inline std::ostream& operator<<(std::ostream &os, const IConnectionPairList& l) {
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
  IBrick(const IBrick &b) : rb(b.rb), b(b.b), bi(b.bi) {}
};

struct Configuration : public LDRPrinter {
private:
  Brick origBricks[6];

public:
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

  template <int ADD_X, int ADD_Y>
  bool isRealizable(std::vector<IConnectionPair> &found) const {
    for(int i = 0; i < bricksSize; ++i) {
      const IBrick &ib = bricks[i];
      for(int j = i+1; j < bricksSize; ++j) {
        const IBrick &jb = bricks[j];
#ifdef _TRACE
        std::cout << "   " << ib.bi << "/" << ib.b << " VS " << jb.bi << "/" << jb.b << std::endl;
#endif
        if(ib.bi.configurationSCCI == jb.bi.configurationSCCI) {
#ifdef _TRACE
          std::cout << "   same configurationSCCI => OK" << std::endl;
#endif
          continue; // from same scc.
        }
        bool connected;
        ConnectionPoint pi, pj;
        if(ib.b.intersects<ADD_X,ADD_Y>(jb.b, jb.rb, connected, pj, pi, ib.rb)) {
          if(!connected) {
#ifdef _TRACE
            std::cout << "   Intersect, no connect => fail!" << std::endl;
#endif
            return false;
          }
#ifdef _TRACE
          std::cout << "   Connect!" << std::endl;
#endif
          pi.brickI = ib.bi.sccBrickI;
          pj.brickI = jb.bi.sccBrickI;
          IConnectionPair c(IConnectionPoint(ib.bi, pi), IConnectionPoint(jb.bi, pj));
          found.push_back(c);
        }
      }
    }
#ifdef _TRACE
    std::cout << "   OK!" << std::endl;
#endif
    return true;
  }

  void initSCC(const FatSCC &scc) {
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
    Point prevStud = prevBrick.getStudPosition(prevPoint.type);
    //std::cout << "ADD: Prev: " << prevBrick << " w. stud " << prevStud.X << "," << prevStud.Y << std::endl;

    // Compute position of bricks:
    double angle = prevBrick.angle + M_PI/2*(currPoint.type-prevPoint.type-2) + c.angleToRadians();
    int8_t level = prevOrigBrick.level + prevPoint.brick.level() + (prevPoint.above ? 1 : -1);

#ifdef _TRACE
    std::cout << "Configuration::add(" << scc << "," << c << ")" << std::endl;
#endif
    RectilinearBrick rb;
    origBricks[currBrickI] = Brick(rb, currPoint, prevStud, angle, level);
    for(int i = 0; i < scc.size; rb=scc.otherBricks[i++]) {
      Brick brick(rb, currPoint, prevStud, angle, level);
#ifdef _TRACE
      std::cout << " ADD brick " << brick << std::endl;
#endif

      IBrick ib(rb, brick, BrickIdentifier(scc.index, i, currBrickI));
      bricks[bricksSize++] = ib;
    }
  }

  Configuration(FatSCC const * const sccs, const std::vector<Connection> &cs) : bricksSize(0) {
    initSCC(sccs[0]);
#ifdef _TRACE
    std::cout << "Constructing conf. base: " << sccs[0] << std::endl;
#endif
    std::set<Connection> remainingConnections;
    for(std::vector<Connection>::const_iterator it = cs.begin(); it != cs.end(); ++it) {
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

  void toLDR(std::ofstream &os, int x, int y, int) const {    
    int colors[6] = {LDR_COLOR_RED, LDR_COLOR_YELLOW, LDR_COLOR_BLUE, 3, 85, LDR_COLOR_BLACK};

    for(int i = 0; i < bricksSize; ++i) {
      bricks[i].b.toLDR(os, x, y, colors[bricks[i].bi.configurationSCCI]);
    }
  }

  FatSCC toMinSCC() const {
    std::vector<Brick> v;
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
