#ifndef CONFIGURATION_HPP
#define CONFIGURATION_HPP

#include "LDRPrinter.h"
#include "RectilinearBrick.h"
#include "StronglyConnectedConfiguration.hpp"

#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <set>
#include <map>
#include <assert.h> 

struct BrickIdentifier {
  unsigned long sccI; // In SCC list imported from file.
  int sccBrickI; // In SCC.
  int configurationSCCI;
  BrickIdentifier() {}
  BrickIdentifier(const BrickIdentifier &bi) : sccI(bi.sccI), sccBrickI(bi.sccBrickI), configurationSCCI(bi.configurationSCCI) {}
  BrickIdentifier(unsigned long sccI, int sccBrickI, int configurationSCCI) : sccI(sccI), sccBrickI(sccBrickI), configurationSCCI(configurationSCCI) {}

  bool operator<(const BrickIdentifier &bi) const {
    if(sccI != bi.sccI)      
      return sccI < bi.sccI;
    if(sccBrickI != bi.sccBrickI)      
      return sccBrickI < bi.sccBrickI;
    return configurationSCCI < bi.configurationSCCI;
  }
  bool operator!=(const BrickIdentifier &bi) const {
    return sccI != bi.sccI || sccBrickI != bi.sccBrickI || configurationSCCI != bi.configurationSCCI;
  }
  bool operator==(const BrickIdentifier &bi) const {
    return sccI == bi.sccI && sccBrickI == bi.sccBrickI && configurationSCCI == bi.configurationSCCI;
  }
};

inline std::ostream& operator<<(std::ostream &os, const BrickIdentifier& bi) {
  os << "BI[scc=" << bi.sccI << "," << bi.sccBrickI << ",confI=" << bi.configurationSCCI << "]";
  return os;
}

typedef std::pair<BrickIdentifier,ConnectionPoint> IConnectionPoint;

inline std::ostream& operator<<(std::ostream &os, const IConnectionPoint& p) {
  os << "ICP[" << p.first << "," << p.second << "]";
  return os;
}

struct Connection {
  IConnectionPoint p1, p2;
  double angle;
  Connection(){}
  Connection(const Connection &c) : p1(c.p1), p2(c.p2), angle(c.angle) {}
  Connection(const IConnectionPoint &p1, const IConnectionPoint &p2, double angle) : p1(p1), p2(p2), angle(angle) {} 
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
  os << "Connection[" << c.p1 << "-" << c.p2 << ",a=" << c.angle << "]";
  return os;
}

struct ConnectionList {
private:
  std::set<Connection> v;
public:
  ConnectionList() {}
  ConnectionList(const ConnectionList &l) : v(l.v) {}

  bool operator<(const ConnectionList &l) const {
    if(v.size() != l.v.size())      
      return v.size() < l.v.size();
    for(std::set<Connection>::const_iterator it1 = v.begin(), it2 = l.v.begin(); it1 != v.end(); ++it1, ++it2) {
      const Connection &c1 = *it1;
      const Connection &c2 = *it2;
      if(c1 != c2)
	return c1 < c2;
    }
    return false;
  }
  void insert(const Connection &c) {
    Connection toInsert(c);
    if(c.p1.second.above) {
      std::swap(toInsert.p1, toInsert.p2);
    }
    v.insert(toInsert);
  }
  std::set<Connection>::const_iterator begin() const {
    return v.begin();
  }
  std::set<Connection>::const_iterator end() const {
    return v.end();
  }
};

inline std::ostream& operator<<(std::ostream &os, const ConnectionList& l) {
  for(std::set<Connection>::const_iterator it = l.begin(); it != l.end(); ++it) {
    os << *it << " ";
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
  int scci, bricksSize;
  IBrick bricks[6];
  std::map<int,Brick> origBricks;

public:
  bool isRealizable(ConnectionList &found) const {
    for(int i = 0; i < bricksSize; ++i) {
      const IBrick &ib = bricks[i];
      for(int j = i+1; j < bricksSize; ++j) {
	const IBrick &jb = bricks[j];
	//std::cout << "   isRealizable step " << ib.bi << "/" << ib.b << "<>" << jb.bi << "/" << jb.b << std::endl;
	if(ib.bi.configurationSCCI == jb.bi.configurationSCCI)
	  continue; // from same scc.
	bool connected;
	ConnectionPoint pi, pj;
	if(ib.b.intersects(jb.b, jb.rb, connected, pj, pi, ib.rb)) {
	  if(!connected)
	    return false;
	  pi.brickI = ib.bi.sccBrickI;
	  pj.brickI = jb.bi.sccBrickI;
	  Connection c(IConnectionPoint(ib.bi, pi), IConnectionPoint(jb.bi, pj), 0.0);
	  found.insert(c);
	}
      }
    }
    return true;
  }

  Configuration(const FatSCC &scc) {
    scci = 1;
    bricksSize = 0;
    RectilinearBrick b;
    origBricks.insert(std::make_pair(0,Brick(b)));
    for(int i = 0; i < scc.size; b=scc.otherBricks[i++]) {
      Brick brick(b);
      IBrick ib(b, brick, BrickIdentifier(scc.index, i, 0));
      bricks[bricksSize++] = ib;
    }
  }

  void add(const FatSCC &scc, const Connection &c) {
    // Get objects of interest:
    Brick prevOrigBrick = origBricks[c.p1.first.configurationSCCI];
    const ConnectionPoint &prevPoint = c.p1.second;
    const ConnectionPoint &currPoint = c.p2.second;
    Brick prevBrick(prevOrigBrick, c.p1.second.brick);
    Point prevStud = prevBrick.getStudPosition(prevPoint.type);
    //std::cout << "ADD: Prev: " << prevBrick << " w. stud " << prevStud.X << "," << prevStud.Y << std::endl;

    // Compute position of bricks:
    double angle = prevBrick.angle + M_PI/2*(currPoint.type-prevPoint.type-2) + c.angle;
    int level = prevOrigBrick.level + prevPoint.brick.level() + (prevPoint.above ? 1 : -1);

    RectilinearBrick b;
    origBricks.insert(std::make_pair(scci, Brick(b, currPoint, prevStud, angle, level)));
    for(int i = 0; i < scc.size; b=scc.otherBricks[i++]) {
      Brick brick(b, currPoint, prevStud, angle, level);
      //std::cout << "ADD: Curr: " << brick << std::endl;

      IBrick ib(b, brick, BrickIdentifier(scc.index, i, scci));
      bricks[bricksSize++] = ib;
    }
    ++scci;
  }

  void toLDR(std::ofstream &os, int x, int y, int ldrColor) const {
    int colors[6] = {LDR_COLOR_RED, LDR_COLOR_YELLOW, LDR_COLOR_BLACK, LDR_COLOR_BLUE, 2, 85};

    for(int i = 0; i < bricksSize; ++i) {
      //std::cout << "Printing brick for RC: " << bricks[i].b << std::endl;
      bricks[i].b.toLDR(os, x, y, colors[bricks[i].bi.configurationSCCI]);
    }
  }
};

#endif // CONFIGURATION_HPP
