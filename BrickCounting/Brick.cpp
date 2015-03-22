#include "Brick.h"
#define _USE_MATH_DEFINES
//#include <math.h>
#include <tgmath.h>

/*
 Main constructor: Built from a RectilinearConfiguration by connecting to a brick at an angle.
 */
Brick::Brick(const RectilinearBrick& b, Point origin, float originAngle, int8_t originLv) : center(b.x, b.y), angle(originAngle), level(b.level()+originLv) {  
  if(!b.horizontal()) {
    angle -= M_PI/2;
    center.X--;
    center.Y++;
  }

  // center is now on b. Move by turn angle, then translate to origin:
  // Rotate:
  float sina = sin(angle);
  float cosa = cos(angle);
  center.X = center.X*cosa - center.Y*sina;
  center.Y = center.X*sina + center.Y*cosa;
  // Translate:
  center.X += origin.X;
  center.Y -= origin.Y;
}

void Brick::toLDR(std::ofstream &os, int xx, int yy, int ldrColor) const {
  float x = (this->center.X)*20;
  float y = (this->center.Y)*20;
  int z = -24*level;

  os << "1 " << ldrColor << " " << y << " " << z << " " << x << " ";

  float sina = sin(angle);
  float cosa = cos(angle);

  os << cosa << " 0 " << sina << " 0 1 0 " << -sina << " 0 " << cosa << " 3001.dat" << std::endl;
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
  float dx = HORIZONTAL_BRICK_CENTER_TO_SIDE*cosa - HORIZONTAL_BRICK_CENTER_TO_TOP*sina;
  float dy = HORIZONTAL_BRICK_CENTER_TO_SIDE*sina + HORIZONTAL_BRICK_CENTER_TO_TOP*cosa;
  pois[0] = Point(center.X+dx, center.Y+dy);
  pois[1] = Point(center.X-dx, center.Y+dy);
  pois[2] = Point(center.X+dx, center.Y-dy);
  pois[3] = Point(center.X-dx, center.Y-dy);
  // 2 inner:
  pois[4] = Point(center.X+0.75*cosa, center.Y+0.75*sina);
  pois[5] = Point(center.X+0.75*cosa, center.Y-0.75*sina);
}

Point Brick::getStudPosition(ConnectionPointType type) const {
  float sina = sin(angle);
  float cosa = cos(angle);
  float dx2 = STUD_DISTANCE*cosa - HALF_STUD_DISTANCE*sina;
  float dy2 = STUD_DISTANCE*sina + HALF_STUD_DISTANCE*cosa;
  switch(type) {
  case NW: return Point(center.X+dx2, center.Y+dy2);
  case NE: return Point(center.X+dx2, center.Y-dy2);
  case SE: return Point(center.X-dx2, center.Y-dy2);
  case SW: return Point(center.X-dx2, center.Y+dy2);
  default:
    std::cerr << "ERROR: UNKNOWN CONNECTION POIN TYPE: " << type << std::endl;
    return Point();
  }
}

void Brick::getStudPositions(Point *studs) const {
  float sina = sin(angle);
  float cosa = cos(angle);
  // 4 inner:
  float dx1 = HALF_STUD_DISTANCE*cosa - HALF_STUD_DISTANCE*sina;
  float dy1 = HALF_STUD_DISTANCE*sina + HALF_STUD_DISTANCE*cosa;
  studs[0] = Point(center.X+dx1, center.Y+dy1);
  studs[1] = Point(center.X-dx1, center.Y+dy1);
  studs[2] = Point(center.X+dx1, center.Y-dy1);
  studs[3] = Point(center.X-dx1, center.Y-dy1);
  // 4 outer:
  float dx2 = STUD_DISTANCE*cosa - HALF_STUD_DISTANCE*sina;
  float dy2 = STUD_DISTANCE*sina + HALF_STUD_DISTANCE*cosa;
  studs[4] = Point(center.X+dx2, center.Y+dy2);
  studs[5] = Point(center.X+dx2, center.Y-dy2);
  studs[6] = Point(center.X-dx2, center.Y-dy2);
  studs[7] = Point(center.X-dx2, center.Y+dy2);
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
    if(poi.X < HORIZONTAL_BRICK_CENTER_TO_SIDE && poi.Y < HORIZONTAL_BRICK_CENTER_TO_TOP)
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
    if(stud.X < HORIZONTAL_BRICK_CENTER_TO_SIDE && stud.Y < HORIZONTAL_BRICK_CENTER_TO_TOP) {
      // Might intersect - check corner case:
      const float cornerX = HORIZONTAL_BRICK_CENTER_TO_SIDE-STUD_RADIUS;
      const float cornerY = HORIZONTAL_BRICK_CENTER_TO_TOP-STUD_RADIUS;
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

    if(stud.X < HORIZONTAL_BRICK_CENTER_TO_SIDE && stud.Y < HORIZONTAL_BRICK_CENTER_TO_TOP) {
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
      const float cornerX = HORIZONTAL_BRICK_CENTER_TO_SIDE-STUD_RADIUS;
      const float cornerY = HORIZONTAL_BRICK_CENTER_TO_TOP-STUD_RADIUS;
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
