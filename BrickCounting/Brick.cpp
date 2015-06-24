#include "Brick.h"

#include <iostream>
#include <iomanip>
#include <assert.h>

/*
Constructor used for finding connection points:
b: original brick placed in space.
rb: rb in same configuration as b where b is original brick.
*/
Brick::Brick(const Brick& b, const RectilinearBrick& rb) : angle(b.angle), level(b.level + rb.level()) {
  double sina = sin(angle);
  double cosa = cos(angle);
  center.X = rb.x*cosa - rb.y*sina;
  center.Y = rb.x*sina + rb.y*cosa;  

  center.X += b.center.X;
  center.Y += b.center.Y;

  if(rb.horizontal())
    angle -= M_PI/2;
}

/*
Main constructor: Built from a RectilinearConfiguration by connecting to a brick at an angle.
*/
Brick::Brick(const RectilinearBrick& b, const ConnectionPoint& p, const Point &origin, double originAngle, int8_t originLv) : center(b.x-p.x(), b.y-p.y()), angle(originAngle), level(b.level()+originLv-p.brick.level()) {  
#ifdef _BRICK
  std::cout << "Building brick. RB=" << b << std::endl;
  std::cout << " center compared to connection: " << center.X << "," << center.Y << std::endl << " point to connect to: " << origin.X << "," << origin.Y << ", angle of that point: " << originAngle << std::endl;
  std::cout << " Level=" << (int)b.level() << "+" << (int)originLv << "-" << (int)p.brick.level() << std::endl;
#endif

  assert(level > -8);
  assert(level < 8);
  if(p.brick.horizontal())
    angle += M_PI/2;
  // center is now on b. Move by turn angle, then translate to origin:
  // Rotate:
  double sina = sin(angle);
  double cosa = cos(angle);
  double oldX = center.X;
  center.X = center.X*cosa - center.Y*sina;
  center.Y = oldX*sina + center.Y*cosa;

  // Translate:
  center.X += origin.X;
  center.Y += origin.Y;
  if(b.horizontal())
    angle -= M_PI/2;
}

void Brick::toLDR(std::ofstream &os, int xx, int yy, int ldrColor) const {
  double x = (this->center.X+xx)*20;
  double y = (this->center.Y+yy)*20;
  int z = -24*level;

  double sina = sin(-angle+M_PI/2);
  double cosa = cos(-angle+M_PI/2);

  // Brick:
  os << std::fixed;
  os << "1 " << ldrColor << " " << x << " " << z << " " << y << " ";
  os << cosa << " 0 " << sina << " 0 1 0 " << -sina << " 0 " << cosa << " 3001.dat" << std::endl;

  /*  
  // Guide axle:
  Point p = getStudPosition(NW);
  os << "1 " << ldrColor << " " << ((p.X+xx)*20) << " " << (z+8-8) << " " << ((p.Y+yy)*20) << " ";
  os << "0 -1 0 1 0 0 0 0 1 4519.dat" << std::endl;
  // Guide hose:
  p = getStudPosition(NE);
  os << "1 " << ldrColor << " " << ((p.X+xx)*20) << " " << (z+40-8) << " " << ((p.Y+yy)*20) << " ";
  os << "-1 0 0 0 -1 0 0 0 1 76263.dat" << std::endl;
  // Guide pin:
  p = getStudPosition(SE);
  os << "1 " << ldrColor << " " << ((p.X+xx)*20) << " " << (z+8-8) << " " << ((p.Y+yy)*20) << " ";
  os << "0 1 0 -1 0 0 0 0 1 32556.dat" << std::endl;
  // Low guide axle:
  p = getStudPosition(SW);
  os << "1 " << 0 << " " << ((p.X+xx)*20) << " " << (z+8) << " " << ((p.Y+yy)*20) << " ";
  os << "0 -1 0 1 0 0 0 0 1 4519.dat" << std::endl;//*/
  //*/
  /*
  // 8 studs:
  Point studs[8];
  getStudPositions(studs);
  for(int i = 0; i < 8; ++i) {
  Point p = studs[i];
  os << "1 " << 0 << " " << (p.X*20) << " " << (z-5*i) << " " << (p.Y*20) << " ";
  os << "0 -1 0 1 0 0 0 0 1 4519.dat" << std::endl;
  }//*/
  /*
  // 7 POIs:
  Point studs[6];
  getBoxPOIs(studs);
  for(int i = 0; i < 6; ++i) {
  Point p = studs[i];
  os << "1 " << 0 << " " << (p.X*20) << " " << (z) << " " << (p.Y*20) << " ";
  os << "0 -1 0 1 0 0 0 0 1 4519.dat" << std::endl;
  }//*/
}

RectilinearBrick Brick::toRectilinearBrick() const {
  int x = (int)round(this->center.X);
  int y = (int)round(this->center.Y);
  double a = angle*2/M_PI;
  if(a < 0)
    a = -a;
  bool horizontal = (((int)(a+0.5)) & 1) == 1;
  return RectilinearBrick((int8_t)x, (int8_t)y, level, horizontal);
}

void Brick::movePointSoThisIsAxisAlignedAtOrigin(Point &b) const {
  // Tranlate to make this->origin = 0,0:
  b.X -= center.X;
  b.Y -= center.Y;
  // Rotate to make this->angle = 0.
  double sina = sin(-angle);
  double cosa = cos(-angle);
  double oldX = b.X;
  b.X = oldX*cosa - b.Y*sina;
  b.Y = oldX*sina + b.Y*cosa;
}

void Brick::moveBrickSoThisIsAxisAlignedAtOrigin(Brick &b) const {
  movePointSoThisIsAxisAlignedAtOrigin(b.center);
  b.angle -= angle;
}

Point Brick::getStudPosition(ConnectionPointType type) const {
  double sina = sin(angle);
  double cosa = cos(angle);
  double dx, dy;
  switch(type) {
  case NW: dx = -HALF_STUD_DISTANCE; dy = STUD_AND_A_HALF_DISTANCE; break;
  case NE: dx = HALF_STUD_DISTANCE; dy = STUD_AND_A_HALF_DISTANCE; break;
  case SE: dx = HALF_STUD_DISTANCE; dy = -STUD_AND_A_HALF_DISTANCE; break;
  case SW: dx = -HALF_STUD_DISTANCE; dy = -STUD_AND_A_HALF_DISTANCE; break;
  default: dx = 0; dy = 0; std::cerr << "ARRR!" << std::endl; break;
  }
  double dx2 = dx*cosa - dy*sina;
  double dy2 = dx*sina + dy*cosa;
  return Point(center.X+dx2, center.Y+dy2);
}

void Brick::getStudPositions(Point *studs) const {
  double sina = sin(angle);
  double cosa = cos(angle);
  // 4 inner:
  double dx = HALF_STUD_DISTANCE;
  double dy = HALF_STUD_DISTANCE;
  studs[0] = Point(center.X+(-dx*cosa-dy*sina), center.Y+(-dx*sina+dy*cosa));
  studs[1] = Point(center.X+(dx*cosa-dy*sina),  center.Y+(dx*sina+dy*cosa));
  studs[2] = Point(center.X+(dx*cosa+dy*sina),  center.Y+(dx*sina-dy*cosa));
  studs[3] = Point(center.X+(-dx*cosa+dy*sina), center.Y+(-dx*sina-dy*cosa));
  // 4 outer:
  dy = STUD_AND_A_HALF_DISTANCE;
  studs[4] = Point(center.X+(-dx*cosa-dy*sina), center.Y+(-dx*sina+dy*cosa));
  studs[5] = Point(center.X+(dx*cosa-dy*sina),  center.Y+(dx*sina+dy*cosa));
  studs[6] = Point(center.X+(dx*cosa+dy*sina),  center.Y+(dx*sina-dy*cosa));
  studs[7] = Point(center.X+(-dx*cosa+dy*sina), center.Y+(-dx*sina-dy*cosa));
}

bool Brick::operator < (const Brick &b) const {
  if(center != b.center)
    return center < b.center;
  if(angle != b.angle)
    return angle < b.angle;  
  return level < b.level;
}

std::ostream& operator<<(std::ostream &os, const Brick& b) {
  os << "BRICK[" << b.center.X << "," << b.center.Y << "," << ((int)b.level) << ", angle " << b.angle << "]";
  return os;
}
