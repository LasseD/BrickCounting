#include "TurningSingleBrick.h"

#include <cmath>

///////////////////////////////////////////////////////
// Simple geometry functions
///////////////////////////////////////////////////////
namespace math {
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
///////////////////////////////////////////////////////

/*
 A fan intersects a line segment if the segment intersects the circle between minAngle and maxAngle
 */
bool Fan::intersectsLineSegment(const LineSegment &l) const {
  Point i1, i2;
  int intersections = math::findCircleLineIntersections(radius-EPSILON, l, i1, i2);
  if(intersections == 0)
    return false;
  // Return true if one of the intersections is between minAngle and maxAngle:
  if(math::between(l.P1, i1, l.P2) && math::angleBetween(minAngle, math::angleOfPoint(i1), maxAngle)) {
    //std::cout << "Intersects line segment " << l << " at " << i1 << std::endl;
    return true;
  }
  if(math::between(l.P1, i2, l.P2) && math::angleBetween(minAngle, math::angleOfPoint(i2), maxAngle)) {
    //std::cout << "Intersects line segment " << l << " at " << i2 << std::endl;
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
    math::angleBetween(minAngle, math::angleOfPoint(stud), maxAngle);
}

std::ostream& operator<<(std::ostream &os, const Fan& f) {
  os << "Fan[r=" << f.radius << ",min=" << f.minAngle << ",max=" << f.maxAngle << "]";
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
  bool betweenWalls = (innerWallRadius <= normP1 && normP1 <= outerWallRadius) &&
                      (innerWallRadius <= normP2 && normP2 <= outerWallRadius);
  bool inside = betweenWalls && (math::angleBetween(minAngle, math::angleOfPoint(l.P1), maxAngle) || 
				 math::angleBetween(minAngle, math::angleOfPoint(l.P2), maxAngle));
  return inside || 
    Fan(innerWallRadius, minAngle, maxAngle).intersectsLineSegment(l) ||
    Fan(outerWallRadius, minAngle, maxAngle).intersectsLineSegment(l);
}

/*
  The moving stud intersects a stud, if it intersects the tract or one of the end circles.
  TODO LATER: Connect if the distance to the center of the stud is less than click radius. 
 */
bool MovingStud::intersectsStud(const Point &stud) const {  
  return 
    math::normSq(stud) > (radius - STUD_DIAM)*(radius - STUD_DIAM) && 
    math::normSq(stud) < (radius + STUD_DIAM)*(radius + STUD_DIAM) && 
    math::angleBetween(minAngle, math::angleOfPoint(stud), maxAngle);
}

std::ostream& operator<<(std::ostream &os, const MovingStud& f) {
  os << "MovingStud[r=" << f.radius << ",min=" << f.minAngle << ",max=" << f.maxAngle << "]";
  return os;
}

Point TurningSingleBrick::createBricks(const Configuration &configuration, const IConnectionPair &connectionPair) {
  int prevBrickI = connectionPair.P1.first.configurationSCCI;
  const Brick &prevOrigBrick = configuration.origBricks[prevBrickI];
  const ConnectionPoint &prevPoint = connectionPair.P1.second;
  const ConnectionPoint &currPoint = connectionPair.P2.second;
  Brick prevBrick(prevOrigBrick, connectionPair.P1.second.brick);
  Point prevStud = prevBrick.getStudPosition(prevPoint.type);

  double angle = prevBrick.angle + M_PI/2*(currPoint.type-prevPoint.type-2);
  int8_t level = prevOrigBrick.level + prevPoint.brick.level() + (prevPoint.above ? 1 : -1);
  
  RectilinearBrick rb;
  blocks[0] = Brick(rb, currPoint, Point(0,0), angle-MAX_ANGLE_RADIANS, level);
  blocks[1] = Brick(rb, currPoint, Point(0,0), angle+MAX_ANGLE_RADIANS, level);    
  return prevStud;
}

void TurningSingleBrick::createMovingStuds() {
  Point pois1[NUMBER_OF_STUDS];
  blocks[0].getStudPositions(pois1);
  Point pois2[NUMBER_OF_STUDS];
  blocks[1].getStudPositions(pois2);
  for(int i = 0; i < NUMBER_OF_STUDS; ++i) {
    double minAngle = math::angleOfPoint(pois1[i]);
    double maxAngle = math::angleOfPoint(pois2[i]);
    double radius = math::norm(pois1[i]);
    assert(math::abs(math::norm(pois2[i]) - radius) < EPSILON);
    movingStuds[i] = MovingStud(radius, minAngle, maxAngle);
  }
}
