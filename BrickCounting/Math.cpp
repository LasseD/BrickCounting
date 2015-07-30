#include "Math.h"

#include <cmath>

///////////////////////////////////////////////////////
// Simple geometry functions
///////////////////////////////////////////////////////
namespace math {
  bool eqEpsilon(double a, double b) {
    return a >= b-EPSILON && a <= b+EPSILON;
  }
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

  // Using http://mathworld.wolfram.com/Point-LineDistance2-Dimensional.html
  bool findCircleLineIntersections(double r, const LineSegment &l, double &ai1, double &ai2) {
    // First compute distance between circle center and line:
    Point p1(l.P1);
    Point p2(l.P2);
    if(rightTurn(p1, p2, Point(0,0)))
      std::swap(p1, p2);
    const double &x1 = p1.X;
    const double &y1 = p1.Y;
    const double &x2 = p2.X;
    const double &y2 = p2.Y;
    const double distNominator = abs((x2-x1)*(y1) - (x1)*(y2-y1));
    const double distDenominator = sqrt((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));
    const double dist = distNominator/distDenominator;
    if(dist >= r)
      return false;
    double angleOfV = atan2(-x2+x1, y2-y1);
    double angleDiff = acos(dist/r);
    ai1 = angleOfV-angleDiff;
    if(ai1 < -M_PI)
      ai1+=2*M_PI;
    ai2 = angleOfV+angleDiff;
    if(ai2 > M_PI)
      ai2-=2*M_PI;
    return true;
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
    i1.Y = (-D*dx + abs(dy)*rootDiscriminant)/drSq;
    i2.X = (D*dy - signum(dy)*dx*rootDiscriminant)/drSq;
    i2.Y = (-D*dx - abs(dy)*rootDiscriminant)/drSq;
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
#ifdef _DEBUG
    Point i1, i2;
    int ni = math::findCircleLineIntersections(radius, line, i1, i2);
#endif
    bool newIntersections = math::findCircleLineIntersections(radius, line, intersectionMin, intersectionMax);      
    if(!newIntersections) {// != 2) {
#ifdef _DEBUG
      assert(ni != 2);
#endif
      if(math::rightTurn(line.P1, line.P2, Point(0,0))) { // Inside half plane
        intersectionMin = -M_PI;
        intersectionMax = M_PI;
        return true;
      }
      return false; // Ignore no intersection and intersection in a point.
    }

#ifdef _DEBUG
    double im1 = math::angleOfPoint(i1);
    double im2 = math::angleOfPoint(i2);
    if(!(eqEpsilon(im1, intersectionMin) || eqEpsilon(im1, intersectionMax))) {
      std::cout << "im1=" << im1 << " != intersectionMin=" << intersectionMin << ", intersectionMax=" << intersectionMax << std::endl;
      assert(false);
    }
    if(!(eqEpsilon(im2, intersectionMin) || eqEpsilon(im2, intersectionMax))) {
      std::cout << "im2=" << im2 << " != intersectionMin=" << intersectionMin << ", intersectionMax=" << intersectionMax << std::endl;
      assert(false);
    }
#endif
    //intersectionMin = math::angleOfPoint(i1);
    //intersectionMax = math::angleOfPoint(i2);
    if(intersectionMin > intersectionMax)
      std::swap(intersectionMin, intersectionMax);

    // Find mid-point and determine interval inside of half plane:
    double midAngle = (intersectionMin + intersectionMax)/2;
    Point midPoint(radius*cos(midAngle), radius*sin(midAngle));
    if(math::rightTurn(line.P1, line.P2, midPoint)) {
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
    assert(a >= -M_PI - EPSILON);
    assert(a <= M_PI + EPSILON);
    assert(minAngle >= -M_PI - EPSILON);
    assert(minAngle < M_PI + EPSILON);
    assert(maxAngle >= -M_PI - EPSILON);
    assert(maxAngle < M_PI + EPSILON);
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
  // Discontinued due to poor performance.
  int findCircleCircleIntersections(const double r, const Point &p, const double pr, Point &i1, Point &i2) {
    const double distCentresSq = normSq(p);
    const double distCentres = sqrt(distCentresSq);
    assert(distCentresSq > EPSILON);
    assert(distCentres > EPSILON);
    if(distCentres > r + pr || distCentres+pr <= r || distCentres+r <= pr)
      return 0; // No solution.

    const double x1 = p.X;
    const double y1 = p.Y;
    const double a = (r*r-pr*pr+distCentresSq)/(2*distCentres);
    const double h = sqrt(abs(r*r-a*a));
    const double x2 = a*x1/distCentres;
    const double y2 = a*y1/distCentres;

    i1.X = x2 + h*y1/distCentres;
    i1.Y = y2 - h*x1/distCentres;
    i2.X = x2 - h*y1/distCentres;
    i2.Y = y2 + h*x1/distCentres;

    return 2;
  }

  // Basic trigonometry...
  bool findCircleCircleIntersections(const double r, const Point &p, const double pr, double &ai1, double &ai2) {
    const double distCentresSq = normSq(p);
    const double distCentres = sqrt(distCentresSq);
    if(distCentres > r + pr || distCentres+pr <= r || distCentres+r <= pr)
      return false; // No solution.
    if(pr > distCentres) { // Special case: pr encloses origin:
      ai1 = -M_PI;
      ai2 = M_PI;
      return true;
    }

    const double angleP = angleOfPoint(p);
    const double angleDiff = acos((pr*pr-distCentresSq-r*r)/(-2*r*distCentres));
    assert(angleDiff >= 0);
    ai1 = angleP - angleDiff;
    while(ai1 < -M_PI)
      ai1 += 2*M_PI;
    ai2 = angleP + angleDiff;
    while(ai2 > M_PI)
      ai2 -= 2*M_PI;

    return true;
  }

  /*
    Finds the (angle) intervals of the circle at O with radius r where it intersects with the circle at p and radius pr.
   */
  IntervalList findCircleCircleIntersection(double r, const Point &p, double pr) {
    double ai1, ai2;
    bool newIntersections = findCircleCircleIntersections(r, p, pr, ai1, ai2);
    IntervalList ret;
    if(!newIntersections)// < 2)
      return ret;

    // Find the angle interval. Since the first assertion holds, the interval of intersection is less than PI in length:
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
  IntervalList intervalAndRadians(double a1, double a2, double b1, double b2) {
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
        // a consists of [-M_PI;a2] and [a1;M_PI]:
        if(b1 < a2)
          ret.push_back(Interval(b1, MIN(b2,a2)));
        if(b2 > a1)
          ret.push_back(Interval(MAX(b1,a1), b2));
      }
    }
    else {
      if(bJumps) {
        // b consists of [b1;M_PI] and [-M_PI;b2]:
        if(a1 < b2)
          ret.push_back(Interval(a1, MIN(a2, b2)));
        if(a2 > b1)
          ret.push_back(Interval(MAX(a1,b1), a2));
      }
      else { // Normal case: No jumping anywhere:
        if(a2 > b1 && a1 < b2)
          ret.push_back(Interval(MAX(a1, b1), MIN(a2, b2)));
      }
    }
    return ret;
  }

  bool intervalEquals(const IntervalList &a, const IntervalList &b) {
    if(a.size() != b.size()) {
      return false;
    }
    IntervalList::const_iterator itA = a.begin();
    IntervalList::const_iterator itB = b.begin();
    for(;itA != a.end(); ++itA, ++itB) {
      if(*itA != *itB)
        return false;
    }
    return true;
  }

  IntervalList intervalAnd(const IntervalList &a, const IntervalList &b) {
    IntervalList ret;
    IntervalList::const_iterator itA = a.begin();
    IntervalList::const_iterator itB = b.begin();
    while(itA != a.end() && itB != b.end()) {
      // B ends before A starts:
      if(itB->second < itA->first) {
        ++itB;
        continue;
      }
      // A ends before B starts:
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
      // B ends before A starts:
      if(itB->second < itA->first) {
        ret.push_back(*itB);
        ++itB;
        continue;
      }
      // A ends before B starts:
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

  IntervalList intervalInverseRadians(const IntervalList &l, double min, double max) {
    IntervalList ret;
    if(l.empty()) {
      return toIntervalsRadians(min, max);
    }

    IntervalList::const_iterator it = l.begin();
    if(min < max) {
      if(it->first > min)
        ret.push_back(Interval(min, it->first));
      double last = it->second;
      ++it;
      for(; it != l.end(); ++it) {
        ret.push_back(Interval(last, it->first));
        last = it->second;
      }
      if(last != max)
        ret.push_back(Interval(last, max));
    }
    else {
      // First section:
      if(it->first >= min) // Nothing in first section:
        ret.push_back(Interval(-M_PI, max));
      else {
        if(it->first > -M_PI)
          ret.push_back(Interval(-M_PI, it->first));
        double last = it->second;
        ++it;
        for(; it != l.end() && it->first < max; ++it) {
          ret.push_back(Interval(last, it->first));
          last = it->second;
        }
        if(last != max)
          ret.push_back(Interval(last, max));
      }

      // Second section:
      if(it == l.end()) { // Nothing in second section:
        ret.push_back(Interval(min, M_PI));
      }
      else {
        if(it->first > min)
          ret.push_back(Interval(min, it->first));
        double last = it->second;
        ++it;
        for(; it != l.end(); ++it) {
          ret.push_back(Interval(last, it->first));
          last = it->second;
        }
        if(last != M_PI)
          ret.push_back(Interval(last, M_PI));
      }
    }
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
      if(eqEpsilon(prev.second, it->first)) {
        prev.second = it->second;
      }
      else {
        ret.push_back(prev);
        prev = *it;
      }
    }
    ret.push_back(prev);
    
#ifdef _TRACE
    if(!intervalEquals(ret, l))
      std::cout << "COLLAPSING INTERVALS: " << l << " -> " << ret << std::endl;
#endif
    return ret;
  }

  void intervalToArray(const Interval &fullInterval, const IntervalList &l, bool *array, unsigned int sizeArray) {
    const double min = fullInterval.first;
    const double max = fullInterval.second;
    IntervalList::const_iterator it = l.begin();
    for(unsigned int i = 0; i < sizeArray; ++i) {
      double v = min + ((max-min)*i)/((double)sizeArray-1);
      if(it == l.end() || v < it->first) {
        array[i] = false; // sizeArray-1-i
      }
      else if(v > it->second) {
        array[i] = false; // sizeArray-1-i
        ++it;
      }
      else
        array[i] = true; // sizeArray-1-i
    }
  }

  IntervalList intervalReverse(const IntervalList &l) {
    IntervalList ret;
    for(int i = l.size()-1; i >= 0; --i) {
      ret.push_back(Interval(-l[i].second, -l[i].first));
    }
    return ret;
  }

  IntervalList toIntervalsRadians(double min, double max) {
    IntervalList ret;
    if(min < max) {
      ret.push_back(Interval(min, max));
    }
    else {
      ret.push_back(Interval(-M_PI, max));
      ret.push_back(Interval(min, M_PI));
    }
    return ret;
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

