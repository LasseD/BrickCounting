#ifndef RECTILINEAR_BRICK_H
#define RECTILINEAR_BRICK_H

#include "LDRPrinter.h"

#include <iostream>
#include <fstream>
#include <stdint.h>

#define HALF_STUD_DISTANCE 0.5
#define STUD_DISTANCE 1.0
#define STUD_AND_A_HALF_DISTANCE 1.5

#define STRONGLY_CONNECTED_BRICK_POSITIONS 76

// Forward declaration:
struct ConnectionPoint;

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
  void flipHorizontal();

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

  // For specialized use: Pretend brick is at a 4x4 grid. Get x/y of connection point in this grid (+brick pos.).
  uint8_t x4x4() const {
    if(type == NW || type == SW)
      return brick.x+1;
    return brick.x+2;
  }
  uint8_t y4x4() const {
    if(type == SW || type == SE) 
      return brick.y;
    return brick.y+3;
  }

  // For specialized use: Position of connection point 
  double x() const {
    if(type == NW || type == SW)
      return brick.x-HALF_STUD_DISTANCE;
    return brick.x+HALF_STUD_DISTANCE;
  }
  double y() const {
    if(type == SW || type == SE) 
      return brick.y-STUD_AND_A_HALF_DISTANCE;
    return brick.y+STUD_AND_A_HALF_DISTANCE;
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
