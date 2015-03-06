#include "Brick.h"
#define _USE_MATH_DEFINES
#include <math.h>

Brick::Brick(const RectilinearBrick& b) : x(b.x), y(b.y), level(b.level()) {
  if(b.horizontal())
    angle = M_PI/2;
  else
    angle = 0;
}
