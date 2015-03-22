#ifndef RECTILINEAR_CONFIGURATION_HPP
#define RECTILINEAR_CONFIGURATION_HPP

#include "LDRPrinter.h"
#include "RectilinearBrick.h"
#include "StronglyConnectedConfiguration.hpp"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <set>
#include <assert.h> 

struct IBrick {
  RectilinearBrick rb;
  Brick b;
  int scci, rbi; // scci = SCC index, rbi = rectililnear brick index in configuration.
  IBrick(const RectilinearBrick &rb, const Brick &b, int scci, int rbi) : rb(rb), b(b), scci(scci), rbi(rbi) {}
};

struct IConnection {
  int rci;
  Connection c;
};

class RectilinearConfiguration : public LDRPrinter {
private:
  int scci;
  std::vector<IBrick> bricks;
  std::vector<Connection> connections;

public:
  template <unsigned int SIZE>
  RectilinearConfiguration(const StronglyConnectedConfiguration<SIZE> &scc) {
    scci = 1;
    RectilinearBrick b;
    for(int i = 0; i < SIZE; b=scc.otherBricks[i++]) {
      Brick brick(b, Point(0,0), 0, 0);
      IBrick ib(b, brick, 0, i);
      bricks.push_back(ib);
    }
  }

  template <unsigned int SIZE>
  void add(const StronglyConnectedConfiguration<SIZE> &scc, IConnection c) {
    RectilinearBrick b;
    for(int i = 0; i < SIZE; b=scc.otherBricks[i++]) {
      // TODO: Ensure brick is transformed to correct position:
      Brick brick(b, Point(0,0), 0, 0);

      IBrick ib(b, brick, scci, i);
      bricks.push_back(ib);
    }
    ++scci;
    // TODO: Connection
  }

  bool isRealizable(std::set<Connection> &foundConnections) {
    // TODO!
    std::cout << "YAY" << std::endl;
    return false;
  }

  void toLDR(std::ofstream &os, int x, int y, int ldrColor) const {
    int colors[6] = {LDR_COLOR_RED, LDR_COLOR_YELLOW, LDR_COLOR_BLACK, LDR_COLOR_BLUE, 2, 3};

    std::vector<IBrick>::const_iterator it = bricks.begin();
    for(; it != bricks.end(); ++it) {
      it->b.toLDR(os, x, y, colors[it->scci]);
    }
  }
};

#endif // RECTILINEAR_CONFIGURATION_HPP
