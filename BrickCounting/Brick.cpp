#include "Brick.h"

#include <iostream>
#include <iomanip>
#include <assert.h>
#include <math.h>
#include <cmath>

double round(double number) {
  return number < 0.0 ? ceil(number - 0.5) : floor(number + 0.5);
}

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
  if(p.brick.horizontal())
    angle += M_PI/2;
  //std::cout << "Building brick. RB=" << b << std::endl << " center compared to connection: " << center.X << "," << center.Y << std::endl << " point to connect to: " << origin.X << "," << origin.Y << ", angle of that point: " << originAngle << std::endl;
  // center is now on b. Move by turn angle, then translate to origin:
  // Rotate:
  double sina = sin(angle);
  double cosa = cos(angle);
  double oldX = center.X;
  center.X = center.X*cosa - center.Y*sina;
  center.Y = oldX*sina + center.Y*cosa;

  //std::cout << " Center compared to connection after rotating " << angle << ": " << center.X << "," << center.Y << std::endl;

  // Translate:
  center.X += origin.X;
  center.Y += origin.Y;
  if(b.horizontal())
    angle -= M_PI/2;
  //std::cout << " After translation: " << center.X << "," << center.Y << "," << ((int)level) << " angle " << angle << std::endl;
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

void Brick::moveBrickSoThisIsAxisAlignedAtOrigin(Brick &b) const {
  // Tranlate to make this->origin = 0,0:
  b.center.X -= center.X;
  b.center.Y -= center.Y;
  // Rotate to make this->angle = 0.
  double sina = sin(-angle);
  double cosa = cos(-angle);
  double oldX = b.center.X;
  b.center.X = oldX*cosa - b.center.Y*sina;
  b.center.Y = oldX*sina + b.center.Y*cosa;
  b.angle -= angle;
}

void Brick::getBoxPOIs(Point *pois) const {
  double sina = sin(angle);
  double cosa = cos(angle);
  // 4 corners:
  double dx = VERTICAL_BRICK_CENTER_TO_SIDE;
  double dy = VERTICAL_BRICK_CENTER_TO_TOP;
  pois[0] = Point(center.X+(dx*cosa-dy*sina),  center.Y+(dx*sina+dy*cosa));
  pois[1] = Point(center.X+(-dx*cosa-dy*sina), center.Y+(-dx*sina+dy*cosa));
  pois[2] = Point(center.X+(dx*cosa+dy*sina),  center.Y+(dx*sina-dy*cosa));
  pois[3] = Point(center.X+(-dx*cosa+dy*sina), center.Y+(-dx*sina-dy*cosa));
  // 4 sides:
  pois[4] = Point(center.X+dx*cosa,  center.Y+dx*sina);
  pois[5] = Point(center.X-dx*cosa, center.Y-dx*sina);
  pois[6] = Point(center.X+dy*sina,  center.Y+dy*cosa);
  pois[7] = Point(center.X-dy*sina, center.Y-dy*cosa);
  // 2 inner:
  pois[8] = Point(center.X+0.75*sina, center.Y+0.75*cosa);
  pois[9] = Point(center.X-0.75*sina, center.Y-0.75*cosa);
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

bool Brick::boxIntersectsPOIsFrom(Brick &b) const {
  //std::cout << "Testing " << *this << " vs " << b << std::endl; 
  moveBrickSoThisIsAxisAlignedAtOrigin(b);
  // Get POIs:
  Point pois[NUMBER_OF_POIS_FOR_BOX_INTERSECTION];
  b.getBoxPOIs(pois);
  // Check each POI:
  for(int i = 0; i < NUMBER_OF_POIS_FOR_BOX_INTERSECTION; ++i) {
    Point &poi = pois[i];
    if(poi.X < 0)
      poi.X = -poi.X;
    if(poi.Y < 0)
      poi.Y = -poi.Y;
    if(poi.X < VERTICAL_BRICK_CENTER_TO_SIDE && poi.Y < VERTICAL_BRICK_CENTER_TO_TOP) {
      //std::cout << *this << " intersects " << b << std::endl; 
      return true;      
    }
  }
  return false;
}

/*
Assumes b is on same level.
*/
bool Brick::boxesIntersect(const Brick &b) const {
  Brick tmpB(b);
  Brick tmpThis(*this);
  return boxIntersectsPOIsFrom(tmpB) || b.boxIntersectsPOIsFrom(tmpThis);
}

bool Brick::boxIntersectsStudsFrom(Brick &b, const RectilinearBrick &bSource, bool &connected, ConnectionPoint &foundConnectionB, ConnectionPoint &foundConnectionThis, const RectilinearBrick &source) const {
  //std::cout << " Checking stud intersection between " << *this << " and " << b << std::endl;
  moveBrickSoThisIsAxisAlignedAtOrigin(b);
  //std::cout << "  After axis aligning this. b=" << b << std::endl;
  Point studsOfB[NUMBER_OF_STUDS];
  b.getStudPositions(studsOfB);
  // X handle four inner:
  for(int i = 0; i < 4; ++i) {
    Point &stud = studsOfB[i];
    if(stud.X < 0)
      stud.X = -stud.X;
    if(stud.Y < 0)
      stud.Y = -stud.Y;
    if(stud.X < VERTICAL_BRICK_CENTER_TO_SIDE+STUD_RADIUS && 
       stud.Y < VERTICAL_BRICK_CENTER_TO_TOP+STUD_RADIUS) {
      // Might intersect - check corner case:
      const double cornerX = VERTICAL_BRICK_CENTER_TO_SIDE-STUD_RADIUS;
      const double cornerY = VERTICAL_BRICK_CENTER_TO_TOP-STUD_RADIUS;
      if(stud.X < VERTICAL_BRICK_CENTER_TO_SIDE ||
         stud.Y < VERTICAL_BRICK_CENTER_TO_TOP ||
         STUD_DIAM*STUD_DIAM > (stud.X-cornerX)*(stud.X-cornerX)+(stud.Y-cornerY)*(stud.Y-cornerY)) {
          //std::cout << "  Corner case " << stud.X << "," << stud.Y << std::endl;
          connected = false;
          return true;
      }
    }
  }
  // Handle four outer specially as they might cause connection:
  for(int i = 4; i < NUMBER_OF_STUDS; ++i) {
    Point stud = studsOfB[i];
    //std::cout << "  stud: " << stud.X << "," << stud.Y << std::endl;
    if(stud.X < 0)
      stud.X = -stud.X;
    if(stud.Y < 0)
      stud.Y = -stud.Y;

    if(stud.X < VERTICAL_BRICK_CENTER_TO_SIDE+STUD_RADIUS && 
       stud.Y < VERTICAL_BRICK_CENTER_TO_TOP+STUD_RADIUS) {
      // X check if it hits stud:
      const double studX = HALF_STUD_DISTANCE;
      const double studY = STUD_AND_A_HALF_DISTANCE;   
      //std::cout << " snap distance sq " << SNAP_DISTANCE*SNAP_DISTANCE << " vs " << ((stud.X-studX)*(stud.X-studX)+(stud.Y-studY)*(stud.Y-studY)) << std::endl;
      if(SNAP_DISTANCE*SNAP_DISTANCE >= (stud.X-studX)*(stud.X-studX)+(stud.Y-studY)*(stud.Y-studY)) {
        // We are already connected:
        if(connected) {
          connected = false;
          //std::cout << "  Double outer stud intersection " << stud.X << "," << stud.Y << std::endl;
          return true;
        }
        // Compute corner:
        connected = true;
        if(studsOfB[i].X < 0 && studsOfB[i].Y < 0)
          foundConnectionThis = ConnectionPoint(SW, source, false, -1);
        else if(studsOfB[i].X < 0 && studsOfB[i].Y > 0)
          foundConnectionThis = ConnectionPoint(NW, source, false, -1);
        else if(studsOfB[i].X > 0 && studsOfB[i].Y < 0)
          foundConnectionThis = ConnectionPoint(SE, source, false, -1);
        else // if(stud.X > 0 && stud.Y > 0)
          foundConnectionThis = ConnectionPoint(NE, source, false, -1);
        foundConnectionB = ConnectionPoint((ConnectionPointType)(i-4), bSource, true, -1);
        continue;
      }

      // Might intersect - check corner case:
      const double cornerX = VERTICAL_BRICK_CENTER_TO_SIDE-STUD_RADIUS;
      const double cornerY = VERTICAL_BRICK_CENTER_TO_TOP-STUD_RADIUS;
      if(stud.X < VERTICAL_BRICK_CENTER_TO_SIDE ||
         stud.Y < VERTICAL_BRICK_CENTER_TO_TOP ||
         STUD_DIAM*STUD_DIAM > (stud.X-cornerX)*(stud.X-cornerX)+(stud.Y-cornerY)*(stud.Y-cornerY)) {
          //std::cout << "  Corner case " << stud.X << "," << stud.Y << std::endl;
          connected = false;
          return true;
      }
    }
  }
  //std::cout << "  All checked. No problems. Returning connected=" << connected << std::endl;
  return connected;
}

/*
1: If not even close level-wise: return false.
2: If on same level: Check that boxes do not collide.
- If current implementation doesn't work, use algorithm from http://www.ragestorm.net/tutorial?id=22
3: If on adjacent levels:
- If any of the 8 studs of lower brick inside box of upper: intersect.
- If intersect: Check if only intersecting one corner stud.
- If only one corner stud, check if corner connected.
*/
bool Brick::intersects(const Brick &b, const RectilinearBrick &bSource, bool &connected, ConnectionPoint &foundConnectionB, ConnectionPoint &foundConnectionThis, const RectilinearBrick &source) const {
  //std::cout << "! Checking intersection between " << *this << " and " << b << std::endl;
  connected = false;
  if(level > b.level+1 || b.level > level+1)
    return false;
  if(level == b.level) {
    bool ret = boxesIntersect(b);
    return ret;
  }
  Brick tmpB(b);
  Brick tmpThis(*this);
  if(level < b.level)
    return b.boxIntersectsStudsFrom(tmpThis, source, connected, foundConnectionThis, foundConnectionB, bSource);
  return boxIntersectsStudsFrom(tmpB, bSource, connected, foundConnectionB, foundConnectionThis, source);
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
