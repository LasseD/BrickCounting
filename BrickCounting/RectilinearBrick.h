#ifndef RECTILINEAR_BRICK_H
#define RECTILINEAR_BRICK_H

#include <iostream>
#include <fstream>

enum ConnectionPointType {NW, NE, SW, SE};

struct ConnectionPoint {
  ConnectionPointType type;
  bool isAbove;
  ConnectionPoint(ConnectionPointType type, bool isAbove) : type(type), isAbove(isAbove) {}
};

class RectilinearBrick {
public:
  int x, y, level;
  bool horizontal;
  RectilinearBrick(int x, int y, int level, bool horizontal) : x(x), y(y), level(level), horizontal(horizontal) {}
  RectilinearBrick() : x(0), y(0), level(0), horizontal(false) {}

  bool operator < (const RectilinearBrick &b) const;
  bool operator == (const RectilinearBrick &b) const;
  bool operator != (const RectilinearBrick &b) const;

  bool intersects(const RectilinearBrick &b) const;
  
  void constructAllStronglyConnected(RectilinearBrick *bricks, int &bricksSize);
  void constructAllStronglyConnected(RectilinearBrick *bricks, int &bricksSize, int level);
  
  //void getConnectionPointsAbove(ConnectionPoint *pts, int &sizePts);
  //void getConnectionPointsBelow(ConnectionPoint *pts, int &sizePts);
  
  void serialize(std::ofstream &os) const;
  void deserialize(std::ifstream &is);

private:
  /*void getConnectionPointsNoLevel(ConnectionPoint *pts, int &sizePts);
    void setConnectionPointsDataNoLevel(ConnectionPoint &pt, bool isAbove, ConnectionPointType type) {
    pt.isAbove = isAbove;
    pt.type = type;
    }*/
  bool inInterval(int min, int max, int a) const {
    return  min <= a && a <= max;
  }
  bool distLessThan2(int a, int b) const {
    return a == b || a-1 == b || b-1 == a;
  }
};

std::ostream& operator<<(std::ostream &os, const RectilinearBrick& b);

#endif
