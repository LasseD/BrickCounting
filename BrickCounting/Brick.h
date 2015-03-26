
#ifndef BRICK_H
#define BRICK_H

#include "RectilinearBrick.h"
#include "LDRPrinter.h"

#define _USE_MATH_DEFINES
#include <math.h>
//#include <tgmath.h>
#include <stdint.h>
#include <iostream>

#define NUMBER_OF_POIS_FOR_BOX_INTERSECTION 6
#define NUMBER_OF_STUDS 8
// 4mm in units:
#define VERTICAL_BRICK_HALF_WIDTH 1.0
#define VERTICAL_BRICK_CENTER_TO_SIDE 0.9875
#define VERTICAL_BRICK_CENTER_TO_TOP 1.9875
#define STUD_RADIUS 0.3
// 0.0625 is 0.5 mm.
#define SNAP_DISTANCE 0.0625

typedef std::pair<double,double> Point;
#define X first
#define Y second

/*
A Brick at any location and angle.
Default same as a Rectilinear brick: vertical at 0,0,0 (angle 0)
 */
class Brick : public LDRPrinter {
public:
  Point center;
  double angle; // angle 0 = horizontal.
  int8_t level;

  Brick(const Brick& b) : center(b.center), angle(b.angle), level(b.level) {}
  Brick(const RectilinearBrick& b) : center(b.x, b.y), angle(b.horizontal() ? -M_PI/2 : 0), level(b.level()) {}
  Brick(const Brick& b, const RectilinearBrick& rb);
  Brick() {}
  Brick(double cx, double cy, double a, int8_t lv) : center(cx, cy), angle(a), level(lv) {}
  Brick(const RectilinearBrick& b, const ConnectionPoint& p, const Point &origin, double originAngle, int8_t originLv);

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

std::ostream& operator<<(std::ostream &os, const Brick& b);

#endif // BRICK_H
