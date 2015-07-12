#ifndef MATH_H
#define MATH_H

#include <utility>
#include <iostream>
#include <vector>
#define _USE_MATH_DEFINES
#include <math.h>
#include <assert.h>

// Ensure cross platform compatibility of std::min:
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((b) < (a) ? (a) : (b))
#define EPSILON 1e-6

#define X first
#define Y second
#define P1 first
#define P2 second

typedef std::pair<double,double> Point;
std::ostream& operator<<(std::ostream &os, const Point& p);

typedef std::pair<double,double> Interval;
typedef std::vector<Interval> IntervalList;
std::ostream& operator<<(std::ostream &os, const IntervalList& p);

typedef std::pair<Point,Point> LineSegment;
std::ostream& operator<<(std::ostream &os, const LineSegment& l);

namespace math {
  double round(double number);
  int signum(double d);
  double abs(double d);
  double normSq(const Point &p);
  double norm(const Point &p);
  double distSq(const Point &p1, const Point &p2);
  double dist(const Point &p1, const Point &p2);
  int findCircleLineIntersections(double r, const LineSegment &l, Point &i1, Point &i2);
  bool between(double a, double b, double c);
  bool angleBetween(double minAngle, double a, double maxAngle);
  bool between(const Point &a, const Point &b, const Point &c);
  double angleOfPoint(const Point &p);
  /*
    Normalize an angle to the interval [-PI;PI[ in radians.
   */
  double normalizeAngle(double a);

  IntervalList intervalAnd(const IntervalList &a, const IntervalList &b);
  IntervalList intervalAnd(double a1, double a2, double b1, double b2);
  IntervalList intervalInverse(const IntervalList &l, const Interval fullInterval);
  IntervalList intervalOr(const IntervalList &a, const IntervalList &b);
  IntervalList collapseIntervals(const IntervalList &l);
  void intervalToArray(const Interval &fullInterval, const IntervalList &l, bool *array, unsigned int sizeArray);

  bool rightTurn(const Point &lineStart, const Point &lineEnd, const Point &p);

  int findCircleCircleIntersections(double r, const Point &p, double pr, Point &i1, Point &i2); // Actual primitive
  IntervalList findCircleCircleIntersection(double r, const Point &p, double pr); // Returns intersections as IntervalList

  /*
    The half plane is divided by the line 
   */
  bool findCircleHalfPlaneIntersection(double radius, const LineSegment &line, double &intersectionMin, double &intersectionMax);
}


#endif // MATH_H