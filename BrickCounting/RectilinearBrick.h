#ifndef RECTILINEAR_BRICK_H
#define RECTILINEAR_BRICK_H

#include "LDRPrinter.h"

#include <iostream>
#include <fstream>
#include <stdint.h>

#define STRONGLY_CONNECTED_BRICK_POSITIONS 76

// Forward declaration:
class ConnectionPoint;

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

  void getConnectionPointsAbove(ConnectionPoint *pts, int &sizePts);
  void getConnectionPointsBelow(ConnectionPoint *pts, int &sizePts);

  void serialize(std::ofstream &os) const;
  void deserialize(std::ifstream &is);

  void toLDR(std::ofstream &os, int x, int y, int ldrColor) const;

  bool atConnectingLevelOf(const ConnectionPoint &p) const;
  bool blocks(const ConnectionPoint &p) const;
  bool angleLocks(const ConnectionPoint &p) const;

private:
  void getConnectionPoints(ConnectionPoint *pts, int &sizePts, bool above);
  bool inInterval(int8_t min, int8_t max, int8_t a) const;
  bool distLessThan2(int8_t a, int8_t b) const;
};

std::ostream& operator<<(std::ostream &os, const RectilinearBrick& b);


enum ConnectionPointType {NW = 0, NE = 1, SE = 2, SW = 3};

struct ConnectionPoint {
  ConnectionPointType type;
  RectilinearBrick brick;
  bool above, angleLocked;

  ConnectionPoint(ConnectionPointType type, const RectilinearBrick &brick, bool above) : type(type), brick(brick), above(above), angleLocked(false) {}
  ConnectionPoint() {}
  ConnectionPoint(const ConnectionPoint &p) : type(p.type), brick(p.brick), above(p.above), angleLocked(p.angleLocked) {}

  uint8_t x() const {
    if(brick.horizontal()) {
      if(type == NW || type == SW)
	return brick.x;
      return brick.x+3;
    }
    else {
      if(type == NW || type == SW)
	return brick.x;
      return brick.x+1;
    }
  }
  uint8_t y() const {
    if(brick.horizontal()) {
      if(type == SW || type == SE) 
	return brick.y;
      return brick.y+1;
    }
    else {
      if(type == SW || type == SE)
	return brick.y;
      return brick.y+3;
    }
  }

  bool operator < (const ConnectionPoint &p) const {
    if(brick != p.brick)
      return brick < p.brick;
    if(type != p.type)
      return type < p.type;
    return above < p.above;
  }
};

typedef std::pair<ConnectionPoint,ConnectionPoint> Connection;

#endif // RECTILINEAR_BRICK_H
