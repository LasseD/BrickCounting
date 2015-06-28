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
  
  int findCircleCircleIntersectionsLeftOfLine(double r, const Point &p, double pr, const LineSegment &l, Point &i1, Point &i2, bool &raise) {
    int ret = findCircleCircleIntersections(r, p, pr, i1, i2);
    if(ret == 0)
      return ret;
    if(!rightTurn(l.P1, l.P2, p)) {
      std::cout << "!rightturn(" << l.P1 <<","<< l.P2 <<","<< p << ")" << std::endl;
      raise = true;
      return 0;
    }
    //assert(!rightTurn(l.P1, l.P2, p));
    bool i1OK = !rightTurn(l.P1, l.P2, i1);
    bool i2OK = !rightTurn(l.P1, l.P2, i2);
    if(i1OK && i2OK)
      return 2;
    if(!i1OK && !i2OK)
      return 0;
    if(i1OK)
      return 1;
    i1 = i2;
    return 1;
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

  IntervalList fullInterval(double min, double max) {
    IntervalList ret;
    if(max - min > M_PI) {
      ret.push_back(Interval(0, min));
      ret.push_back(Interval(max, 2*M_PI));    
    }
    else
      ret.push_back(Interval(min,max));
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
  for(IntervalList::const_iterator it = l.begin(); it != l.end(); ++it) {
    os << "[" << *it << "]";
  }
  return os;
}

