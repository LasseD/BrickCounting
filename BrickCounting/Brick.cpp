#include "Brick.h"

#include <iostream>
#include <iomanip>

/*
Constructor used for finding connection points:
b: original brick placed in space.
rb: rb in same configuration as b where b is original brick.
 */
Brick::Brick(const Brick& b, const RectilinearBrick& rb) : angle(b.angle), level(b.level + rb.level()) {
  float sina = sin(angle);
  float cosa = cos(angle);
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
Brick::Brick(const RectilinearBrick& b, const ConnectionPoint& p, const Point &origin, float originAngle, int8_t originLv) : center(b.x-p.x(), b.y-p.y()), angle(originAngle), level(b.level()+originLv) {  
  std::cout << "Building brick. RB=" << b << std::endl << " center compared to connection: " << center.X << "," << center.Y << std::endl << " point to connect to: " << origin.X << "," << origin.Y << ", angle of that point: " << originAngle << std::endl;
  // center is now on b. Move by turn angle, then translate to origin:
  // Rotate:
  float sina = sin(angle);
  float cosa = cos(angle);
  float oldX = center.X;
  center.X = center.X*cosa - center.Y*sina;
  center.Y = oldX*sina + center.Y*cosa;

  std::cout << " Center compared to connection after rotating " << angle << ": " << center.X << "," << center.Y << std::endl;
  /*if(!b.horizontal()) { // Vertical!
    angle += M_PI/2;
  }//*/

  // Translate:
  center.X += origin.X;
  center.Y += origin.Y;
  std::cout << " After translation: " << center.X << "," << center.Y << "," << ((int)level) << " angle " << angle << std::endl;
}

void Brick::toLDR(std::ofstream &os, int xx, int yy, int ldrColor) const {
  float x = (this->center.X)*20;
  float y = (this->center.Y)*20;
  int z = -24*level;

  float sina = sin(-angle+M_PI/2);
  float cosa = cos(-angle+M_PI/2);

  // Brick:
  os << std::fixed;
  os << "1 " << ldrColor << " " << x << " " << z << " " << y << " ";
  os << cosa << " 0 " << sina << " 0 1 0 " << -sina << " 0 " << cosa << " 3001.dat" << std::endl;

  
  // Guide axle:
  Point p = getStudPosition(NW);
  os << "1 " << ldrColor << " " << (p.X*20) << " " << (z+8-8) << " " << (p.Y*20) << " ";
  os << "0 -1 0 1 0 0 0 0 1 4519.dat" << std::endl;
  // Guide hose:
  p = getStudPosition(NE);
  os << "1 " << ldrColor << " " << (p.X*20) << " " << (z+40-8) << " " << (p.Y*20) << " ";
  os << "-1 0 0 0 -1 0 0 0 1 76263.dat" << std::endl;
  // Guide pin:
  p = getStudPosition(SE);
  os << "1 " << ldrColor << " " << (p.X*20) << " " << (z+8-8) << " " << (p.Y*20) << " ";
  os << "0 1 0 -1 0 0 0 0 1 32556.dat" << std::endl;
  // Low guide axle:
  p = getStudPosition(SW);
  os << "1 " << 0 << " " << (p.X*20) << " " << (z+8) << " " << (p.Y*20) << " ";
  os << "0 -1 0 1 0 0 0 0 1 4519.dat" << std::endl;//*/

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

void Brick::moveBrickSoThisIsAxisAlignedAtOrigin(Brick &b) const {
  // Tranlate to make this->origin = 0,0:
  b.center.X = b.center.X-center.X;
  b.center.Y = b.center.Y-center.Y;
  // Rotate to make this->angle = 0.
  float sina = sin(-angle);
  float cosa = cos(-angle);
  b.center.X = b.center.X*cosa - b.center.Y*sina;
  b.center.Y = b.center.X*sina + b.center.Y*cosa;
  b.angle = b.angle-angle;
}

void Brick::getBoxPOIs(Point *pois) const {
  float sina = sin(angle);
  float cosa = cos(angle);
  // 4 corners:
  float dx = VERTICAL_BRICK_CENTER_TO_SIDE;
  float dy = VERTICAL_BRICK_CENTER_TO_TOP;
  pois[0] = Point(center.X+(dx*cosa-dy*sina),  center.Y+(dx*sina+dy*cosa));
  pois[1] = Point(center.X+(-dx*cosa-dy*sina), center.Y+(-dx*sina+dy*cosa));
  pois[2] = Point(center.X+(dx*cosa+dy*sina),  center.Y+(dx*sina-dy*cosa));
  pois[3] = Point(center.X+(-dx*cosa+dy*sina), center.Y+(-dx*sina-dy*cosa));
  // 2 inner:
  pois[4] = Point(center.X+0.75*sina, center.Y+0.75*cosa);
  pois[5] = Point(center.X-0.75*sina, center.Y-0.75*cosa);
}

Point Brick::getStudPosition(ConnectionPointType type) const {
  float sina = sin(angle);
  float cosa = cos(angle);
  float dx, dy;
  switch(type) {
  case NW: dx = -HALF_STUD_DISTANCE; dy = STUD_AND_A_HALF_DISTANCE; break;
  case NE: dx = HALF_STUD_DISTANCE; dy = STUD_AND_A_HALF_DISTANCE; break;
  case SE: dx = HALF_STUD_DISTANCE; dy = -STUD_AND_A_HALF_DISTANCE; break;
  case SW: dx = -HALF_STUD_DISTANCE; dy = -STUD_AND_A_HALF_DISTANCE; break;
  default: dx = 0; dy = 0; std::cerr << "ARRR!" << std::endl; break;
  }
  float dx2 = dx*cosa - dy*sina;
  float dy2 = dx*sina + dy*cosa;
  return Point(center.X+dx2, center.Y+dy2);
}

void Brick::getStudPositions(Point *studs) const {
  float sina = sin(angle);
  float cosa = cos(angle);
  // 4 inner:
  float dx = HALF_STUD_DISTANCE;
  float dy = HALF_STUD_DISTANCE;
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
    if(poi.X < VERTICAL_BRICK_CENTER_TO_SIDE && poi.Y < VERTICAL_BRICK_CENTER_TO_TOP)
      return true;
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

 bool Brick::boxIntersectsStudsFrom(Brick &b, const RectilinearBrick &bSource, bool &connected, Connection &foundConnection, const RectilinearBrick &source) const {
  moveBrickSoThisIsAxisAlignedAtOrigin(b);
  Point studsOfB[NUMBER_OF_STUDS];
  b.getStudPositions(studsOfB);
  // X handle four inner:
  for(int i = 0; i < 4; ++i) {
    Point &stud = studsOfB[i];
    if(stud.X < 0)
      stud.X = -stud.X;
    if(stud.Y < 0)
      stud.Y = -stud.Y;
    if(stud.X < VERTICAL_BRICK_CENTER_TO_SIDE && stud.Y < VERTICAL_BRICK_CENTER_TO_TOP) {
      // Might intersect - check corner case:
      const float cornerX = VERTICAL_BRICK_CENTER_TO_SIDE-STUD_RADIUS;
      const float cornerY = VERTICAL_BRICK_CENTER_TO_TOP-STUD_RADIUS;
      if(stud.X > cornerX && stud.Y > cornerY) {
	return STUD_RADIUS*STUD_RADIUS > (stud.X-cornerX)*(stud.X-cornerX)+(stud.Y-cornerY)*(stud.Y-cornerY);
      }
      return true;
    }
  }
  // Handle four outer specially as they might cause connection:
  for(int i = 4; i < NUMBER_OF_STUDS; ++i) {
    Point stud = studsOfB[i];
    if(stud.X < 0)
      stud.X = -stud.X;
    if(stud.Y < 0)
      stud.Y = -stud.Y;

    if(stud.X < VERTICAL_BRICK_CENTER_TO_SIDE && stud.Y < VERTICAL_BRICK_CENTER_TO_TOP) {
      // X check if it hits stud:
      const float studX = STUD_DISTANCE;
      const float studY = HALF_STUD_DISTANCE;    
      if(SNAP_DISTANCE*SNAP_DISTANCE >= (stud.X-studX)*(stud.X-studX)+(stud.Y-studY)*(stud.Y-studY)) {
	// We are already connected:
	if(connected) {
	  connected = false;
	  return true;
	}
	// Compute corner:
	connected = true;
	if(stud.X < 0 && stud.Y < 0)
	  foundConnection.first = ConnectionPoint(SW, source, true);
	else if(stud.X < 0 && stud.Y > 0)
	  foundConnection.first = ConnectionPoint(NW, source, true);
	else if(stud.X > 0 && stud.Y < 0)
	  foundConnection.first = ConnectionPoint(SE, source, true);
	else // if(stud.X > 0 && stud.Y > 0)
	  foundConnection.first = ConnectionPoint(NE, source, true);
	foundConnection.second = ConnectionPoint((ConnectionPointType)(i-4), bSource, false);
      }      

      // Might intersect - check corner case:
      const float cornerX = VERTICAL_BRICK_CENTER_TO_SIDE-STUD_RADIUS;
      const float cornerY = VERTICAL_BRICK_CENTER_TO_TOP-STUD_RADIUS;
      if(stud.X > cornerX && stud.Y > cornerY) {
	return STUD_RADIUS*STUD_RADIUS > (stud.X-cornerX)*(stud.X-cornerX)+(stud.Y-cornerY)*(stud.Y-cornerY);
      }
      return true;
    }
  }
  return connected;
}

/*
  1: If not even close level-wise: return false.
  2: If on same level: Check that boxes do not collide.
   - If current implementation doesn't work, use algorithm from http://www.ragestorm.net/tutorial?id=22 TODO
  3: If on adjacent levels:
   - If any of the 8 studs of lower brick inside box of upper: intersect.
   - If intersect: Check if only intersecting one corner stud.
   - If only one corner stud, check if corner connected.
 */
bool Brick::intersects(const Brick &b, const RectilinearBrick &bSource, bool &connected, Connection &foundConnection, const RectilinearBrick &source) const {
  connected = false;
  if(level > b.level+1 || b.level > level+1)
    return false;
  if(level == b.level)
    return boxesIntersect(b);
  Brick tmpB(b);
  Brick tmpThis(b);
  if(level < b.level)
    return b.boxIntersectsStudsFrom(tmpThis, source, connected, foundConnection, bSource);
  return boxIntersectsStudsFrom(tmpB, source, connected, foundConnection, bSource);
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
