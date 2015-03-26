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

typedef std::pair<int,Connection> IConnection;

class RectilinearConfiguration : public LDRPrinter {
private:
  int scci, bricksI;
  IBrick bricks[6];
  std::map<int,Brick> origBricks;

public:
  template <unsigned int SIZE>
  RectilinearConfiguration(const StronglyConnectedConfiguration<SIZE> &scc) {
    scci = 1;
    bricksI = 0;
    RectilinearBrick b;
    origBricks.insert(std::make_pair(0,Brick(b)));
    for(int i = 0; i < SIZE; b=scc.otherBricks[i++]) {
      Brick brick(b);
      IBrick ib(b, brick, 0, i);
      bricks[bricksI++] = ib;
    }
  }

  template <unsigned int SIZE>
  void add(const StronglyConnectedConfiguration<SIZE> &scc, IConnection c) {
    // Get objects of interest:
    Brick prevOrigBrick = origBricks[c.first];
    ConnectionPoint &prevPoint = c.second.first;
    ConnectionPoint &currPoint = c.second.second;
    Brick prevBrick(prevOrigBrick, c.second.first.brick);
    Point prevStud = prevBrick.getStudPosition(prevPoint.type);
    std::cout << "ADD: Prev: " << prevBrick << " w. stud " << prevStud.X << "," << prevStud.Y << std::endl;

    // Compute position of bricks:
    float angle = prevBrick.angle + M_PI/2*(currPoint.type-prevPoint.type-2);
    int level = prevOrigBrick.level + prevPoint.brick.level() + (prevPoint.above ? 1 : -1);

    RectilinearBrick b;
    origBricks.insert(std::make_pair(scci, Brick(b, currPoint, prevStud, angle, level)));
    for(int i = 0; i < SIZE; b=scc.otherBricks[i++]) {
      Brick brick(b, currPoint, prevStud, angle, level);
      std::cout << "ADD: Curr: " << brick << std::endl;

      IBrick ib(b, brick, scci, i);
      bricks[bricksI++] = ib;
    }
    ++scci;
  }

  bool isRealizable(std::set<Connection> &foundConnections) {
    // TODO!
    std::cout << "YAY" << std::endl;
    return false;
  }

  void toLDR(std::ofstream &os, int x, int y, int ldrColor) const {
    int colors[6] = {LDR_COLOR_RED, LDR_COLOR_YELLOW, LDR_COLOR_BLACK, LDR_COLOR_BLUE, 2, 85};

    for(int i = 0; i < bricksI; ++i) {
      //std::cout << "Printing brick for RC: " << bricks[i].b << std::endl;
      bricks[i].b.toLDR(os, x, y, colors[bricks[i].scci]);
    }
  }
};

#endif // RECTILINEAR_CONFIGURATION_HPP
