#ifndef BRICK_H
#define BRICK_H

#include "RectilinearBrick.h"
#include "LDRPrinter.h"

#include <stdint.h>
#include <iostream>

class Brick : LDRPrinter {
public:
  float x, y, angle; // angle 0 = vertical.
  uint8_t level;

  Brick(const RectilinearBrick& b);

  void toLDR(std::ostream &os, int x, int y, int ldrColor);
};

#endif // BRICK_H
