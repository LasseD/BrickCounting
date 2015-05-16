#include "RectilinearBrick.h"
#include "ConnectionPoint.h"

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
}
void RectilinearBrick::setHorizontalFalse() {
  levelShifted &= ~1;
}
void RectilinearBrick::flipHorizontal() {
  levelShifted ^= 1;
}

bool RectilinearBrick::intersects(const RectilinearBrick &b) const {
  if(b.level() != level()) {
    return false;
  }
  bool h = horizontal();
  bool bh = b.horizontal();
  if(h && bh) {
    return (x-3 <= b.x && b.x <= x+3) && (y-1 <= b.y && b.y <= y+1);
  }
  if(!h && !bh) {
    return x-1 <= b.x && b.x <= x+1 && y-3 <= b.y && b.y <= y+3;
  }
  return x-2 <= b.x && b.x <= x+2 && y-2 <= b.y && b.y <= y+2;
}

void RectilinearBrick::constructAllStronglyConnected(RectilinearBrick *bricks, int &bricksSize, bool includeCorners) const {
  int lv = level();
  if(lv > 0)
    constructAllStronglyConnected(bricks, bricksSize, lv-1, includeCorners);  
  constructAllStronglyConnected(bricks, bricksSize, lv+1, includeCorners);  
}

void RectilinearBrick::constructAllStronglyConnected(RectilinearBrick *bricks, int &bricksSize, int level, bool includeCorners) const {
  if(horizontal()) {
    // all horizontal:
    for(int8_t xx = x-3; xx <= x+3; ++xx) {
      bool xExtreme = xx == x-3 || xx == x+3;
      for(int8_t yy = y-1; yy <= y+1; ++yy) {
	bool yExtreme = yy == y-1 || yy == y+1;
	if(includeCorners || !(xExtreme && yExtreme))
	  bricks[bricksSize++] = RectilinearBrick(xx, yy, level, true);
      }
    }
  }
  else { // vertical
    // all vertical:
    for(int8_t xx = x-1; xx <= x+1; ++xx) {
      bool xExtreme = xx == x-1 || xx == x+1;
      for(int8_t yy = y-3; yy <= y+3; ++yy) {
	bool yExtreme = yy == y-3 || yy == y+3;
	if(includeCorners || !(xExtreme && yExtreme))
	  bricks[bricksSize++] = RectilinearBrick(xx, yy, level, false);
      }
    }
  }
  // all crossing:
  for(int8_t xx = x-2; xx <= x+2; ++xx) {
    bool xExtreme = xx == x-2 || xx == x+2;
    for(int8_t yy = y-2; yy <= y+2; ++yy) {
      bool yExtreme = yy == y-2 || yy == y+2;
      if(includeCorners || !(xExtreme && yExtreme))
	bricks[bricksSize++] = RectilinearBrick(xx, yy, level, !horizontal());
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

  x *= 20;
  y *= 20;
  int z = -24*level();

  os << "1 " << ldrColor << " " << x << " " << z << " " << y << " ";

  if(!horizontal()) {
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

void RectilinearBrick::getConnectionPointsAbove(ConnectionPoint *pts, int brickI) {
  getConnectionPoints(pts, true, brickI);
}
void RectilinearBrick::getConnectionPointsBelow(ConnectionPoint *pts, int brickI) {
  getConnectionPoints(pts, false, brickI);
}
void RectilinearBrick::getConnectionPoints(ConnectionPoint *pts, bool above, int brickI) {
  pts[0] = ConnectionPoint(NW, *this, above, brickI);
  pts[1] = ConnectionPoint(NE, *this, above, brickI);
  pts[2] = ConnectionPoint(SE, *this, above, brickI);
  pts[3] = ConnectionPoint(SW, *this, above, brickI);
}

bool RectilinearBrick::atConnectingLevelOf(const ConnectionPoint &p) const {
  return p.above ? level() == p.brick.level()+1 : level()+1 == p.brick.level();
}
bool RectilinearBrick::blocks(const ConnectionPoint &p) const {
  if(!atConnectingLevelOf(p))
    return false;
  if(horizontal()) {
    return (p.y4x4() == y+1 || p.y4x4() == y+2) && (p.x4x4() >= x && p.x4x4() <= x+3);
  }
  else {
    return (p.x4x4() == x+1 || p.x4x4() == x+2) && (p.y4x4() >= y && p.y4x4() <= y+3);
  }
}
bool RectilinearBrick::angleLocks(const ConnectionPoint &p) const {
  if(!atConnectingLevelOf(p))
    return false;
  if(horizontal()) {
    return (p.x4x4() == x-1 && inInterval(y+1,y+2,p.y4x4())) ||
      (p.x4x4() == x+4 && inInterval(y+1,y+2,p.y4x4())) ||
      (p.y4x4() == y && inInterval(x,x+3,p.x4x4())) ||
      (p.y4x4() == y+3 && inInterval(x,x+3,p.x4x4()));
  }
  else {
    return (p.y4x4() == y-1 && inInterval(x+1,x+2,p.x4x4())) ||
      (p.y4x4() == y+4 && inInterval(x+1,x+2,p.x4x4())) ||
      (p.x4x4() == x && inInterval(y,y+3,p.y4x4())) ||
      (p.x4x4() == x+3 && inInterval(y,y+3,p.y4x4()));
  }  
}

bool RectilinearBrick::isBase() const {
  return x == 0 && y == 0 && levelShifted == 0;
}

bool RectilinearBrick::inInterval(int8_t min, int8_t max, int8_t a) const {
  return  min <= a && a <= max;
}
bool RectilinearBrick::distLessThan2(int8_t a, int8_t b) const {
  return a == b || a-1 == b || b-1 == a;
}

