#ifndef RECTILINEAR_BRICK_H
#define RECTILINEAR_BRICK_H

#include "LDRPrinter.h"

#include <iostream>
#include <fstream>
#include <stdint.h>

#define STRONGLY_CONNECTED_BRICK_POSITIONS 76

enum ConnectionPointType {NW = 0, NE = 1, SW = 2, SE = 3};

struct ConnectionPoint {
  ConnectionPointType type;
  bool isAbove;
  ConnectionPoint(ConnectionPointType type, bool isAbove) : type(type), isAbove(isAbove) {}
};

class RectilinearBrick : public LDRPrinter {
public:
  int8_t x, y;
  uint8_t levelShifted;
  //bool horizontal; : last bit of levelShifted
  RectilinearBrick(int8_t x, int8_t y, int level, bool horizontal) : x(x), y(y), levelShifted((level<<1)+horizontal) {}
  RectilinearBrick(const RectilinearBrick& b) : x(b.x), y(b.y), levelShifted(b.levelShifted) {}
  RectilinearBrick() : x(0), y(0), levelShifted(0) {}

  bool operator < (const RectilinearBrick &b) const;
  bool operator == (const RectilinearBrick &b) const;
  bool operator != (const RectilinearBrick &b) const;
  bool horizontal() const;
  int level() const;
  void setHorizontalTrue();
  void setHorizontalFalse();

  bool intersects(const RectilinearBrick &b) const;

  void constructAllStronglyConnected(RectilinearBrick *bricks, int &bricksSize) const;
  void constructAllStronglyConnected(RectilinearBrick *bricks, int &bricksSize, int level) const;

  //void getConnectionPointsAbove(ConnectionPoint *pts, int &sizePts);
  //void getConnectionPointsBelow(ConnectionPoint *pts, int &sizePts);

  void serialize(std::ofstream &os) const;
  void deserialize(std::ifstream &is);

  void toLDR(std::ofstream &os, int x, int y, int ldrColor) const;

private:
  /*void getConnectionPointsNoLevel(ConnectionPoint *pts, int &sizePts);
  void setConnectionPointsDataNoLevel(ConnectionPoint &pt, bool isAbove, ConnectionPointType type) {
  pt.isAbove = isAbove;
  pt.type = type;
  }*/
  bool inInterval(int8_t min, int8_t max, int8_t a) const {
    return  min <= a && a <= max;
  }
  bool distLessThan2(int8_t a, int8_t b) const {
    return a == b || a-1 == b || b-1 == a;
  }
};

std::ostream& operator<<(std::ostream &os, const RectilinearBrick& b);

#endif // RECTILINEAR_BRICK_H
