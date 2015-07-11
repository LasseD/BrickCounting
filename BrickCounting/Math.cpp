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
  double distSq(const Point &p1, const Point &p2) {
    double dx = p2.X-p1.X;
    double dy = p2.Y-p1.Y;
    return dx*dx+dy*dy;
  }
  double dist(const Point &p1, const Point &p2) {
    return sqrt(distSq(p1,p2));
  }
  double normalizeAngle(double a) {
    while(a < -M_PI)
        a += 2*M_PI;
    while(a >= M_PI)
        a -= 2*M_PI;
    return a;
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

  /*
    Assumes the right side of the line is the half plane in which the intersection is to be reported.
    The intersection is saved as an interval in [intersectionMin;intersectionMax]. 
    If intersectionMin > intersectionMax, then the intersection passes -M_PI/M_PI and consists of two intervals:
      [intersectionMin;M_PI] and [-M_PI;intersectionMax]
    Returns true if there is an intersection. 
   */
  bool findCircleHalfPlaneIntersection(double radius, const LineSegment &line, double &intersectionMin, double &intersectionMax) {
    Point i1, i2;
    int newIntersections = math::findCircleLineIntersections(radius, line, i1, i2);      
    if(newIntersections != 2)
      return false; // Ignore no intersection and intersection in a point.

    intersectionMin = math::angleOfPoint(i1);
    intersectionMax = math::angleOfPoint(i2);
    if(intersectionMin > intersectionMax)
      std::swap(intersectionMin, intersectionMax);

    // Find mid-point and determine interval inside of half plane:
    double midAngle = (intersectionMin + intersectionMax)/2;
    if(math::rightTurn(line.P1, line.P2, Point(radius*cos(midAngle), radius*sin(midAngle)))) {
      // OK: Mid-point of interval (on side without jump) is inside the half plane.
    }
    else {
      // Swap the intersections to indicate that the side with the jump is inside the half plane.
      std::swap(intersectionMin, intersectionMax);
    }

    return true; // Interval found.
  }

  // Returns a<=b<=c or c<=b<=a
  bool between(double a, double b, double c) {
    return (a <= b && b <= c) || (a >= b && b >= c);
  }
  bool angleBetween(double minAngle, double a, double maxAngle) {
    assert(a >= -M_PI);
    assert(a < M_PI);
    assert(minAngle >= -M_PI);
    assert(minAngle < M_PI);
    assert(maxAngle >= -M_PI);
    assert(maxAngle < M_PI);
    if(minAngle > maxAngle) {
      return (-M_PI <= a && a <= maxAngle) || (minAngle <= a && a <= M_PI);
    }
    return minAngle <= a && a <= maxAngle;
  }
  bool between(const Point &a, const Point &b, const Point &c) {
    return between(a.X, b.X, c.X) && between(a.Y, b.Y, c.Y);
  }
  double angleOfPoint(const Point &p) {
    return atan2(p.Y,p.X);
  }

  bool rightTurn(const Point &lineStart, const Point &lineEnd, const Point &p) {
    return (lineEnd.X-lineStart.X)*(p.Y-lineStart.Y) - (lineEnd.Y-lineStart.Y)*(p.X-lineStart.X) < 0;
  }

  // http://stackoverflow.com/questions/3349125/circle-circle-intersection-points
  int findCircleCircleIntersections(double r, const Point &p, double pr, Point &i1, Point &i2) {
    double distCentres = norm(p);
    if(distCentres > r + pr)
      return 0; // No solution.
    double x1 = p.X;
    double y1 = p.Y;
    double a = (r*r-pr*pr+distCentres*distCentres)/2*distCentres;
    double h = sqrt(r*r-a*a);
    double x2 = a*x1/distCentres;
    double y2 = a*y1/distCentres;

    i1.X = x2 + h*y1/distCentres;
    i1.Y = y2 + h*x1/distCentres;
    i2.X = x2 - h*y1/distCentres;
    i2.Y = y2 - h*x1/distCentres;

    return 2;
  }

  /*
    Finds the (angle) intervals of the circle at O with radius r where it intersects with the circle at p and radius pr.
   */
  IntervalList findCircleCircleIntersection(double r, const Point &p, double pr) {
    assert(r > pr);
    IntervalList ret;
    Point i1, i2;
    int newIntersections = findCircleCircleIntersections(r, p, pr, i1, i2);
    if(newIntersections < 2)
      return ret;

    // Find the angle interval. Since the first assertion holds, the interval of intersection is less than PI in length:
    double ai1 = math::angleOfPoint(i1);
    double ai2 = math::angleOfPoint(i2);
    if(ai1 > ai2)
      std::swap(ai1, ai2);

    if(ai2-ai1 < M_PI) {
      ret.push_back(Interval(ai1, ai2));
    }
    else {
      ret.push_back(Interval(-M_PI, ai1));
      ret.push_back(Interval(ai2, M_PI));
    }
    return ret;
  }

  /*
    Handles splitting of a and b if they jump.
   */
  IntervalList intervalAnd(double a1, double a2, double b1, double b2) {
    bool aJumps = a1 > a2;
    bool bJumps = b1 > b2;
    IntervalList ret;
    if(aJumps) {
      if(bJumps) {
        // a consists of [a1;M_PI] and [-M_PI;a2],
        // b consists of [b1;M_PI] and [-M_PI;b2]:
        ret.push_back(Interval(-M_PI, MIN(a2, b2)));
        ret.push_back(Interval(MAX(a1, b1), M_PI));
      }
      else {
        // a consists of [a1;M_PI] and [-M_PI;a2]:
        if(b2 > a1)
          ret.push_back(Interval(a1, b2));
        if(b1 < a2)
          ret.push_back(Interval(a2, b1));
      }
    }
    else {
      if(bJumps) {
        // b consists of [b1;M_PI] and [-M_PI;b2]:
        if(a2 > b1)
          ret.push_back(Interval(b1, a2));
        if(a1 < b2)
          ret.push_back(Interval(b2, a1));
      }
      else { // Normal case: No jumping anywhere:
        if(a2 > b1 && a1 < b2)
          ret.push_back(Interval(MAX(a1, b1), MIN(a2, b2)));
      }
    }
    return ret;
  }

  IntervalList intervalAnd(const IntervalList &a, const IntervalList &b) {
    IntervalList ret;
    IntervalList::const_iterator itA = a.begin();
    IntervalList::const_iterator itB = b.begin();
    while(itA != a.end() && itB != b.end()) {
      // B ends before A:
      if(itB->second < itA->first) {
        ++itB;
        continue;
      }
      // A ends before B:
      if(itA->second < itB->first) {
        ++itA;
        continue;
      }
      // Both A and B end after they both have started:
      const double min = MAX(itA->first, itB->first);
      if(itA->second < itB->second) {
        ret.push_back(Interval(min,itA->second));      
        ++itA;
      }
      else {
        ret.push_back(Interval(min,itB->second));      
        ++itB;
      }
    }
    return ret;    
  }

  IntervalList intervalOr(const IntervalList &a, const IntervalList &b) {
    IntervalList ret;
    IntervalList::const_iterator itA = a.begin();
    IntervalList::const_iterator itB = b.begin();
    while(itA != a.end() && itB != b.end()) {
      // B ends before A:
      if(itB->second < itA->first) {
        ret.push_back(*itB);
        ++itB;
        continue;
      }
      // A ends before B:
      if(itA->second < itB->first) {
        ret.push_back(*itA);
        ++itA;
        continue;
      }
      const double min = MIN(itA->first, itB->first);
      double max;
      while(true) {
        max = MAX(itA->second, itB->second);
        if(itA->second < max) {
          ++itA;
          if(itA == a.end() || itA->first > max) {
            ++itB;
            break;
          }
        }
        else {
          ++itB;
          if(itB == b.end() || itB->first > max) {
            ++itA;
            break;
          }
        }
      }
      ret.push_back(Interval(min,max));      
    }
    // Clean up rest:
    while(itA != a.end()) {
      ret.push_back(*itA);
      ++itA;
    }
    while(itB != b.end()) {
      ret.push_back(*itB);
      ++itB;
    }
    return ret;
  }

  IntervalList intervalInverse(const IntervalList &l, const Interval fullInterval) {
    IntervalList ret;
    if(l.empty()) {
      ret.push_back(fullInterval);
      return ret;
    }
    double last = l[0].first;
    for(IntervalList::const_iterator it = l.begin(); it != l.end(); ++it) {
      if(last != it->first)
        ret.push_back(Interval(last, it->first));
      last = it->second;
    }
    if(last != fullInterval.second)
      ret.push_back(Interval(last, fullInterval.second));
    return ret;
  }

  IntervalList collapseIntervals(const IntervalList &l) {
    IntervalList ret;
    if(l.empty()) {
      return ret;
    }

    IntervalList::const_iterator it = l.begin();
    Interval prev = *it;
    ++it;
    for(; it != l.end(); ++it) {
      if(prev.second != it->first) {
        ret.push_back(prev);
        prev = *it;
      }
      else
        prev.second = it->second;
    }
    ret.push_back(prev);

    return ret;
  }

  void intervalToArray(const Interval &fullInterval, const IntervalList &l, bool *array, unsigned int sizeArray) {
    const double min = fullInterval.first;
    const double max = fullInterval.second;
    IntervalList::const_iterator it = l.begin();
    for(unsigned int i = 0; i < sizeArray; ++i) {
      double v = min + ((max-min)*i)/(sizeArray-1);
      if(it == l.end() || v < it->first) {
        array[i] = false;
      }
      else if(v > it->second) {
        array[i] = false;
        ++it;
      }
      else
        array[i] = true;
    }
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

std::ostream& operator<<(std::ostream &os, const IntervalList& l) {
  os << "{";
  for(IntervalList::const_iterator it = l.begin(); it != l.end(); ++it) {
    os << "[" << *it << "]";
  }
  os << "}";
  return os;
}

