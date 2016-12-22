#ifndef MODELLING_BRICK_H
#define MODELLING_BRICK_H

#include <assert.h>
#include <stdint.h>
#include <iostream>
#include <vector>

#include "../Common.h"
#include "../geometry/BasicGeometry.h"
#include "RectilinearBrick.h"
#include "ConnectionPoint.h"

#define NUMBER_OF_POIS_FOR_BOX_INTERSECTION 10
#define NUMBER_OF_STUDS 8
// 4mm in units:
#define VERTICAL_BRICK_HALF_WIDTH 1.0
#define VERTICAL_BRICK_CENTER_TO_SIDE 0.9875
#define VERTICAL_BRICK_CENTER_TO_TOP 1.9875
#define BRICK_UNIT_GAP 0.0000125
#define STUD_RADIUS 0.3
#define STUD_DIAM (STUD_RADIUS+STUD_RADIUS)
// 0.0625 is 0.5 mm.
#define SNAP_DISTANCE 0.0625
// Molding tolerance: 0.1mm
#define MOLDING_TOLERANCE_MULTIPLIER 1000
#define EPSILON_TOLERANCE_MULTIPLIER 1

namespace modelling {
	typedef std::pair<double, double> ClickInfo; // angle, dist of stud making the click.

												 /*
												 A Brick at any location and angle.
												 Default same as a Rectilinear brick: vertical at 0,0,0 (angle 0)
												 */
	class Brick; // Forward declaration so that << can be used in template methods.
	std::ostream& operator<<(std::ostream &os, const Brick& b);

	class Brick {
	public:
		geometry::Point center;
		double angle; // angle 0 = vertical.
		int8_t level;

		Brick(const Brick& b) : center(b.center), angle(b.angle), level(b.level) {}
		Brick(const RectilinearBrick& b) : center(b.x, b.y), angle(b.horizontal() ? -M_PI / 2 : 0), level(b.level()) {}
		Brick(const Brick& b, const RectilinearBrick& rb);
		Brick() : center(0, 0), angle(0), level(0) {}
		Brick(double cx, double cy, double a, int8_t lv) : center(cx, cy), angle(a), level(lv) {}
		Brick(const RectilinearBrick& b, const ConnectionPoint& p, const geometry::Point &origin, double originAngle, int8_t originLv);

		void /*Brick::*/toLDR(std::ofstream &os, int ldrColor) const;

		RectilinearBrick /*Brick::*/toRectilinearBrick() const;

		void /*Brick::*/moveBrickSoThisIsAxisAlignedAtOrigin(Brick &b) const;
		void /*Brick::*/movePointSoThisIsAxisAlignedAtOrigin(geometry::Point &p) const;

		template <int ADD_XY>
		void /*Brick::*/getBoxPOIs(geometry::Point *pois) const {
			double sina = sin(angle);
			double cosa = cos(angle);

			const double dx = VERTICAL_BRICK_CENTER_TO_SIDE + ADD_XY * BRICK_UNIT_GAP;
			const double dy = VERTICAL_BRICK_CENTER_TO_TOP + ADD_XY * BRICK_UNIT_GAP;

			// 4 corners:
			pois[0] = geometry::Point(center.X + (-dx*cosa - dy*sina), center.Y + (-dx*sina + dy*cosa));
			pois[1] = geometry::Point(center.X + (dx*cosa - dy*sina), center.Y + (dx*sina + dy*cosa));
			pois[2] = geometry::Point(center.X + (dx*cosa + dy*sina), center.Y + (dx*sina - dy*cosa));
			pois[3] = geometry::Point(center.X + (-dx*cosa + dy*sina), center.Y + (-dx*sina - dy*cosa));
			// 2 inner:
			pois[4] = geometry::Point(center.X + 0.75*sina, center.Y - 0.75*cosa);
			pois[5] = geometry::Point(center.X - 0.75*sina, center.Y + 0.75*cosa);
			// 4 sides:
			pois[6] = geometry::Point(center.X + dx*cosa, center.Y + dx*sina);
			pois[7] = geometry::Point(center.X - dx*cosa, center.Y - dx*sina);
			pois[8] = geometry::Point(center.X + dy*sina, center.Y - dy*cosa);
			pois[9] = geometry::Point(center.X - dy*sina, center.Y + dy*cosa);
		}

		template <int ADD_XY, int STUD_ADD>
		void /*Brick::*/getBoxLineSegments(geometry::LineSegment *segments) const {
			double sina = sin(angle);
			double cosa = cos(angle);
			// 4 corners:
			const double dx = VERTICAL_BRICK_CENTER_TO_SIDE + ADD_XY * BRICK_UNIT_GAP;
			const double DX = dx + STUD_ADD*STUD_RADIUS;
			const double dy = VERTICAL_BRICK_CENTER_TO_TOP + ADD_XY * BRICK_UNIT_GAP;
			const double DY = dy + STUD_ADD*STUD_RADIUS;

			segments[0].P1 = geometry::Point(center.X + (-DX*cosa - dy*sina), center.Y + (-DX*sina + dy*cosa));
			segments[0].P2 = geometry::Point(center.X + (DX*cosa - dy*sina), center.Y + (DX*sina + dy*cosa));

			segments[1].P1 = geometry::Point(center.X + (dx*cosa - DY*sina), center.Y + (dx*sina + DY*cosa));
			segments[1].P2 = geometry::Point(center.X + (dx*cosa + DY*sina), center.Y + (dx*sina - DY*cosa));

			segments[2].P1 = geometry::Point(center.X + (DX*cosa + dy*sina), center.Y + (DX*sina - dy*cosa));
			segments[2].P2 = geometry::Point(center.X + (-DX*cosa + dy*sina), center.Y + (-DX*sina - dy*cosa));

			segments[3].P1 = geometry::Point(center.X + (-dx*cosa + DY*sina), center.Y + (-dx*sina - DY*cosa));
			segments[3].P2 = geometry::Point(center.X + (-dx*cosa - DY*sina), center.Y + (-dx*sina + DY*cosa));
		}

		bool /*Brick::*/outerStudIntersectsStudAtOrigin() const;
		void /*Brick::*/getStudIntersectionWithMovingStud(double radius, double minAngle, double maxAngle, std::vector<ClickInfo> &found) const;

		/*
		Returns the intersection between a rectangle defined by four points and a circle cutout (between minAngle and maxAngle).
		*/
		template <int ADD_XY>
		geometry::IntervalList /*Brick::*/rectangleIntersectionWithCircle(geometry::Point const * const points, double radius, double minAngle, double maxAngle) const {
#ifdef _TRACE
			std::cout << "    RECT VS Circle INIT. Rect=" << points[0] << "; " << points[1] << "; " << points[2] << "; " << points[3] << ", radius=" << radius << std::endl;
#endif
			geometry::IntervalList ret;

			bool retInitiated = false;
			// Intersection with the circle must be on the right side of all four line segments:
			for (int i = 0; i < 4; ++i) {
				// Find intersections:
				geometry::RadianInterval intersection;
				const geometry::Point &p1 = points[i];
				const geometry::Point &p2 = points[(i + 1) % 4];
				geometry::LineSegment segment(p1, p2);
#ifdef _TRACE
				std::cout << "     RECT VS Circle SEGMENT: " << segment << std::endl;
#endif
				bool intersects = geometry::findCircleHalfPlaneIntersection(radius, segment, intersection);
				if (!intersects) {
#ifdef _TRACE
					std::cout << "      RECT VS Circle BAIL." << std::endl;
#endif
					geometry::IntervalList empty;
					return empty;
				}
				if (!retInitiated) {
					geometry::intervalAndRadians(geometry::RadianInterval(minAngle, maxAngle), intersection, ret);
#ifdef _TRACE
					const double &intersectionMin = intersection.P1;
					const double &intersectionMax = intersection.P2;
					std::cout << "      RECT VS Circle &&. " << intersectionMin << "," << intersectionMax << " & " << minAngle << "," << maxAngle << " = " << ret << std::endl;
#endif
					retInitiated = true;
				}
				else {
					geometry::IntervalList toMerge;
					geometry::intervalAndRadians(geometry::RadianInterval(minAngle, maxAngle), intersection, toMerge);
#ifdef _TRACE
					const double &intersectionMin = intersection.P1;
					const double &intersectionMax = intersection.P2;
					std::cout << "      RECT VS Circle &&. " << intersectionMin << "," << intersectionMax << " & " << minAngle << "," << maxAngle << " = " << toMerge << " => " << geometry::intervalAnd(ret, toMerge) << std::endl;
#endif
					geometry::IntervalList copyRet(ret); ret.clear();
					geometry::intervalAnd(copyRet, toMerge, ret);
				}
				if (ret.empty())
					return ret;
			}
#ifdef _TRACE
			if (!ret.empty()) {
				std::cout << "     RECT VS Circle. Rect=" << points[0] << "; " << points[1] << "; " << points[2] << "; " << points[3] << std::endl;
				std::cout << "     RECT VS Circle => " << ret << std::endl;
			}
#endif
			return ret;
		}

		template <int ADD_XY>
		bool /*Brick::*/boxIntersectsInnerStud(geometry::Point &stud) const {
			const double cornerX = VERTICAL_BRICK_CENTER_TO_SIDE + ADD_XY * BRICK_UNIT_GAP;
			const double cornerY = VERTICAL_BRICK_CENTER_TO_TOP + ADD_XY * BRICK_UNIT_GAP;

			if (stud.X < 0) stud.X = -stud.X;
			if (stud.Y < 0) stud.Y = -stud.Y;

			if (stud.X >= cornerX + STUD_RADIUS || stud.Y >= cornerY + STUD_RADIUS) {
				return false;
			}

			// Might intersect - check corner case:
			if (stud.X < cornerX || stud.Y < cornerY) {
				return true;
			}
			const double dx = (stud.X - cornerX);
			const double dy = (stud.Y - cornerY);
			if (STUD_RADIUS*STUD_RADIUS > dx*dx + dy*dy) {
				return true;
			}
			return false;
		}

		/*
		Helper method for blockIntersectionWithMovingStud when radius is 0.
		If stud doesn't intersect, then return empty interval.
		Otherwise, return full interval.
		*/
		template <int ADD_XY>
		geometry::IntervalList /*Brick::*/blockIntersectionWithRotatingStud(double minAngle, double maxAngle, bool allowClick) const {
			geometry::Point p(0, 0);
			movePointSoThisIsAxisAlignedAtOrigin(p);
			geometry::IntervalList ret;
			if (allowClick) {
				bool fakeConnected = false;
				ConnectionPoint fakeCP;
				RectilinearBrick fakeRB;
				if (boxIntersectsOuterStud<ADD_XY>(p, fakeConnected, fakeCP, fakeCP, fakeRB, fakeRB, 4)) {
					geometry::toIntervalsRadians(geometry::RadianInterval(minAngle, maxAngle), ret);
				}
			}
			else {
				if (boxIntersectsInnerStud<ADD_XY>(p)) {
					geometry::toIntervalsRadians(geometry::RadianInterval(minAngle, maxAngle), ret);
				}
			}
			return ret;
		}

		/*
		Helper method for blockIntersectionWithFan when radius is 0.
		If point doesn't intersect, then return empty interval.
		Otherwise, return full interval.
		*/
		template <int ADD_XY>
		geometry::IntervalList /*Brick::*/blockIntersectionWithRotatingPoint(double minAngle, double maxAngle) const {
			geometry::Point p(0, 0);
			movePointSoThisIsAxisAlignedAtOrigin(p);

			const double cornerX = VERTICAL_BRICK_CENTER_TO_SIDE + ADD_XY * BRICK_UNIT_GAP;
			const double cornerY = VERTICAL_BRICK_CENTER_TO_TOP + ADD_XY * BRICK_UNIT_GAP;

			if (p.X < 0) p.X = -p.X;
			if (p.Y < 0) p.Y = -p.Y;

			geometry::IntervalList ret;
			if (p.X < cornerX && p.Y < cornerY)
				geometry::toIntervalsRadians(geometry::RadianInterval(minAngle, maxAngle), ret);
			return ret;
		}

		/*
		Investigates the equivalent problem where a circle (center p, radius) intersects the Minkowski sum of brick and a stud.
		Intersects block on either a line segment or a quarter-circle at one of the corners. This problem can be split into two intersections against rectangles and four against circles.
		Returns intervals of intersections between minAngle and maxAngle.
		*/
		template <int ADD_XY>
		geometry::IntervalList /*Brick::*/blockIntersectionWithMovingStud(double radius, double minAngle, double maxAngle) const {
#ifdef _TRACE
			bool spilled = false;
#endif
			// First check line segments: The intersection with the moving stud must be on the right side of ALL four segments:
			geometry::LineSegment segments[4];
			getBoxLineSegments<ADD_XY, 1>(segments); // ",1" ensures line segments are moved one stud radius out.    
			geometry::Point p1[4] = { segments[0].P1, segments[0].P2, segments[2].P1, segments[2].P2 };
			geometry::IntervalList ret = rectangleIntersectionWithCircle<ADD_XY>(p1, radius, minAngle, maxAngle);
#ifdef _TRACE
			if (!ret.empty()) {
				std::cout << "  BLOCK vs MS " << *this << ", r=" << radius << ", [" << minAngle << ";" << maxAngle << "]" << std::endl;
				geometry::LineSegment sx[4];
				getBoxLineSegments<ADD_XY, 0>(sx); // ",1" ensures line segments are moved one stud radius out.    
				std::cout << "  BLOCK vs MS BASE BLOCK: " << sx[0] << ", " << sx[2] << std::endl;
				std::cout << "   WIDE Box intersect: " << ret << std::endl;
				spilled = true;
			}
#endif

			geometry::Point p2[4] = { segments[1].P1, segments[1].P2, segments[3].P1, segments[3].P2 };
			geometry::IntervalList intersectionsWithRect2 = rectangleIntersectionWithCircle<ADD_XY>(p2, radius, minAngle, maxAngle);
			geometry::IntervalList copyRet(ret); ret.clear();
			geometry::intervalOr(copyRet, intersectionsWithRect2, ret);
#ifdef _TRACE
			if (!ret.empty()) {
				if (!spilled) {
					std::cout << "  BLOCK vs MS " << *this << ", r=" << radius << ", [" << minAngle << ";" << maxAngle << "]" << std::endl;
					geometry::LineSegment sx[4];
					getBoxLineSegments<ADD_XY, 0>(sx); // ",1" ensures line segments are moved one stud radius out.    
					std::cout << "  BLOCK vs MS BASE BLOCK: " << sx[0] << ", " << sx[2] << std::endl;
					spilled = true;
				}
				std::cout << "   TALL Box intersect: " << intersectionsWithRect2 << " => " << ret << std::endl;
			}
#endif

			// Now check quarter circles: 
			//  Subtract the intersection intervals that intersect the corner circles on the left of the corner dividers:
			geometry::Point pois[NUMBER_OF_POIS_FOR_BOX_INTERSECTION];
			getBoxPOIs<ADD_XY>(pois);
			for (int i = 0; i < 4; ++i) {
				// Find intersections:
				geometry::IntervalList intervalFromCircle;
				geometry::findCircleCircleIntersection(radius, pois[i], STUD_RADIUS, intervalFromCircle);
				for (const geometry::Interval* it = intervalFromCircle.begin(); it != intervalFromCircle.end(); ++it) {
					geometry::IntervalList intervalFromCircleInCorrectInterval;
					geometry::intervalAndRadians(geometry::RadianInterval(minAngle, maxAngle), geometry::RadianInterval(it->first, it->second), intervalFromCircleInCorrectInterval);
					geometry::IntervalList copyRet2(ret); ret.clear();
					geometry::intervalOr(copyRet2, intervalFromCircleInCorrectInterval, ret);
#ifdef _TRACE
					if (!ret.empty()) {
						if (!spilled) {
							std::cout << "  BLOCK vs MS " << *this << ", r=" << radius << ", [" << minAngle << ";" << maxAngle << "]" << std::endl;
							geometry::LineSegment sx[4];
							getBoxLineSegments<ADD_XY, 0>(sx); // ",1" ensures line segments are moved one stud radius out.    
							std::cout << "  BLOCK vs MS BASE BLOCK: " << sx[0] << ", " << sx[2] << std::endl;
							spilled = true;
						}
						std::cout << "   CORNER<" << ADD_XY << "> " << i << " (" << pois[i] << ") intersect: " << *it << "=>" << intervalFromCircleInCorrectInterval << " => " << ret << std::endl;
					}
#endif
				}
			}

			return ret;
		}

		/*
		Investigates the equivalent problem where a circle (center p, radius) intersects the brick.
		Returns intervals of intersections between minAngle and maxAngle.
		*/
		template <int ADD_XY>
		geometry::IntervalList /*Brick::*/blockIntersectionWithMovingPoint(double radius, double minAngle, double maxAngle) const {
			geometry::Point pois[NUMBER_OF_POIS_FOR_BOX_INTERSECTION];
			getBoxPOIs<ADD_XY>(pois);
			return rectangleIntersectionWithCircle<ADD_XY>(pois, radius, minAngle, maxAngle);
		}

		geometry::Point /*Brick::*/getStudPosition(ConnectionPointType type) const;
		void /*Brick::*/getStudPositions(geometry::Point *positions) const;

		template <int ADD_XY>
		bool /*Brick::*/boxIntersectsPOIsFrom(Brick &b) const {
			const double dx = VERTICAL_BRICK_CENTER_TO_SIDE + ADD_XY * BRICK_UNIT_GAP;
			const double dy = VERTICAL_BRICK_CENTER_TO_TOP + ADD_XY * BRICK_UNIT_GAP;
			moveBrickSoThisIsAxisAlignedAtOrigin(b);
			// Get POIs:
			geometry::Point pois[NUMBER_OF_POIS_FOR_BOX_INTERSECTION];
			b.getBoxPOIs<ADD_XY>(pois);
			// Check each POI:
			for (int i = 0; i < NUMBER_OF_POIS_FOR_BOX_INTERSECTION; ++i) {
				geometry::Point &poi = pois[i];
				if (poi.X < 0)
					poi.X = -poi.X;
				if (poi.Y < 0)
					poi.Y = -poi.Y;
				if (poi.X + EPSILON < dx && poi.Y + EPSILON < dy) {
					return true;
				}
			}
			return false;
		}

		/*
		Assumes b is on same level.
		*/
		template <int ADD_XY>
		bool /*Brick::*/boxesIntersect(const Brick &b) const {
			Brick tmpB(b);
			Brick tmpThis(*this);
			return boxIntersectsPOIsFrom<ADD_XY>(tmpB) || b.boxIntersectsPOIsFrom<ADD_XY>(tmpThis);
		}

		template <int ADD_XY>
		bool /*Brick::*/boxIntersectsOuterStud(const geometry::Point &studOfB, bool &connected, ConnectionPoint &foundConnectionThis, ConnectionPoint &foundConnectionB, const RectilinearBrick &source, const RectilinearBrick &bSource, int i) const {
			const double cornerX = VERTICAL_BRICK_CENTER_TO_SIDE + ADD_XY * BRICK_UNIT_GAP;
			const double cornerY = VERTICAL_BRICK_CENTER_TO_TOP + ADD_XY * BRICK_UNIT_GAP;

			geometry::Point stud(studOfB);
			if (stud.X < 0) stud.X = -stud.X;
			if (stud.Y < 0) stud.Y = -stud.Y;

			if (stud.X >= cornerX + STUD_RADIUS || stud.Y >= cornerY + STUD_RADIUS)
				return false;

			// X check if it hits stud:
			const double studX = HALF_STUD_DISTANCE;
			const double studY = STUD_AND_A_HALF_DISTANCE;
			if (SNAP_DISTANCE*SNAP_DISTANCE >= (stud.X - studX)*(stud.X - studX) + (stud.Y - studY)*(stud.Y - studY)) {
				// We are already connected:
				if (connected) {
					connected = false;
					return true;
				}
				// Compute corner:
				connected = true;
				if (studOfB.X < 0 && studOfB.Y < 0)
					foundConnectionThis = ConnectionPoint(SW, source, false, -1);
				else if (studOfB.X < 0 && studOfB.Y > 0)
					foundConnectionThis = ConnectionPoint(NW, source, false, -1);
				else if (studOfB.X > 0 && studOfB.Y < 0)
					foundConnectionThis = ConnectionPoint(SE, source, false, -1);
				else // if(stud.X > 0 && stud.Y > 0)
					foundConnectionThis = ConnectionPoint(NE, source, false, -1);
				foundConnectionB = ConnectionPoint((ConnectionPointType)(i - 4), bSource, true, -1);
				return false;
			}

			// Might intersect - check corner case:
			if (stud.X < cornerX || stud.Y < cornerY || STUD_RADIUS*STUD_RADIUS >(stud.X - cornerX)*(stud.X - cornerX) + (stud.Y - cornerY)*(stud.Y - cornerY)) {
				connected = false;
				return true;
			}
			return false;
		}

		template <int ADD_XY>
		bool /*Brick::*/boxIntersectsStudsFrom(Brick &b, const RectilinearBrick &bSource, bool &connected, ConnectionPoint &foundConnectionB, ConnectionPoint &foundConnectionThis, const RectilinearBrick &source) const {
			moveBrickSoThisIsAxisAlignedAtOrigin(b);
			geometry::Point studsOfB[NUMBER_OF_STUDS];
			b.getStudPositions(studsOfB);

			// X handle four inner:
			for (int i = 0; i < 4; ++i) {
				geometry::Point &stud = studsOfB[i];
				if (boxIntersectsInnerStud<ADD_XY>(stud)) {
					connected = false;
					return true;
				}
			}
			// Handle four outer specially as they might cause connection:
			for (int i = 4; i < NUMBER_OF_STUDS; ++i) {
				geometry::Point stud = studsOfB[i];
				if (boxIntersectsOuterStud<ADD_XY>(stud, connected, foundConnectionThis, foundConnectionB, source, bSource, i))
					return true;
			}
			return connected;
		}

		/*
		Return true if this brick intersects the other.
		If the two bricks are corner connected, then connected is set to true and the found connection is set.
		1: If not even close level-wise: return false.
		2: If on same level: Check that boxes do not collide.
		- If current implementation doesn't work, use algorithm from http://www.ragestorm.net/tutorial?id=22
		3: If on adjacent levels:
		- If any of the 8 studs of lower brick inside box of upper: intersect.
		- If intersect: Check if only intersecting one corner stud.
		- If only one corner stud, check if corner connected.
		*/
		template <int ADD_XY>
		bool intersects(const Brick &b, const RectilinearBrick &bSource, bool &connected, ConnectionPoint &foundConnectionB, ConnectionPoint &foundConnectionThis, const RectilinearBrick &source) const {
			connected = false;
			if (level > b.level + 1 || b.level > level + 1)
				return false;
			if (level == b.level) {
				bool ret = boxesIntersect<ADD_XY>(b);
				return ret;
			}
			Brick tmpB(b);
			Brick tmpThis(*this);
			if (level < b.level)
				return b.boxIntersectsStudsFrom<ADD_XY>(tmpThis, source, connected, foundConnectionThis, foundConnectionB, bSource);
			return boxIntersectsStudsFrom<ADD_XY>(tmpB, bSource, connected, foundConnectionB, foundConnectionThis, source);
		}

		bool operator < (const Brick &b) const;
	};
}

#endif // MODELLING_BRICK_H
