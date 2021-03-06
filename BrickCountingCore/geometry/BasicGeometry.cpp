#include "BasicGeometry.h"

#include <cmath>

#include "../Common.h"

///////////////////////////////////////////////////////
// Simple geometry functions
///////////////////////////////////////////////////////
namespace geometry {
  bool inInterval(int8_t min, int8_t max, int8_t a) {
    return min <= a && a <= max;
  }
  bool distLessThan2(int8_t a, int8_t b) {
    return a == b || a-1 == b || b-1 == a;
  }
  unsigned int diff(int8_t a, int8_t b) {
    if(a > b)
      return a-(int)b;
    return b-(int)a;
  }

  bool eqEpsilon(double a, double b) {
    return a >= b-EPSILON && a <= b+EPSILON;
  }
  bool eqEpsilon(const Point &p1, const Point &p2) {
    return eqEpsilon(p1.X,p2.X) && eqEpsilon(p1.Y,p2.Y);
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

  bool circleCutoutIntersectsLineSegment(double r, const RadianInterval &circleInterval, const LineSegment &l) {
    Point i1, i2;
    int intersections = findCircleLineIntersections(r, l, i1, i2);
    if(intersections == 0)
      return false;
    // Return true if one of the intersections is between minAngle and maxAngle:
    if(betweenEndPointsOfLineSegmentEpsilon(l.P1, i1, l.P2) && inRadianInterval(angleOfPoint(i1), circleInterval)) {
      return true;
    }
    if(betweenEndPointsOfLineSegmentEpsilon(l.P1, i2, l.P2) && inRadianInterval(angleOfPoint(i2), circleInterval)) {
      return true;
    }
    return false;
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
  bool findCircleHalfPlaneIntersection(double radius, const LineSegment &line, RadianInterval &intersection) {
    bool newIntersections = findCircleLineIntersections(radius, line, intersection.P1, intersection.P2);      
    if(!newIntersections) {// != 2) {
      if(rightTurn(line.P1, line.P2, Point(0,0))) { // Inside half plane
        intersection.P1 = -M_PI;
        intersection.P2 = M_PI;
        return true;
      }
      return false; // Ignore no intersection and intersection in a point.
    }

    if(intersection.P1 > intersection.P2)
      std::swap(intersection.P1, intersection.P2);

    // Find mid-point and determine interval inside of half plane:
    double midAngle = (intersection.P1 + intersection.P2)/2;
    Point midPoint(radius*cos(midAngle), radius*sin(midAngle));
    if(rightTurn(line.P1, line.P2, midPoint)) {
      // OK: Mid-point of interval (on side without jump) is inside the half plane.
    }
    else {
      // Swap the intersections to indicate that the side with the jump is inside the half plane.
      std::swap(intersection.P1, intersection.P2);
    }

    return true; // Interval found.
  }

  // Returns a<=b<=c or c<=b<=a
  bool between(double a, double b, double c) {
    return (a <= b && b <= c) || (a >= b && b >= c);
  }
  bool betweenEpsilon(double a, double b, double c) {
    return (a-EPSILON <= b && b <= c+EPSILON) || (a+EPSILON >= b && b >= c-EPSILON);
  }

  bool inRadianInterval(double a, const RadianInterval &interval) {
    const double &from = interval.P1;
    const double &to = interval.P2;
    assert(a >= -M_PI - EPSILON);
    assert(a <= M_PI + EPSILON);
    assert(from >= -M_PI - EPSILON);
    assert(from < M_PI + EPSILON);
    assert(to >= -M_PI - EPSILON);
    assert(to < M_PI + EPSILON);

    if(from > to) { // Jumps at PI/-PI:
      return (-M_PI <= a && a <= to) || (from <= a && a <= M_PI);
    }
    return from <= a && a <= to;
  }
  bool betweenEndPointsOfLineSegmentEpsilon(const Point &a, const Point &b, const Point &c) {
    return betweenEpsilon(a.X, b.X, c.X) && betweenEpsilon(a.Y, b.Y, c.Y);
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
  void findCircleCircleIntersection(double r, const Point &p, double pr, IntervalList &result) {
    assert(result.empty());
    double ai1, ai2;
    bool newIntersections = findCircleCircleIntersections(r, p, pr, ai1, ai2);
    if(!newIntersections)// < 2)
      return;

    // Find the angle interval. Since the first assertion holds, the interval of intersection is less than PI in length:
    if(ai1 > ai2)
      std::swap(ai1, ai2);

    if(ai2-ai1 < M_PI) {
      result.push_back(Interval(ai1, ai2));
    }
    else {
      result.push_back(Interval(-M_PI, ai1));
      result.push_back(Interval(ai2, M_PI));
    }
  }

  /*
  Handles splitting of a and b if they jump.
  */
  void intervalAndRadians(const RadianInterval &a, const RadianInterval &b, IntervalList &result) {
    assert(result.empty());
    const double &a1 = a.P1;
    const double &a2 = a.P2;
    const double &b1 = b.P1;
    const double &b2 = b.P2;

    bool aJumps = a1 > a2;
    bool bJumps = b1 > b2;
    if(aJumps) {
      if(bJumps) {
        // a consists of [a1;M_PI] and [-M_PI;a2],
        // b consists of [b1;M_PI] and [-M_PI;b2]:
        result.push_back(Interval(-M_PI, MIN(a2, b2)));
        if(b1 < a2) {
          result.push_back(Interval(b1, a2));
        }
        if(a1 < b2) {
          result.push_back(Interval(a1, b2));
        }
        result.push_back(Interval(MAX(a1, b1), M_PI));
      }
      else {
        // a consists of [-M_PI;a2] and [a1;M_PI]:
        if(b1 < a2)
          result.push_back(Interval(b1, MIN(b2,a2)));
        if(b2 > a1)
          result.push_back(Interval(MAX(b1,a1), b2));
      }
    }
    else {
      if(bJumps) {
        // b consists of [b1;M_PI] and [-M_PI;b2]:
        if(a1 < b2)
          result.push_back(Interval(a1, MIN(a2, b2)));
        if(a2 > b1)
          result.push_back(Interval(MAX(a1,b1), a2));
      }
      else { // Normal case: No jumping anywhere:
        if(a2 > b1 && a1 < b2)
          result.push_back(Interval(MAX(a1, b1), MIN(a2, b2)));
      }
    }
  }

  bool intervalEquals(const IntervalList &a, const IntervalList &b) {
    if(a.size() != b.size()) {
      return false;
    }
    const Interval* itA = a.begin();
    const Interval* itB = b.begin();
    for(;itA != a.end(); ++itA, ++itB) {
      if(*itA != *itB)
        return false;
    }
    return true;
  }

  bool intervalContains(const IntervalList &a, double d) {
    for(const Interval* it = a.begin(); it != a.end(); ++it) {
      if(it->second < d)
        continue;
      return it->first <= d;
    }
    return false;
  }

  bool isFullInterval(const IntervalList &a, double min, double max) {
    if(a.size() != 1)
      return false;
    const Interval &interval = *a.begin();
    return eqEpsilon(interval.first, min) && eqEpsilon(interval.second, max);
  }

  void intervalAnd(const IntervalList &a, const IntervalList &b, IntervalList &result) {
    assert(result.empty());
    const Interval* itA = a.begin();
    const Interval* itB = b.begin();
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
        if(min < itA->second-EPSILON)
          result.push_back(Interval(min,itA->second));      
        ++itA;
      }
      else {
        if(min < itB->second-EPSILON)
          result.push_back(Interval(min,itB->second));      
        ++itB;
      }
    }
  }

  void intervalOr(const IntervalList &a, const IntervalList &b, IntervalList &result) {
    assert(result.empty());
    const Interval* itA = a.begin();
    const Interval* itB = b.begin();
    while(itA != a.end() && itB != b.end()) {
      // B ends before A starts:
      if(itB->second < itA->first) {
        result.push_back(*itB);
        ++itB;
        continue;
      }
      // A ends before B starts:
      if(itA->second < itB->first) {
        result.push_back(*itA);
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
      result.push_back(Interval(min,max));      
    }
    // Clean up rest:
    while(itA != a.end()) {
      result.push_back(*itA);
      ++itA;
    }
    while(itB != b.end()) {
      result.push_back(*itB);
      ++itB;
    }
  }

  void intervalInverseRadians(const IntervalList &l, const RadianInterval &minmax, IntervalList &result) {
    assert(result.empty());
    const double &min = minmax.P1;
    const double &max = minmax.P2;

    if(l.empty()) {
      toIntervalsRadians(minmax, result);
      return;
    }

    const Interval* it = l.begin();
    if(min < max) {
      if(it->first > min)
        result.push_back(Interval(min, it->first));
      double last = it->second;
      ++it;
      for(; it != l.end(); ++it) {
        result.push_back(Interval(last, it->first));
        last = it->second;
      }
      if(last != max)
        result.push_back(Interval(last, max));
    }
    else {
      // First section:
      if(it->first >= min) // Nothing in first section:
        result.push_back(Interval(-M_PI, max));
      else {
        if(it->first > -M_PI)
          result.push_back(Interval(-M_PI, it->first));
        double last = it->second;
        ++it;
        for(; it != l.end() && it->first < max; ++it) {
          result.push_back(Interval(last, it->first));
          last = it->second;
        }
        if(last != max)
          result.push_back(Interval(last, max));
      }

      // Second section:
      if(it == l.end()) { // Nothing in second section:
        result.push_back(Interval(min, M_PI));
      }
      else {
        if(it->first > min)
          result.push_back(Interval(min, it->first));
        double last = it->second;
        ++it;
        for(; it != l.end(); ++it) {
          result.push_back(Interval(last, it->first));
          last = it->second;
        }
        if(last != M_PI)
          result.push_back(Interval(last, M_PI));
      }
    }
  }

  void collapseIntervals(IntervalList &l) {
    if(l.size() <= 1) {
      return;
    }

    const Interval* it = l.begin();
    unsigned int size = 0;
    Interval prev = *it;
    ++it;
    for(; it != l.end(); ++it) {
      if(eqEpsilon(prev.second, it->first)) {
        prev.second = it->second;
      }
      else {
        l[size++] = prev;
        prev = *it;
      }
    }
    l[size++] = prev;
    l.truncate(size);
  }

  void intervalReverse(IntervalList &l) {
    if(l.empty())
      return;
    if(l.size() == 1) {
      double sec = l[0].second;
      l[0].second = -l[0].first;
      l[0].first = -sec;
      return;
    }

    for(unsigned int i = 0, j = l.size()-1; i <= j; ++i, --j) {
      Interval ii = l[i];
      l[i].second = -l[j].first;
      l[i].first = -l[j].second;
      l[j].second = -ii.first;
      l[j].first = -ii.second;
    }
  }

  void toIntervalsRadians(const RadianInterval &interval, IntervalList &result) {
    assert(result.empty());
    const double &min = interval.P1;
    const double &max = interval.P2;

    if(min < max) {
      result.push_back(Interval(min, max));
    }
    else {
      result.push_back(Interval(-M_PI, max));
      result.push_back(Interval(min, M_PI));
    }
  }

  IntervalListVector::IntervalListVector() : intervalsSize(0), indicatorSize(0) {
    assert(false);std::cerr << "DIE DEFAULT CONSTRUCTOR FOR IntervalListVector SHOULD NEVER BE CALLED" << std::endl;
    int *die = NULL; die[0] = 42;
  }
  IntervalListVector& IntervalListVector::operator=(const IntervalListVector &) {
    return *(new IntervalListVector()); // FAILS ON PURPOSE
  }

  IntervalListVector::IntervalListVector(uint32_t indicatorSize, unsigned int maxLoadFactor) : intervalsSize(512+indicatorSize*maxLoadFactor), indicatorSize(indicatorSize), intervalsI(0) {
    intervals = new Interval[intervalsSize]; // Deleted in ~IntervalListVector()
    indicators = new IntervalIndicator[indicatorSize]; // Deleted in ~IntervalListVector()
  }
  IntervalListVector::~IntervalListVector() {
    delete[] intervals;
    delete[] indicators;
  }
  void IntervalListVector::insert(uint32_t location, const IntervalList &intervalList) {
    assert(location < indicatorSize);
    assert(intervalList.size() < 20);
    if(intervalsI + intervalList.size() > intervalsSize) {
      assert(false);std::cerr << "DIE X017: " << intervalsI <<"+"<< intervalList.size() <<">"<< intervalsSize << std::endl;
      int *die = NULL; die[0] = 42;
    }
    indicators[location].first = intervalsI;
    indicators[location].second = (unsigned short)intervalList.size();
    for(const Interval* it = intervalList.begin(); it != intervalList.end(); ++it)
      intervals[intervalsI++] = *it;
  }
  void IntervalListVector::insertEmpty(uint32_t location) {
    assert(location < indicatorSize);
    assert(intervalsI < intervalsSize);
    indicators[location].first = intervalsI;
    indicators[location].second = 0;
  }
  void IntervalListVector::get(uint32_t location, IntervalList &intervalList) const {
    assert(location < indicatorSize);
    uint32_t intervalsIndex = indicators[location].first;
    assert(intervalsIndex < intervalsSize);
    assert(intervalsIndex + indicators[location].second < intervalsSize);
    for(unsigned short i = 0; i < indicators[location].second; ++i)
      intervalList.push_back(intervals[intervalsIndex + i]);
  }
  Interval IntervalListVector::get(uint32_t location, unsigned short intervalIndex) const {
    assert(location < indicatorSize);
    uint32_t intervalsIndex = indicators[location].first;
    return intervals[intervalsIndex + intervalIndex];
  }
  uint32_t IntervalListVector::sizeIndicator() const {
    return indicatorSize;
  }
  uint32_t IntervalListVector::sizeNonEmptyIntervals() const {
    return intervalsI;
  }
  unsigned short IntervalListVector::intervalSizeForIndicator(uint32_t i) const {
    assert(i < indicatorSize);
    return indicators[i].second;
  }

  /*
  The combination type consists of 1 to 6 integers in the range [1;6]. 3 bits are used per integer, finally three bits contain the number of integers.
   */
  uint32_t encodeCombinationType(const util::TinyVector<int, 6> &l) {
    assert(l.size() <= 6);
    uint32_t ret = 0;
    for(const int* it = l.begin(); it != l.end(); ++it) {
      assert(*it <= 6);
      ret = (ret << 3) + *it;
    }
    ret = (ret << 3) + (uint32_t)l.size();
    return ret;
  }

}

std::ostream& operator<<(std::ostream &os, const geometry::Point& p) {
  os << p.X << "," << p.Y;
  return os;
}

std::ostream& operator<<(std::ostream &os, const geometry::LineSegment& l) {
  os << l.P1 << "->" << l.P2;
  return os;
}

std::ostream& operator<<(std::ostream &os, const geometry::IntervalList& l) {
  os << "{";
  for(const geometry::Interval* it = l.begin(); it != l.end(); ++it) {
    os << "[" << *it << "]";
  }
  os << "}";
  return os;
}
