#include "Math.h"

#include <cmath>

///////////////////////////////////////////////////////
// Simple geometry functions
///////////////////////////////////////////////////////
namespace math {
  double round(double number) {
    return number < 0.0 ? ceil(number - 0.5) : floor(number + 0.5);
  }
  int signum(double d) {
    if(d < 0)
      return -1;
    return 1;
  }
  double abs(double d) {
    if(d < 0)
      return -d;
    return d;
  }
  double normSq(const Point &p) {
    return p.X*p.X + p.Y*p.Y;
  }
  double norm(const Point &p) {
    return sqrt(normSq(p));
  }

  // From http://mathworld.wolfram.com/Circle-LineIntersection.html
  int findCircleLineIntersections(double r, const LineSegment &l, Point &i1, Point &i2) {
    const double dx = l.P2.X - l.P1.X;
    const double dy = l.P2.Y - l.P1.Y;
    const double drSq = dx*dx + dy*dy;
    const double D = l.P1.X*l.P2.Y - l.P2.X*l.P1.Y;
    const double discriminant = r*r*drSq - D*D;

    if(discriminant < 0)
      return 0;

    const double rootDiscriminant = sqrt(discriminant);

    i1.X = (D*dy + signum(dy)*dx*rootDiscriminant)/drSq;
    i1.Y = (D*dx + abs(dy)*rootDiscriminant)/drSq;
    i2.X = (D*dy - signum(dy)*dx*rootDiscriminant)/drSq;
    i2.Y = (D*dx - abs(dy)*rootDiscriminant)/drSq;
    return discriminant > 0 ? 2 : 1;
  }

  // Returns a<=b<=c or c<=b<=a
  bool between(double a, double b, double c) {
    return (a <= b && b <= c) || (a >= b && b >= c);
  }
  bool angleBetween(double minAngle, double a, double maxAngle) {
    if(maxAngle - minAngle > M_PI) {
      return (0 <= a && a <= minAngle) || (maxAngle <= a && a <= 2*M_PI);
    }
    else {
      return minAngle <= a && a <= maxAngle;
    }
  }
  bool between(const Point &a, const Point &b, const Point &c) {
    return between(a.X, b.X, c.X) && between(a.Y, b.Y, c.Y);
  }
  double angleOfPoint(const Point &p) {
    double ret = atan2(p.Y,p.X);
    return ret < 0 ? ret + 2*M_PI : ret;
  }
}

std::ostream& operator<<(std::ostream &os, const Point& p) {
  os << p.X << "," << p.Y;
  return os;
}

std::ostream& operator<<(std::ostream &os, const LineSegment& l) {
  os << l.P1 << "->" << l.P2;
  return os;
}
