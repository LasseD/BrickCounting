#include "RectilinearBrick.h"
#include <assert.h>

bool RectilinearBrick::operator < (const RectilinearBrick &b) const {
  if(level() != b.level())
    return level() < b.level();
  if(horizontal() != b.horizontal())
    return b.horizontal();

  if(x != b.x)
    return x < b.x;
  return y < b.y;
}
bool RectilinearBrick::operator == (const RectilinearBrick &b) const {
  return (levelShifted == b.levelShifted) && (x == b.x) && (y == b.y);
}
bool RectilinearBrick::operator != (const RectilinearBrick &b) const {
  return (levelShifted != b.levelShifted) || (x != b.x) || (y != b.y);
}
bool RectilinearBrick::horizontal() const {
  return (levelShifted & 1) == 1; 
}
int RectilinearBrick::level() const {
  return levelShifted >> 1;
}
void RectilinearBrick::setHorizontalTrue() {
  levelShifted |= 1;
  assert(level() >= 0);
  assert(level() <= 5);
}
void RectilinearBrick::setHorizontalFalse() {
  levelShifted &= ~1;
  assert(level() >= 0);
  assert(level() <= 5);
}

bool RectilinearBrick::intersects(const RectilinearBrick &b) const {
  if(b.level() != level()) {
    return false;
  }
  bool h = horizontal();
  bool bh = b.horizontal();
  if(h && bh) {
    return inInterval(x-3, x+3, b.x) && inInterval(y-1, y+1, b.y);
  }
  if(!h && !bh) {
    return inInterval(x-1, x+1, b.x) && inInterval(y-3, y+3, b.y);
  }
  if(h && !bh) {
    return inInterval(x-1, x+3, b.x) && inInterval(y-3, y+1, b.y);
  }
  return inInterval(x-3, x+1, b.x) && inInterval(y-1, y+3, b.y);
}

void RectilinearBrick::constructAllStronglyConnected(RectilinearBrick *bricks, int &bricksSize) const {
  int lv = level();
  if(lv > 0)
    constructAllStronglyConnected(bricks, bricksSize, lv-1);  
  constructAllStronglyConnected(bricks, bricksSize, lv+1);  
}

void RectilinearBrick::constructAllStronglyConnected(RectilinearBrick *bricks, int &bricksSize, int level) const {
  if(horizontal()) {
    // all horizontal:
    for(int8_t xx = x-2; xx <= x+2; ++xx) {
      for(int8_t yy = y-1; yy <= y+1; ++yy) {
        bricks[bricksSize++] = RectilinearBrick(xx, yy, level, true);
      }
    }
    bricks[bricksSize++] = RectilinearBrick(x-3, y, level, true);
    bricks[bricksSize++] = RectilinearBrick(x+3, y, level, true);
    // all vertical:
    for(int8_t xx = x-1; xx <= x+3; ++xx) {
      bool xExtreme = xx == x-1 || xx == x+3;
      for(int8_t yy = y-3; yy <= y+1; ++yy) {
        bool yExtreme = yy == y-3 || yy == y+1;
        if(!(xExtreme && yExtreme))
          bricks[bricksSize++] = RectilinearBrick(xx, yy, level, false);
      }
    }
  }
  else { // vertical
    // all vertical:
    for(int8_t xx = x-1; xx <= x+1; ++xx) {
      for(int8_t yy = y-2; yy <= y+2; ++yy) {
        bricks[bricksSize++] = RectilinearBrick(xx, yy, level, false);
      }
    }
    bricks[bricksSize++] = RectilinearBrick(x, y-3, level, false);
    bricks[bricksSize++] = RectilinearBrick(x, y+3, level, false);
    // all horizontal:
    for(int8_t xx = x-3; xx <= x+1; ++xx) {
      bool xExtreme = xx == x-3 || xx == x+1;
      for(int8_t yy = y-1; yy <= y+3; ++yy) {
        bool yExtreme = yy == y-1 || yy == y+3;
        if(!(xExtreme && yExtreme))
          bricks[bricksSize++] = RectilinearBrick(xx, yy, level, true);
      }
    }
  }
}

void RectilinearBrick::serialize(std::ofstream &os) const {
  os.write(reinterpret_cast<const char *>(&x), 1);
  os.write(reinterpret_cast<const char *>(&y), 1);
  os.write(reinterpret_cast<const char *>(&levelShifted), 1);
}
void RectilinearBrick::deserialize(std::ifstream &is) {
  is.read((char*)&x, 1);
  is.read((char*)&y, 1);
  is.read((char*)&levelShifted, 1);
}

void RectilinearBrick::toLDR(std::ofstream &os, int x, int y, int ldrColor) const {
  x += this->x;
  y += this->y;
  if(horizontal()) {
    x += 2;
    y += 1;
  }
  else {
    x += 1;
    y += 2;
  }

  x *= 20;
  y *= 20;
  int z = -24*level();

  os << "1 " << ldrColor << " " << y << " " << z << " " << x << " ";

  if(horizontal()) {
    os << "0 0 1 0 1 0 -1 0 0 3001.dat" << std::endl;
  }
  else {
    os << "1 0 0 0 1 0 0 0 1 3001.dat" << std::endl;
  }
}

std::ostream& operator<<(std::ostream& os, const RectilinearBrick& b)
{
  os<< (b.horizontal()?"[-":"[|") << " "<<((int)b.x)<<","<< ((int)b.y) <<", lv "<< b.level() << "]";
  return os;
}

/*void RectilinearBrick::getConnectionPointsAbove(ConnectionPoint *pts, int &sizePts) {
getConnectionPointsNoLevel(pts, sizePts);
for(unsigned int i = 0; i < 4; ++i) {
pts[i].level = level+1;
}
}
void RectilinearBrick::getConnectionPointsBelow(ConnectionPoint *pts, int &sizePts) {
getConnectionPointsNoLevel(pts, sizePts);
for(unsigned int i = 0; i < 4; ++i) {
pts[i].level = level-1;
}
}
void RectilinearBrick::getConnectionPointsNoLevel(ConnectionPoint *pts, int &sizePts) {
sizePts = 4;
setConnectionPointsDataNoLevel(pts[0], x, y, CONNECTION_DOWN_LEFT);
if(horizontal) {
setConnectionPointsDataNoLevel(pts[1], x+3, y, CONNECTION_DOWN_RIGHT);
setConnectionPointsDataNoLevel(pts[2], x, y+1, CONNECTION_UP_LEFT);
setConnectionPointsDataNoLevel(pts[3], x+3, y+1, CONNECTION_UP_RIGHT);
}
else {
setConnectionPointsDataNoLevel(pts[1], x+1, y, CONNECTION_DOWN_RIGHT);
setConnectionPointsDataNoLevel(pts[2], x, y+3, CONNECTION_UP_LEFT);
setConnectionPointsDataNoLevel(pts[3], x+1, y+3, CONNECTION_UP_RIGHT);
}
}*/

