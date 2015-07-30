#ifndef MATH_H
#define MATH_H

#include <utility>
#include <iostream>
#include <vector>
#define _USE_MATH_DEFINES
#include <math.h>
#include <assert.h>

//#define _TRACE 1

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
  bool eqEpsilon(double a, double b);
  double round(double number);
  int signum(double d);
  double abs(double d);
  double normSq(const Point &p);
  double norm(const Point &p);
  double distSq(const Point &p1, const Point &p2);
  double dist(const Point &p1, const Point &p2);
  bool between(double a, double b, double c);
  bool angleBetween(double minAngle, double a, double maxAngle);
  bool between(const Point &a, const Point &b, const Point &c);
  double angleOfPoint(const Point &p);
  /*
    Normalize an angle to the interval [-PI;PI[ in radians.
   */
  double normalizeAngle(double a);

  bool intervalEquals(const IntervalList &a, const IntervalList &b);
  IntervalList intervalAnd(const IntervalList &a, const IntervalList &b);
  IntervalList intervalAndRadians(double a1, double a2, double b1, double b2);
  IntervalList intervalInverseRadians(const IntervalList &l, double min, double max);
  IntervalList intervalOr(const IntervalList &a, const IntervalList &b);
  IntervalList intervalReverse(const IntervalList &l);
  IntervalList collapseIntervals(const IntervalList &l);
  void intervalToArray(const Interval &fullInterval, const IntervalList &l, bool *array, unsigned int sizeArray);
  IntervalList toIntervalsRadians(double min, double max);

  bool rightTurn(const Point &lineStart, const Point &lineEnd, const Point &p);

  int findCircleCircleIntersections(const double r, const Point &p, const double pr, Point &i1, Point &i2); // Actual primitive
  bool findCircleCircleIntersections(const double r, const Point &p, const double pr, double &ai1, double &ai2); // Actual primitive
  IntervalList findCircleCircleIntersection(double r, const Point &p, double pr); // Returns intersections as IntervalList

  int findCircleLineIntersections(double r, const LineSegment &l, Point &i1, Point &i2);
  bool findCircleLineIntersections(double r, const LineSegment &l, double &ai1, double &ai2);
  /*
    The half plane is divided by the line 
   */
  bool findCircleHalfPlaneIntersection(double radius, const LineSegment &line, double &intersectionMin, double &intersectionMax);
}

#endif // MATH_H
