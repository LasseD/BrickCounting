#ifndef BRICK_H
#define BRICK_H

#include "RectilinearBrick.h"
#include "ConnectionPoint.h"
#include "LDRPrinter.h"
#include "Math.h"

#include <assert.h>
#include <stdint.h>
#include <iostream>

#define NUMBER_OF_POIS_FOR_BOX_INTERSECTION 10
#define NUMBER_OF_STUDS 8
// 4mm in units:
#define VERTICAL_BRICK_HALF_WIDTH 1.0
#define VERTICAL_BRICK_CENTER_TO_SIDE 0.9875
#define VERTICAL_BRICK_CENTER_TO_TOP 1.9875
#define L_VERTICAL_BRICK_CENTER_TO_SIDE_ADD (1-VERTICAL_BRICK_CENTER_TO_SIDE)
#define L_VERTICAL_BRICK_CENTER_TO_TOP_ADD (2-VERTICAL_BRICK_CENTER_TO_TOP)
#define STUD_RADIUS 0.3
#define STUD_DIAM (STUD_RADIUS+STUD_RADIUS)
// 0.0625 is 0.5 mm.
#define SNAP_DISTANCE 0.0625

/*
A Brick at any location and angle.
Default same as a Rectilinear brick: vertical at 0,0,0 (angle 0)
*/
class Brick; // Forward declaration so that << can be used in template methods.
std::ostream& operator<<(std::ostream &os, const Brick& b);

class Brick : public LDRPrinter {
public:
  Point center;
  double angle; // angle 0 = vertical.
  int8_t level;

  Brick(const Brick& b) : center(b.center), angle(b.angle), level(b.level) {}
  Brick(const RectilinearBrick& b) : center(b.x, b.y), angle(b.horizontal() ? -M_PI/2 : 0), level(b.level()) {}
  Brick(const Brick& b, const RectilinearBrick& rb);
  Brick() : center(0,0), angle(0), level(0) {}
  Brick(double cx, double cy, double a, int8_t lv) : center(cx, cy), angle(a), level(lv) {}
  Brick(const RectilinearBrick& b, const ConnectionPoint& p, const Point &origin, double originAngle, int8_t originLv);

  void toLDR(std::ofstream &os, int x, int y, int ldrColor) const;

  RectilinearBrick toRectilinearBrick() const;

  void moveBrickSoThisIsAxisAlignedAtOrigin(Brick &b) const;
  void movePointSoThisIsAxisAlignedAtOrigin(Point &p) const;

  template <int ADD_XY>
  void getBoxPOIs(Point *pois) const {
    double sina = sin(angle);
    double cosa = cos(angle);

    const double dx = VERTICAL_BRICK_CENTER_TO_SIDE + ADD_XY * L_VERTICAL_BRICK_CENTER_TO_SIDE_ADD;
    const double dy = VERTICAL_BRICK_CENTER_TO_TOP + ADD_XY * L_VERTICAL_BRICK_CENTER_TO_TOP_ADD;

    // 4 corners:
    pois[0] = Point(center.X+(-dx*cosa-dy*sina), center.Y+(-dx*sina+dy*cosa));
    pois[1] = Point(center.X+(dx*cosa-dy*sina),  center.Y+(dx*sina+dy*cosa));
    pois[2] = Point(center.X+(dx*cosa+dy*sina),  center.Y+(dx*sina-dy*cosa));
    pois[3] = Point(center.X+(-dx*cosa+dy*sina), center.Y+(-dx*sina-dy*cosa));
    // 4 sides:
    pois[4] = Point(center.X+dx*cosa, center.Y+dx*sina);
    pois[5] = Point(center.X-dx*cosa, center.Y-dx*sina);
    pois[6] = Point(center.X+dy*sina, center.Y-dy*cosa);
    pois[7] = Point(center.X-dy*sina, center.Y+dy*cosa);
    // 2 inner:
    pois[8] = Point(center.X+0.75*sina, center.Y-0.75*cosa);
    pois[9] = Point(center.X-0.75*sina, center.Y+0.75*cosa);
  }

  template <int ADD_XY, int STUD_ADD>
  void getBoxLineSegments(LineSegment *segments) const {
    double sina = sin(angle);
    double cosa = cos(angle);
    // 4 corners:
    const double dx = VERTICAL_BRICK_CENTER_TO_SIDE + ADD_XY * L_VERTICAL_BRICK_CENTER_TO_SIDE_ADD;
    const double DX = dx + STUD_ADD*STUD_RADIUS;
    const double dy = VERTICAL_BRICK_CENTER_TO_TOP + ADD_XY * L_VERTICAL_BRICK_CENTER_TO_TOP_ADD;
    const double DY = dy + STUD_ADD*STUD_RADIUS;

    segments[0].P1 = Point(center.X+(-DX*cosa-dy*sina), center.Y+(-DX*sina+dy*cosa));
    segments[0].P2 = Point(center.X+( DX*cosa-dy*sina), center.Y+( DX*sina+dy*cosa));

    segments[1].P1 = Point(center.X+( dx*cosa-DY*sina), center.Y+( dx*sina+DY*cosa));
    segments[1].P2 = Point(center.X+( dx*cosa+DY*sina), center.Y+( dx*sina-DY*cosa));

    segments[2].P1 = Point(center.X+( DX*cosa+dy*sina), center.Y+( DX*sina-dy*cosa));
    segments[2].P2 = Point(center.X+(-DX*cosa+dy*sina), center.Y+(-DX*sina-dy*cosa));

    segments[3].P1 = Point(center.X+(-dx*cosa+DY*sina), center.Y+(-dx*sina-DY*cosa));
    segments[3].P2 = Point(center.X+(-dx*cosa-DY*sina), center.Y+(-dx*sina+DY*cosa));
  }

  bool /*Brick::*/outerStudIntersectsStudAtOrigin() const;
  void /*Brick::*/getStudIntersectionsWithMovingStud(double radius, double minAngle, double maxAngle, std::vector<double> &angles) const;

  /*
    Returns the intersection between a rectangle defined by four points and a circle cutout (between minAngle and maxAngle).
   */
  template <int ADD_XY>
  IntervalList rectangleIntersectionWithCircle(Point const * const points, double radius, double minAngle, double maxAngle) const {
#ifdef _TRACE
//    std::cout << "    RECT VS Circle. Rect=" << points[0] << "; " << points[1] << "; " << points[2] << "; " << points[3] << std::endl;
#endif
    IntervalList ret;

    bool retInitiated = false;
    // Intersection with the circle must be on the right side of all four line segments:
    for(int i = 0; i < 4; ++i) {
      // Find intersections:
      double intersectionMin, intersectionMax;
      const Point &p1 = points[i];
      const Point &p2 = points[(i+1)%4];
      LineSegment segment(p1, p2);
      bool intersects = math::findCircleHalfPlaneIntersection(radius, segment, intersectionMin, intersectionMax);
      if(!intersects) {
        IntervalList empty;
        return empty;
      }
#ifdef _TRACE
//      std::cout << "     INTERECTS LINE " << segment << " IN [" << intersectionMin << ";" << intersectionMax << "]" << std::endl;
#endif
      if(!retInitiated) {
        ret = math::intervalAndRadians(minAngle, maxAngle, intersectionMin, intersectionMax);
        retInitiated = true;
      }
      else {
        IntervalList toMerge = math::intervalAndRadians(minAngle, maxAngle, intersectionMin, intersectionMax);
        ret = math::intervalAnd(ret, toMerge);
      }
    }
#ifdef _TRACE
//    std::cout << "    RECT VS Circle => " << ret << std::endl;
#endif
    return ret;
  }

  template <int ADD_XY>
  bool boxIntersectsInnerStud(Point &stud) const {
    const double cornerX = VERTICAL_BRICK_CENTER_TO_SIDE + ADD_XY * L_VERTICAL_BRICK_CENTER_TO_SIDE_ADD;
    const double cornerY = VERTICAL_BRICK_CENTER_TO_TOP + ADD_XY * L_VERTICAL_BRICK_CENTER_TO_TOP_ADD;

    // X handle four inner:
    if(stud.X < 0)
      stud.X = -stud.X;
    if(stud.Y < 0)
      stud.Y = -stud.Y;

    if(stud.X < cornerX+STUD_RADIUS && stud.Y < cornerY+STUD_RADIUS) {
      // Might intersect - check corner case:
      if(stud.X < cornerX) {
        return true;
      }
      if(stud.Y < cornerY) {
        return true;
      }
      if(STUD_RADIUS*STUD_RADIUS > (stud.X-cornerX)*(stud.X-cornerX)+(stud.Y-cornerY)*(stud.Y-cornerY)) {
        return true;
      }
    }
    return false;
  }

  /*
    Helper method for blockIntersectionWithMovingStud when radius is 0.
    If stud doesn't intersect, then return empty interval.
    Otherwise, return full interval.
   */
  template <int ADD_XY>
  IntervalList /*Brick::*/blockIntersectionWithRotatingStud(double minAngle, double maxAngle) const {
    Point p(0,0);
    movePointSoThisIsAxisAlignedAtOrigin(p);
    if(boxIntersectsInnerStud<ADD_XY>(p)) {
      return math::toIntervalsRadians(minAngle, maxAngle);
    }
    IntervalList empty;
    return empty;
  }

  /*
  Investigates the equivalent problem where a circle (center p, radius) intersects the Minkowski sum of brick and a stud.
  Intersects block on either a line segment or a quarter-circle at one of the corners. This problem can be split into two intersections against rectangles and four against circles.
  Returns intervals of intersections between minAngle and maxAngle.
  */
  template <int ADD_XY>
  IntervalList blockIntersectionWithMovingStud(double radius, double minAngle, double maxAngle) const {
#ifdef _TRACE
    std::cout << "  BLOCK vs MS " << *this << ", r=" << radius << ", [" << minAngle << ";" << maxAngle << "]" << std::endl;
    LineSegment sx[4];
    getBoxLineSegments<ADD_XY,0>(sx); // ",1" ensures line segments are moved one stud radius out.    
    std::cout << "   BASE BLOCK: " << sx[0] << ", " << sx[2] << std::endl;
#endif

    assert(radius < EPSILON || radius > STUD_RADIUS);
    if(radius < EPSILON) {
#ifdef _TRACE
      std::cout << "  R=0, SO FIND QUICK!" << std::endl;
#endif
      return blockIntersectionWithRotatingStud<ADD_XY>(minAngle, maxAngle);
    }
    // First check line segments: The intersection with the moving stud must be on the right side of ALL four segments:
    LineSegment segments[4];
    getBoxLineSegments<ADD_XY,1>(segments); // ",1" ensures line segments are moved one stud radius out.    
    Point p1[4] = {segments[0].P1, segments[0].P2, segments[2].P1, segments[2].P2};
    IntervalList ret = rectangleIntersectionWithCircle<ADD_XY>(p1, radius, minAngle, maxAngle);
#ifdef _TRACE
    std::cout << "   WIDE Box intersect: " << ret << std::endl;
#endif

    Point p2[4] = {segments[1].P1, segments[1].P2, segments[3].P1, segments[3].P2};
    IntervalList intersectionsWithRect2 = rectangleIntersectionWithCircle<ADD_XY>(p2, radius, minAngle, maxAngle);
    ret = math::intervalOr(ret, intersectionsWithRect2);
#ifdef _TRACE
    std::cout << "   TALL Box intersect: " << intersectionsWithRect2 << " => " << ret << std::endl;
#endif

    // Now check quarter circles: 
    //  Subtract the intersection intervals that intersect the corner circles on the left of the corner dividers:
    Point pois[NUMBER_OF_POIS_FOR_BOX_INTERSECTION];
    getBoxPOIs<ADD_XY>(pois);
    for(int i = 0; i < 4; ++i) {
      // Find intersections:
      IntervalList intervalFromCircle = math::findCircleCircleIntersection(radius, pois[i], STUD_RADIUS);
      for(IntervalList::const_iterator it = intervalFromCircle.begin(); it != intervalFromCircle.end(); ++it) {
        IntervalList intervalFromCircleInCorrectInterval = math::intervalAndRadians(minAngle, maxAngle, it->first, it->second);
        ret = math::intervalOr(ret, intervalFromCircleInCorrectInterval);
#ifdef _TRACE
        std::cout << "   circle<" << ADD_XY << "> " << i << " (" << pois[i] << ") intersect: " << *it << "=>" << intervalFromCircleInCorrectInterval << " => " << ret << std::endl;
#endif
      }
    }

    return ret;
  }

  Point getStudPosition(ConnectionPointType type) const;
  void getStudPositions(Point *positions) const;

  template <int ADD_XY>
  bool boxIntersectsPOIsFrom(Brick &b) const {
    const double dx = VERTICAL_BRICK_CENTER_TO_SIDE + ADD_XY * L_VERTICAL_BRICK_CENTER_TO_SIDE_ADD;
    const double dy = VERTICAL_BRICK_CENTER_TO_TOP + ADD_XY * L_VERTICAL_BRICK_CENTER_TO_TOP_ADD;
    moveBrickSoThisIsAxisAlignedAtOrigin(b);
    // Get POIs:
    Point pois[NUMBER_OF_POIS_FOR_BOX_INTERSECTION];
    b.getBoxPOIs<ADD_XY>(pois);
    // Check each POI:
    for(int i = 0; i < NUMBER_OF_POIS_FOR_BOX_INTERSECTION; ++i) {
      Point &poi = pois[i];
      if(poi.X < 0)
        poi.X = -poi.X;
      if(poi.Y < 0)
        poi.Y = -poi.Y;
      if(poi.X + EPSILON < dx && poi.Y + EPSILON < dy) {
        return true;
      }
    }
    return false;
  }

  /*
  Assumes b is on same level.
  */
  template <int ADD_XY>
  bool boxesIntersect(const Brick &b) const {
    Brick tmpB(b);
    Brick tmpThis(*this);
    return boxIntersectsPOIsFrom<ADD_XY>(tmpB) || b.boxIntersectsPOIsFrom<ADD_XY>(tmpThis);
  }

  template <int ADD_XY>
  bool boxIntersectsOuterStud(const Point &studOfB, bool &connected, ConnectionPoint &foundConnectionThis, ConnectionPoint &foundConnectionB, const RectilinearBrick &source, const RectilinearBrick &bSource, int i) const {
    const double cornerX = VERTICAL_BRICK_CENTER_TO_SIDE + ADD_XY * L_VERTICAL_BRICK_CENTER_TO_SIDE_ADD;
    const double cornerY = VERTICAL_BRICK_CENTER_TO_TOP + ADD_XY * L_VERTICAL_BRICK_CENTER_TO_TOP_ADD;

    Point stud(studOfB);
    if(stud.X < 0)
      stud.X = -stud.X;
    if(stud.Y < 0)
      stud.Y = -stud.Y;

    if(stud.X < cornerX+STUD_RADIUS && stud.Y < cornerY+STUD_RADIUS) {
      // X check if it hits stud:
      const double studX = HALF_STUD_DISTANCE;
      const double studY = STUD_AND_A_HALF_DISTANCE;   
      if(SNAP_DISTANCE*SNAP_DISTANCE >= (stud.X-studX)*(stud.X-studX)+(stud.Y-studY)*(stud.Y-studY)) {
        // We are already connected:
        if(connected) {
          connected = false;
          return true;
        }
        // Compute corner:
        connected = true;
        if(studOfB.X < 0 && studOfB.Y < 0)
          foundConnectionThis = ConnectionPoint(SW, source, false, -1);
        else if(studOfB.X < 0 && studOfB.Y > 0)
          foundConnectionThis = ConnectionPoint(NW, source, false, -1);
        else if(studOfB.X > 0 && studOfB.Y < 0)
          foundConnectionThis = ConnectionPoint(SE, source, false, -1);
        else // if(stud.X > 0 && stud.Y > 0)
          foundConnectionThis = ConnectionPoint(NE, source, false, -1);
        foundConnectionB = ConnectionPoint((ConnectionPointType)(i-4), bSource, true, -1);
        return false;
      }

      // Might intersect - check corner case:
      if(stud.X < cornerX ||
        stud.Y < cornerY ||
        STUD_RADIUS*STUD_RADIUS > (stud.X-cornerX)*(stud.X-cornerX)+(stud.Y-cornerY)*(stud.Y-cornerY)) {
          connected = false;
          return true;
      }
    }
    return false;
  }

  template <int ADD_XY>
  bool boxIntersectsStudsFrom(Brick &b, const RectilinearBrick &bSource, bool &connected, ConnectionPoint &foundConnectionB, ConnectionPoint &foundConnectionThis, const RectilinearBrick &source) const {
    moveBrickSoThisIsAxisAlignedAtOrigin(b);
    Point studsOfB[NUMBER_OF_STUDS];
    b.getStudPositions(studsOfB);

    // X handle four inner:
    for(int i = 0; i < 4; ++i) {
      Point &stud = studsOfB[i];
      if(boxIntersectsInnerStud<ADD_XY>(stud)) {
        connected = false;
        return true;
      }
    }
    // Handle four outer specially as they might cause connection:
    for(int i = 4; i < NUMBER_OF_STUDS; ++i) {
      Point stud = studsOfB[i];
      if(boxIntersectsOuterStud<ADD_XY>(stud, connected, foundConnectionThis, foundConnectionB, source, bSource, i))
        return true;
    }
    return connected;
  }

  /*
  Return true if this brick intersects the other. 
  If the two bricks are corner connected, then connected is set to true and the found connection is set.
  1: If not even close level-wise: return false.
  2: If on same level: Check that boxes do not collide.
  - If current implementation doesn't work, use algorithm from http://www.ragestorm.net/tutorial?id=22
  3: If on adjacent levels:
  - If any of the 8 studs of lower brick inside box of upper: intersect.
  - If intersect: Check if only intersecting one corner stud.
  - If only one corner stud, check if corner connected.
  */
  template <int ADD_XY>
  bool intersects(const Brick &b, const RectilinearBrick &bSource, bool &connected, ConnectionPoint &foundConnectionB, ConnectionPoint &foundConnectionThis, const RectilinearBrick &source) const {
    connected = false;
    if(level > b.level+1 || b.level > level+1)
      return false;
    if(level == b.level) {
      bool ret = boxesIntersect<ADD_XY>(b);
      return ret;
    }
    Brick tmpB(b);
    Brick tmpThis(*this);
    if(level < b.level)
      return b.boxIntersectsStudsFrom<ADD_XY>(tmpThis, source, connected, foundConnectionThis, foundConnectionB, bSource);
    return boxIntersectsStudsFrom<ADD_XY>(tmpB, bSource, connected, foundConnectionB, foundConnectionThis, source);
  }

  bool operator < (const Brick &b) const;
};

#endif // BRICK_H
