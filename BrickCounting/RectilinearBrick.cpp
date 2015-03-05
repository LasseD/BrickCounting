#include "RectilinearBrick.h"

bool RectilinearBrick::operator < (const RectilinearBrick &b) const {
  if(level != b.level)
    return level < b.level;
  if(x != b.x)
    return x < b.x;
  if(y != b.y)
    return y < b.y;
  return horizontal < b.horizontal;
}
bool RectilinearBrick::operator == (const RectilinearBrick &b) const {
  return (level == b.level) && (x == b.x) && (y == b.y) && (horizontal == b.horizontal);
}
bool RectilinearBrick::operator != (const RectilinearBrick &b) const {
  return (level != b.level) || (x != b.x) || (y != b.y) || (horizontal != b.horizontal);
}

bool RectilinearBrick::intersects(const RectilinearBrick &b) const {
  if(b.level != level) {
    return false;
  }
  else if(horizontal && b.horizontal) {
    return inInterval(x-3, x+3, b.x) && inInterval(y-1, y+1, b.y);
  }
  else if(!horizontal && !b.horizontal) {
    return inInterval(x-1, x+1, b.x) && inInterval(y-3, y+3, b.y);
  }
  else if(horizontal && !b.horizontal) {
    return inInterval(x-1, x+3, b.x) && inInterval(y-3, y+1, b.y);
  }
  else {
    return inInterval(x-3, x+1, b.x) && inInterval(y-1, y+3, b.y);
  }
}

void RectilinearBrick::constructAllStronglyConnected(RectilinearBrick *bricks, int &bricksSize) const {
  if(level > 0)
    constructAllStronglyConnected(bricks, bricksSize, level-1);  
  constructAllStronglyConnected(bricks, bricksSize, level+1);  
}

void RectilinearBrick::constructAllStronglyConnected(RectilinearBrick *bricks, int &bricksSize, int level) const {
  if(horizontal) {
    // all horizontal:
    for(int xx = x-2; xx <= x+2; ++xx) {
      for(int yy = y-1; yy <= y+1; ++yy) {
        bricks[bricksSize++] = RectilinearBrick(xx, yy, level, true);
      }
    }
    bricks[bricksSize++] = RectilinearBrick(x-3, y, level, true);
    bricks[bricksSize++] = RectilinearBrick(x+3, y, level, true);
    // all vertical:
    for(int xx = x-1; xx <= x+3; ++xx) {
      bool xExtreme = xx == x-1 || xx == x+3;
      for(int yy = y-3; yy <= y+1; ++yy) {
        bool yExtreme = yy == y-3 || yy == y+1;
        if(!(xExtreme && yExtreme))
          bricks[bricksSize++] = RectilinearBrick(xx, yy, level, false);
      }
    }
  }
  else { // vertical
    // all vertical:
    for(int xx = x-1; xx <= x+1; ++xx) {
      for(int yy = y-2; yy <= y+2; ++yy) {
        bricks[bricksSize++] = RectilinearBrick(xx, yy, level, false);
      }
    }
    bricks[bricksSize++] = RectilinearBrick(x, y-3, level, false);
    bricks[bricksSize++] = RectilinearBrick(x, y+3, level, false);
    // all horizontal:
    for(int xx = x-3; xx <= x+1; ++xx) {
      bool xExtreme = xx == x-3 || xx == x+1;
      for(int yy = y-1; yy <= y+3; ++yy) {
        bool yExtreme = yy == y-1 || yy == y+3;
        if(!(xExtreme && yExtreme))
          bricks[bricksSize++] = RectilinearBrick(xx, yy, level, true);
      }
    }
  }
}

void RectilinearBrick::serialize(std::ofstream &os) const {
  os.write(reinterpret_cast<const char *>(&x), sizeof(int));
  os.write(reinterpret_cast<const char *>(&y), sizeof(int));
  os.write(reinterpret_cast<const char *>(&level), sizeof(int));
  os.write(reinterpret_cast<const char *>(&horizontal), 1);
}
void RectilinearBrick::deserialize(std::ifstream &is) {
  is.read((char*)&x, sizeof(int));
  is.read((char*)&y, sizeof(int));
  is.read((char*)&level, sizeof(int));
  is.read((char*)&horizontal, 1);
}

std::ostream& operator<<(std::ostream& os, const RectilinearBrick& b)
{
  os << "[RectilinearBrick:x="<<b.x<<",y="<< b.y <<",lv="<< b.level<< (b.horizontal?",horizontal]":",vertical]");
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

