#ifndef MATH_H
#define MATH_H

#include <utility>
#include <iostream>
#define _USE_MATH_DEFINES
#include <math.h>

// Ensure cross platform compatibility of std::min:
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define EPSILON 1e-6

#define X first
#define Y second
#define P1 first
#define P2 second

typedef std::pair<double,double> Point;
std::ostream& operator<<(std::ostream &os, const Point& p);

typedef std::pair<Point,Point> LineSegment;
std::ostream& operator<<(std::ostream &os, const LineSegment& l);

namespace math {
  double round(double number);
  int signum(double d);
  double abs(double d);
  double normSq(const Point &p);
  double norm(const Point &p);
  int findCircleLineIntersections(double r, const LineSegment &l, Point &i1, Point &i2);
  bool between(double a, double b, double c);
  bool angleBetween(double minAngle, double a, double maxAngle);
  bool between(const Point &a, const Point &b, const Point &c);
  double angleOfPoint(const Point &p);
}


#endif // MATH_H
