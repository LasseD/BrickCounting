#include "Brick.h"
#define _USE_MATH_DEFINES
#include <math.h>

Brick::Brick(const RectilinearBrick& b) : x(b.x), y(b.y), level(b.level()) {
  if(b.horizontal())
    angle = M_PI/2;
  else
    angle = 0;
}

Brick::toLDR(std::ostream &os, int x, int y, int ldrColor) {
  x *= 20;
  y *= 20;
  z *= -24;
  os << "1 " << ldrColor << " " << x << " " << z << " " << y <<  " 0 0 0 1 0 0 0 1 0 0 0 1 3001.dat" << std::endl;
}
