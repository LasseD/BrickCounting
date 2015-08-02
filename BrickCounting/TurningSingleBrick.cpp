#include "TurningSingleBrick.h"

namespace math {
  double angleToOriginalInterval(double a, const RadianInterval &interval) {
    const double &minAngle = interval.P1;
    const double &maxAngle = interval.P2;

    if(maxAngle < minAngle) {
      // There are now two intervals: [-PI;maxAngle] and [minAngle;PI].
      if(a <= maxAngle) {
        return MAX_ANGLE_RADIANS - 2*MAX_ANGLE_RADIANS * (maxAngle-a)/(2*M_PI+maxAngle-minAngle);
      }
      else {
        return -MAX_ANGLE_RADIANS + 2*MAX_ANGLE_RADIANS * (a-minAngle)/(2*M_PI+maxAngle-minAngle);
      }
    }
    else {
      return -MAX_ANGLE_RADIANS + (2*MAX_ANGLE_RADIANS * (a-minAngle)/(maxAngle-minAngle));
    }
  }

  IntervalList intervalsToOriginalInterval(const IntervalList &l, const RadianInterval &interval) {
    IntervalList ret;
    for(IntervalList::const_iterator it = l.begin(); it != l.end(); ++it) {
      double a = angleToOriginalInterval(it->first, interval);
      double b = angleToOriginalInterval(it->second, interval);
      ret.push_back(Interval(a,b));
    }
    std::sort(ret.begin(), ret.end());
    ret = math::collapseIntervals(ret);
    return ret;
  }
}

Fan::Fan() : radius(0), interval(0,0) {}
Fan::Fan(const Fan &f) : radius(f.radius), interval(f.interval) {}
Fan::Fan(double radius, double min, double max) : radius(radius), interval(min,max) {}
Fan::Fan(double radius, const RadianInterval &interval) : radius(radius), interval(interval) {}

MovingStud::MovingStud() : radius(0), interval(0,0) {}
MovingStud::MovingStud(const MovingStud &f) : radius(f.radius), interval(f.interval) {}
MovingStud::MovingStud(double radius, double min, double max) : radius(radius), interval(min,max) {}

/*
A fan intersects a line segment if the segment intersects the circle between minAngle and maxAngle
*/
bool Fan::intersectsLineSegment(const LineSegment &l) const {
  Point i1, i2;
  int intersections = math::findCircleLineIntersections(radius-EPSILON, l, i1, i2);
  if(intersections == 0)
    return false;
  // Return true if one of the intersections is between minAngle and maxAngle:
  if(math::between(l.P1, i1, l.P2) && math::inRadianInterval(math::angleOfPoint(i1), interval)) {
    return true;
  }
  if(math::between(l.P1, i2, l.P2) && math::inRadianInterval(math::angleOfPoint(i2), interval)) {
    return true;
  }
  return false;
}

/*
A fan intersects a stud if the distance to the center of the stud is less than radius + stud radius
AND if the center of the stud lies between minAngle and maxAngle
*/
bool Fan::intersectsStud(const Point &stud) const {
  return math::normSq(stud) < (radius + STUD_RADIUS)*(radius + STUD_RADIUS) && 
         math::inRadianInterval(math::angleOfPoint(stud), interval);
}

std::ostream& operator<<(std::ostream &os, const Fan& f) {
  os << "Fan[r=" << f.radius << ",min=" << f.interval.P1 << ",max=" << f.interval.P2 << "]";
  return os;
}

/*
A tract intersects a line segment if both end points of the line segment lie inside the tract, or if the line segment intersects either the inner or outer tract wall.
*/
bool MovingStud::tractIntersectsLineSegment(const LineSegment &l) const {
  const double normP1 = math::norm(l.P1);
  const double normP2 = math::norm(l.P2);
  const double innerWallRadius = radius - STUD_RADIUS;
  const double outerWallRadius = radius + STUD_RADIUS;
  bool endPointsBetweenWalls = (innerWallRadius <= normP1 && normP1 <= outerWallRadius) &&
                               (innerWallRadius <= normP2 && normP2 <= outerWallRadius);
  bool endPointsInside = endPointsBetweenWalls && (math::inRadianInterval(math::angleOfPoint(l.P1), interval) || 
                                                   math::inRadianInterval(math::angleOfPoint(l.P2), interval));
  return endPointsInside || 
    Fan(innerWallRadius, interval).intersectsLineSegment(l) ||
    Fan(outerWallRadius, interval).intersectsLineSegment(l);
}

Point MovingStud::minPoint() const {
  return Point(radius*cos(interval.P1), radius*sin(interval.P1));
}

Point MovingStud::maxPoint() const {
  return Point(radius*cos(interval.P2), radius*sin(interval.P2));
}

/*
The moving stud intersects a stud, if it intersects the tract or one of the end circles.
*/
bool MovingStud::intersectsStud(const Point &stud) const {  
  bool intersectsTract = math::normSq(stud) > (radius - STUD_DIAM)*(radius - STUD_DIAM) && 
                         math::normSq(stud) < (radius + STUD_DIAM)*(radius + STUD_DIAM) && 
                         math::inRadianInterval(math::angleOfPoint(stud), interval);
  if(intersectsTract)
    return true;
  Point minP = minPoint();
  if(math::distSq(minP, stud) < STUD_DIAM*STUD_DIAM)
    return true;
  Point maxP = maxPoint();
  return math::distSq(maxP, stud) < STUD_DIAM*STUD_DIAM;
}

std::ostream& operator<<(std::ostream &os, const MovingStud& f) {
  os << "MovingStud[r=" << f.radius << ",min=" << f.interval.P1 << ",max=" << f.interval.P2 << "]";
  return os;
}

void TurningSingleBrick::createBricksAndStudTranslation(const Configuration &configuration, const IConnectionPair &connectionPair) {
  int prevBrickI = connectionPair.P1.first.configurationSCCI;
  const Brick &prevOrigBrick = configuration.origBricks[prevBrickI];
  const ConnectionPoint &prevPoint = connectionPair.P1.second;
  const ConnectionPoint &currPoint = connectionPair.P2.second;
  Brick prevBrick(prevOrigBrick, connectionPair.P1.second.brick);
  studTranslation = prevBrick.getStudPosition(prevPoint.type);

  double angle = prevBrick.angle + M_PI/2*(currPoint.type-prevPoint.type-2);
  int8_t level = prevOrigBrick.level + prevPoint.brick.level() + (prevPoint.above ? 1 : -1);

  RectilinearBrick rb;
  blocks[0] = Brick(rb, currPoint, Point(0,0), angle-MAX_ANGLE_RADIANS, level);
  blocks[1] = Brick(rb, currPoint, Point(0,0), angle+MAX_ANGLE_RADIANS, level);
  blockAbove = Brick(rb, currPoint, Point(0,0), angle, level);
}

void TurningSingleBrick::createMovingStuds() {
  Point pois1[NUMBER_OF_STUDS];
  blocks[0].getStudPositions(pois1);
  Point pois2[NUMBER_OF_STUDS];
  blocks[1].getStudPositions(pois2);
  for(int i = 0; i < NUMBER_OF_STUDS; ++i) {
    double radius = math::norm(pois1[i]);
    if(radius < EPSILON) {
      movingStuds[i] = MovingStud(0, -M_PI, M_PI);
      continue;
    }
    double minAngle = math::angleOfPoint(pois1[i]);
    double maxAngle = math::angleOfPoint(pois2[i]);
    assert(math::abs(math::norm(pois2[i]) - radius) < EPSILON);
    movingStuds[i] = MovingStud(radius, minAngle, maxAngle);
  }
}
