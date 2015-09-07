#ifndef MATH_H
#define MATH_H

#include <utility>
#include <iostream>
#include <vector>
#define _USE_MATH_DEFINES
#include <math.h>
#include <assert.h>
#include <stdint.h>

//#define _TRACE 1
//#define _COMPARE_ALGORITHMS 1

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
typedef Interval RadianInterval; // Counter clockwise from P1 to P2.
typedef std::vector<Interval> IntervalList;
std::ostream& operator<<(std::ostream &os, const IntervalList& p);

typedef std::pair<Point,Point> LineSegment;
std::ostream& operator<<(std::ostream &os, const LineSegment& l);

namespace math {
  bool eqEpsilon(double a, double b);
  bool eqEpsilon(const Point &p1, const Point &p2);
  double round(double number);
  int signum(double d);
  double abs(double d);
  double normSq(const Point &p);
  double norm(const Point &p);
  double distSq(const Point &p1, const Point &p2);
  double dist(const Point &p1, const Point &p2);
  bool between(double a, double b, double c);
  bool betweenEpsilon(double a, double b, double c);
  bool inRadianInterval(double a, const RadianInterval &interval);
  bool betweenEndPointsOfLineSegmentEpsilon(const Point &a, const Point &b, const Point &c);
  double angleOfPoint(const Point &p);
  /*
    Normalize an angle to the interval [-PI;PI[ in radians.
   */
  double normalizeAngle(double a);

  bool intervalEquals(const IntervalList &a, const IntervalList &b);
  bool intervalContains(const IntervalList &a, double d);
  bool isFullInterval(const IntervalList &a, double min, double max);
  IntervalList intervalAnd(const IntervalList &a, const IntervalList &b);
  IntervalList intervalAndRadians(const RadianInterval &a, const RadianInterval &b);
  IntervalList intervalInverseRadians(const IntervalList &l, const RadianInterval &minmax);
  IntervalList intervalOr(const IntervalList &a, const IntervalList &b);
  IntervalList intervalReverse(const IntervalList &l);
  IntervalList collapseIntervals(const IntervalList &l);
  IntervalList toIntervalsRadians(const RadianInterval &interval);

  bool rightTurn(const Point &lineStart, const Point &lineEnd, const Point &p);

  int findCircleCircleIntersections(const double r, const Point &p, const double pr, Point &i1, Point &i2); // Actual primitive
  bool findCircleCircleIntersections(const double r, const Point &p, const double pr, double &ai1, double &ai2); // Actual primitive
  IntervalList findCircleCircleIntersection(double r, const Point &p, double pr); // Returns intersections as IntervalList

  int findCircleLineIntersections(double r, const LineSegment &l, Point &i1, Point &i2);
  bool findCircleLineIntersections(double r, const LineSegment &l, double &ai1, double &ai2);
  bool circleCutoutIntersectsLineSegment(double r, const RadianInterval &circleInterval, const LineSegment &l);

  /*
    The half plane is divided by the line 
   */
  bool findCircleHalfPlaneIntersection(double radius, const LineSegment &line, RadianInterval &intersection);

  typedef std::pair<unsigned long, unsigned short> IntervalIndicator; // location, size

  struct IntervalListVector {
  private:
    Interval *intervals;
    IntervalIndicator *indicators;
    const uint32_t intervalsSize, indicatorSize;
    uint32_t intervalsI;
    IntervalListVector(); // Unused
    IntervalListVector& operator=(const IntervalListVector &); // Unused
  public:
    IntervalListVector(uint32_t indicatorSize, unsigned int maxLoadFactor);
    ~IntervalListVector();
    void insert(uint32_t location, const IntervalList &intervalList);
    void insertEmpty(uint32_t location);
    void get(uint32_t location, IntervalList &intervalList) const;
    Interval get(uint32_t location, unsigned short intervalIndex) const;
    uint32_t sizeIndicator() const;
    uint32_t sizeNonEmptyIntervals() const;
    unsigned short intervalSizeForIndicator(uint32_t i) const;
    void validateAllIntervalsSet() const;
  };
}

#endif // MATH_H
