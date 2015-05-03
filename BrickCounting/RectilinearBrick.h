#ifndef RECTILINEAR_BRICK_H
#define RECTILINEAR_BRICK_H

#include "LDRPrinter.h"

#include <iostream>
#include <fstream>
#include <stdint.h>

#define HALF_STUD_DISTANCE 0.5
#define STUD_DISTANCE 1.0
#define STUD_AND_A_HALF_DISTANCE 1.5

#define STRONGLY_CONNECTED_BRICK_POSITIONS 92

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

  void constructAllStronglyConnected(RectilinearBrick *bricks, int &bricksSize, bool includeCorners) const;

  void getConnectionPointsAbove(ConnectionPoint *pts, int brickI);
  void getConnectionPointsBelow(ConnectionPoint *pts, int brickI);

  void serialize(std::ofstream &os) const;
  void deserialize(std::ifstream &is);

  void toLDR(std::ofstream &os, int x, int y, int ldrColor) const;

  bool atConnectingLevelOf(const ConnectionPoint &p) const;
  bool blocks(const ConnectionPoint &p) const;
  bool angleLocks(const ConnectionPoint &p) const;

private:
  void constructAllStronglyConnected(RectilinearBrick *bricks, int &bricksSize, int level, bool includeCorners) const;
  void getConnectionPoints(ConnectionPoint *pts, bool above, int brickI);
  bool inInterval(int8_t min, int8_t max, int8_t a) const;
  bool distLessThan2(int8_t a, int8_t b) const;
};

std::ostream& operator<<(std::ostream &os, const RectilinearBrick& b);

#endif // RECTILINEAR_BRICK_H
