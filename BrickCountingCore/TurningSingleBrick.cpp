#include "TurningSingleBrick.h"

namespace geometry {
	double angleToOriginalInterval(double a, const RadianInterval &interval) {
		const double &minAngle = interval.P1;
		const double &maxAngle = interval.P2;

		if (maxAngle < minAngle) {
			// There are now two intervals: [-PI;maxAngle] and [minAngle;PI].
			if (a <= maxAngle) {
				return MAX_ANGLE_RADIANS - 2 * MAX_ANGLE_RADIANS * (maxAngle - a) / (2 * M_PI + maxAngle - minAngle);
			}
			else {
				return -MAX_ANGLE_RADIANS + 2 * MAX_ANGLE_RADIANS * (a - minAngle) / (2 * M_PI + maxAngle - minAngle);
			}
		}
		else {
			return -MAX_ANGLE_RADIANS + (2 * MAX_ANGLE_RADIANS * (a - minAngle) / (maxAngle - minAngle));
		}
	}

	void intervalsToOriginalInterval(const IntervalList &l, const RadianInterval &interval, IntervalList &result) {
		assert(result.empty());
		for (const Interval* it = l.begin(); it != l.end(); ++it) {
			double a = angleToOriginalInterval(it->first, interval);
			double b = angleToOriginalInterval(it->second, interval);
			result.push_back(Interval(a, b));
		}
		std::sort(result.begin(), result.end());
		geometry::collapseIntervals(result);
	}
}

Fan::Fan() : radius(0), interval(0, 0) {}
Fan::Fan(const Fan &f) : radius(f.radius), interval(f.interval) {}
Fan::Fan(double radius, double min, double max) : radius(radius), interval(min, max) {}
Fan::Fan(double radius, const geometry::RadianInterval &interval) : radius(radius), interval(interval) {}

MovingStud::MovingStud() : radius(0), interval(0, 0) {}
MovingStud::MovingStud(const MovingStud &f) : radius(f.radius), interval(f.interval) {}
MovingStud::MovingStud(double radius, double min, double max) : radius(radius), interval(min, max) {}

/*
A fan intersects a line segment if the segment intersects the circle between minAngle and maxAngle
*/
bool Fan::intersectsLineSegment(const geometry::LineSegment &l) const {
	return geometry::circleCutoutIntersectsLineSegment(radius, interval, l);
}

/*
A fan intersects a stud if the distance to the center of the stud is less than radius + stud radius
AND if the center of the stud lies between minAngle and maxAngle
*/
bool Fan::intersectsStud(const geometry::Point &stud) const {
	return geometry::normSq(stud) < (radius + STUD_RADIUS)*(radius + STUD_RADIUS) &&
		geometry::inRadianInterval(geometry::angleOfPoint(stud), interval);
}

std::ostream& operator<<(std::ostream &os, const Fan& f) {
	os << "Fan[r=" << f.radius << ",min=" << f.interval.P1 << ",max=" << f.interval.P2 << "]";
	return os;
}

/*
A tract intersects a line segment if both end points of the line segment lie inside the tract, or if the line segment intersects either the inner or outer tract wall.
Line segment l = P1,P2.
*/
bool MovingStud::tractIntersectsLineSegment(const geometry::LineSegment &l) const {
	const double normP1 = geometry::norm(l.P1);
	const double normP2 = geometry::norm(l.P2);
	const double innerWallRadius = radius - STUD_RADIUS;
	const double outerWallRadius = radius + STUD_RADIUS;

	bool endPointsBetweenWalls = (innerWallRadius <= normP1 && normP1 <= outerWallRadius) &&
		(innerWallRadius <= normP2 && normP2 <= outerWallRadius);
	bool endPointsInside = endPointsBetweenWalls && (geometry::inRadianInterval(geometry::angleOfPoint(l.P1), interval) ||
		geometry::inRadianInterval(geometry::angleOfPoint(l.P2), interval));
	return endPointsInside ||
		Fan(innerWallRadius, interval).intersectsLineSegment(l) ||
		Fan(outerWallRadius, interval).intersectsLineSegment(l);
}

geometry::Point MovingStud::minPoint() const {
	return geometry::Point(radius*cos(interval.P1), radius*sin(interval.P1));
}

geometry::Point MovingStud::maxPoint() const {
	return geometry::Point(radius*cos(interval.P2), radius*sin(interval.P2));
}

/*
The moving stud intersects a stud, if it intersects the tract or one of the end circles.
*/
bool MovingStud::intersectsStud(const geometry::Point &stud) const {
	bool intersectsTract = geometry::normSq(stud) > (radius - STUD_DIAM)*(radius - STUD_DIAM) &&
		geometry::normSq(stud) < (radius + STUD_DIAM)*(radius + STUD_DIAM) &&
		geometry::inRadianInterval(geometry::angleOfPoint(stud), interval);
	if (intersectsTract)
		return true;
	geometry::Point minP = minPoint();
	if (geometry::distSq(minP, stud) < STUD_DIAM*STUD_DIAM)
		return true;
	geometry::Point maxP = maxPoint();
	return geometry::distSq(maxP, stud) < STUD_DIAM*STUD_DIAM;
}

std::ostream& operator<<(std::ostream &os, const MovingStud& f) {
	os << "MovingStud[r=" << f.radius << ",min=" << f.interval.P1 << ",max=" << f.interval.P2 << "]";
	return os;
}

void TurningSingleBrick::createBricksAndStudTranslation(const Configuration &configuration, const IConnectionPair &connectionPair, const RectilinearBrick &rb) {
	int prevBrickI = connectionPair.P1.first.configurationSCCI;
	const Brick &prevOrigBrick = configuration.origBricks[prevBrickI];
	const ConnectionPoint &prevPoint = connectionPair.P1.second;
	const ConnectionPoint &currPoint = connectionPair.P2.second;
	Brick prevBrick(prevOrigBrick, connectionPair.P1.second.brick);
	studTranslation = prevBrick.getStudPosition(prevPoint.type);

	double angle = prevBrick.angle + M_PI / 2 * (currPoint.type - prevPoint.type - 2);
	int8_t level = prevOrigBrick.level + prevPoint.brick.level() + (prevPoint.above ? 1 : -1);

	blocks[0] = Brick(rb, currPoint, geometry::Point(0, 0), angle - MAX_ANGLE_RADIANS, level);
	blocks[1] = Brick(rb, currPoint, geometry::Point(0, 0), angle + MAX_ANGLE_RADIANS, level);
	blockAbove = Brick(rb, currPoint, geometry::Point(0, 0), angle, level);
}

void TurningSingleBrick::createMovingStuds() {
	geometry::Point pois1[NUMBER_OF_STUDS];
	blocks[0].getStudPositions(pois1);
	geometry::Point pois2[NUMBER_OF_STUDS];
	blocks[1].getStudPositions(pois2);
	for (int i = 0; i < NUMBER_OF_STUDS; ++i) {
		double radius = geometry::norm(pois1[i]);
		if (radius < EPSILON) {
			movingStuds[i] = MovingStud(0, -M_PI, M_PI);
			continue;
		}
		double minAngle = geometry::angleOfPoint(pois1[i]);
		double maxAngle = geometry::angleOfPoint(pois2[i]);
		assert(geometry::abs(geometry::norm(pois2[i]) - radius) < EPSILON);
		movingStuds[i] = MovingStud(radius, minAngle, maxAngle);
	}
}
