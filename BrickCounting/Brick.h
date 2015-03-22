#ifndef BRICK_H
#define BRICK_H

#include "RectilinearBrick.h"
#include "LDRPrinter.h"

#include <stdint.h>
#include <iostream>

#define NUMBER_OF_POIS_FOR_BOX_INTERSECTION 6
#define NUMBER_OF_STUDS 8
// 4mm in units:
#define HALF_STUD_DISTANCE 0.5
#define STUD_DISTANCE 1.0
#define HORIZONTAL_BRICK_HALF_WIDTH 1.0
#define HORIZONTAL_BRICK_CENTER_TO_SIDE 1.9875
#define HORIZONTAL_BRICK_CENTER_TO_TOP 0.9875
#define STUD_RADIUS 0.3
// 0.0625 is 0.5 mm.
#define SNAP_DISTANCE 0.0625

typedef std::pair<float,float> Point;
#define X first
#define Y second

class Brick : public LDRPrinter {
public:
  Point center;
  float angle; // angle 0 = horizontal.
  int8_t level;

  Brick(const Brick& b) : center(b.center), angle(b.angle), level(b.level) {}
  Brick(float cx, float cy, float a, int8_t lv) : center(cx, cy), angle(a), level(lv) {}
  Brick(const RectilinearBrick& b, Point origin, float originAngle, int8_t originLv);

  void toLDR(std::ofstream &os, int x, int y, int ldrColor) const;

  void moveBrickSoThisIsAxisAlignedAtOrigin(Brick &b) const;
  void getBoxPOIs(Point *pois) const;
  Point getStudPosition(ConnectionPointType type) const;
  void getStudPositions(Point *positions) const;
  bool boxIntersectsPOIsFrom(Brick &b) const;
  bool boxesIntersect(const Brick &b) const;
  bool boxIntersectsStudsFrom(Brick &b, const RectilinearBrick &bSource, bool &connected, Connection &foundConnection, const RectilinearBrick &source) const;
  /*
    Return true if this brick intersects the other. 
    If the two bricks are corner connected, then connected is set to true and the found connection is set.
   */
  bool intersects(const Brick &b, const RectilinearBrick &bSource, bool &connected, Connection &foundConnection, const RectilinearBrick &source) const;

  bool operator < (const Brick &b) const;
};

#endif // BRICK_H
