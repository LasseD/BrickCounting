#ifndef RECTILINEAR_CONFIGURATION_HPP
#define RECTILINEAR_CONFIGURATION_HPP

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

struct IBrick {
  RectilinearBrick rb;
  Brick b;
  int scci, rbi; // scci = SCC index, rbi = rectililnear brick index in configuration.
  IBrick(const RectilinearBrick &rb, const Brick &b, int scci, int rbi) : rb(rb), b(b), scci(scci), rbi(rbi) {}
  IBrick() {}
  IBrick(const IBrick &b) : rb(b.rb), b(b.b), scci(b.scci), rbi(b.rbi) {}
};

struct RectilinearConfigurationInfo {
  Point p;
  float angle;
  int level;
  RectilinearConfigurationInfo(Point p, float angle, int level) : p(p), angle(angle), level(level) {}
  RectilinearConfigurationInfo() {}
  RectilinearConfigurationInfo(const RectilinearConfigurationInfo &i) : p(i.p), angle(i.angle), level(i.level) {}
};

typedef std::pair<int,Connection> IConnection;

class RectilinearConfiguration : public LDRPrinter {
private:
  int scci, bricksI;
  IBrick bricks[6];
  std::map<int,RectilinearConfigurationInfo> infoMap;

public:
  template <unsigned int SIZE>
  RectilinearConfiguration(const StronglyConnectedConfiguration<SIZE> &scc) {
    scci = 1;
    bricksI = 0;
    RectilinearBrick b;
    for(int i = 0; i < SIZE; b=scc.otherBricks[i++]) {
      Brick brick(b, Point(0,0), 0, 0);
      IBrick ib(b, brick, 0, i);
      bricks[bricksI++] = ib;
    }
    infoMap.insert(std::make_pair(0,RectilinearConfigurationInfo(Point(0, 0), 0, 0)));
  }

  template <unsigned int SIZE>
  void add(const StronglyConnectedConfiguration<SIZE> &scc, IConnection c) {
    // Get objects of interest:
    RectilinearConfigurationInfo prevInfo = infoMap[c.first];
    ConnectionPoint &prevPoint = c.second.first;
    Brick prevBrick(prevPoint.brick, prevInfo.p, prevInfo.angle, prevInfo.level);
    Point prevStud = prevBrick.getStudPosition(prevPoint.type);
    std::cout << "Stud position for " << prevBrick << " is at " << prevStud.X << "," << prevStud.Y << std::endl;

    // Compute position of bricks:
    float angle = M_PI/8;// prevInfo.angle + M_PI/2*(prevPoint.type-c.second.second.type-2);
    int level = prevInfo.level + prevPoint.brick.level() + (prevPoint.above ? 1 : -1);

    RectilinearBrick b;
    for(int i = 0; i < SIZE; b=scc.otherBricks[i++]) {
      // Compute location:
      Point location = prevStud;

      Brick brick(b, prevStud, angle, level);

      IBrick ib(b, brick, scci, i);
      bricks[bricksI++] = ib;
    }
    infoMap.insert(std::make_pair(scci,RectilinearConfigurationInfo(prevStud, angle, level)));
    ++scci;
  }

  bool isRealizable(std::set<Connection> &foundConnections) {
    // TODO!
    std::cout << "YAY" << std::endl;
    return false;
  }

  void toLDR(std::ofstream &os, int x, int y, int ldrColor) const {
    int colors[6] = {LDR_COLOR_RED, LDR_COLOR_YELLOW, LDR_COLOR_BLACK, LDR_COLOR_BLUE, 2, 3};

    for(int i = 0; i < bricksI; ++i) {
      bricks[i].b.toLDR(os, x, y, colors[bricks[i].scci]);
    }
  }
};

#endif // RECTILINEAR_CONFIGURATION_HPP
