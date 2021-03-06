#ifndef MODELLING_TURNING_SINGLE_BRICK_H
#define MODELLING_TURNING_SINGLE_BRICK_H

#include "Math.h"
#include "Brick.h"
#include "Model.hpp"
#include <vector>

namespace geometry {
	double angleToOriginalInterval(double a, const RadianInterval &interval);
	void intervalsToOriginalInterval(const IntervalList &l, const RadianInterval &interval, IntervalList &result);
}

namespace modelling {
	/*
	Fan: A "pizza slice" of a circle. Centered at 0,0.
	Angles: There is always a continuous interval between min and max, and min < max.
	Angle: 0 is horizontal ->
	*/
	struct Fan {
		double radius;
		geometry::RadianInterval interval;

		/*Fan::*/Fan();
		/*Fan::*/Fan(const Fan &f);
		/*Fan::*/Fan(double radius, double min, double max);
		/*Fan::*/Fan(double radius, const geometry::RadianInterval &interval);

		bool /*Fan::*/intersectsLineSegment(const geometry::LineSegment &l) const;
		bool /*Fan::*/intersectsStud(const geometry::Point &stud) const;

		/*
		A fan intersects a block if one of the sides (line segments) of the block intersects the curve.
		*/
		template <int ADD_XY>
		bool /*Fan::*/intersectsBox(const Brick &box) const {
			geometry::LineSegment segments[4];
			box.getBoxLineSegments<ADD_XY, 0>(segments);
			for (int i = 0; i < 4; ++i) {
				if (intersectsLineSegment(segments[i])) {
					return true;
				}
			}
			return false;
		}

		template <int ADD_XY>
		void /*Fan::*/allowableAnglesForBlock(const Brick &block, geometry::IntervalList &ret) const {
			// Compute intersections without interval information:
			if (radius < EPSILON)
				ret = block.blockIntersectionWithRotatingPoint<ADD_XY>(interval.P1, interval.P2);
			else {
				ret = block.blockIntersectionWithMovingPoint<ADD_XY>(radius, interval.P1, interval.P2);
			}
			geometry::IntervalList copyRet(ret); ret.clear();
			geometry::intervalInverseRadians(copyRet, interval, ret);
			copyRet = ret; ret.clear();
			geometry::intervalsToOriginalInterval(copyRet, interval, ret);
		}
	};

	std::ostream& operator<<(std::ostream &os, const Fan& f);

	/*
	The shape of a moving stud consists of a tract and two circles at the ends (end circles).
	*/
	struct MovingStud {
		double radius;
		geometry::RadianInterval interval;

		/*MovingStud::*/MovingStud();
		/*MovingStud::*/MovingStud(const MovingStud &f);
		/*MovingStud::*/MovingStud(double radius, double min, double max);

	private:
		bool /*MovingStud::*/tractIntersectsLineSegment(const geometry::LineSegment &l) const;
		geometry::Point /*MovingStud::*/minPoint() const;
		geometry::Point /*MovingStud::*/maxPoint() const;
	public:
		bool /*MovingStud::*/intersectsStud(const geometry::Point &stud) const;

		/*
		The moving stud intersects a block if it intersects one of the sides (line segments), or if an end circle intersects the brick.
		*/
		template <int ADD_XY>
		bool /*MovingStud::*/intersectsBox(const Brick &box) const {
			// Check tract:
			geometry::LineSegment segments[4];
			box.getBoxLineSegments<ADD_XY, 0>(segments);
			for (int i = 0; i < 4; ++i) {
				if (tractIntersectsLineSegment(segments[i]))
					return true;
			}
			// Check end circles:
			geometry::Point endPoint1 = minPoint();
			box.movePointSoThisIsAxisAlignedAtOrigin(endPoint1);
			if (box.boxIntersectsInnerStud<ADD_XY>(endPoint1))
				return true;
			geometry::Point endPoint2 = maxPoint();
			box.movePointSoThisIsAxisAlignedAtOrigin(endPoint2);
			return box.boxIntersectsInnerStud<ADD_XY>(endPoint2);
		}

		template <int ADD_XY>
		void /*MovingStud::*/allowableAnglesForBlock(const Brick &block, bool allowClick, geometry::IntervalList &ret, std::vector<ClickInfo> &clicks) const {
#ifdef _TRACE
			std::cout << " " << block << " VS MS r=" << radius << ", [" << interval << "], AC=" << allowClick << " STARTED " << std::endl;
#endif
			// Compute intersections without interval information:
			if (radius < SNAP_DISTANCE) {
#ifdef _TRACE
				std::cout << "  R=0, SO FIND QUICK!" << std::endl;
#endif
				ret = block.blockIntersectionWithRotatingStud<ADD_XY>(interval.P1, interval.P2, allowClick);
			}
			else
				ret = block.blockIntersectionWithMovingStud<ADD_XY>(radius, interval.P1, interval.P2);

			// Find intersect between min/max and intersection points:
			// Inverse ret:
			geometry::IntervalList copyRet(ret); ret.clear();
			geometry::intervalInverseRadians(copyRet, interval, ret);
			// Transform ret to interval [-MAX_ANGLE_RADIANS;MAX_ANGLE_RADIANS[
			copyRet = ret; ret.clear();
			geometry::intervalsToOriginalInterval(copyRet, interval, ret);

			if (!allowClick || radius <= SNAP_DISTANCE) {
#ifdef _TRACE
				if (!allowClick)
					std::cout << "  Clicking skipped" << std::endl;
				if (radius <= SNAP_DISTANCE)
					std::cout << "  Click skipped because " << radius << " <= " << SNAP_DISTANCE << std::endl;
#endif
				return;
			}

			// Add intervals for studs:
			std::vector<ClickInfo> foundClicks; // angle, dist
			block.getStudIntersectionWithMovingStud(radius, interval.P1, interval.P2, foundClicks);
			for (std::vector<ClickInfo>::const_iterator it = foundClicks.begin(); it != foundClicks.end(); ++it) {
				double studAngle = it->first;
				double studDist = it->second;
#ifdef _TRACE
				std::cout << "  Adding stud at angle " << studAngle << ", in original interval: " << geometry::angleToOriginalInterval(studAngle, interval) << std::endl;
#endif
				double studAngleTransformed = geometry::angleToOriginalInterval(studAngle, interval);

				const double b = radius;
				const double c = studDist;
				double A = acos((b*b + c*c - SNAP_DISTANCE*SNAP_DISTANCE) / (2 * b*c)); // Cosine rule
																						// Ensure an even results interval:
				if (studAngleTransformed + A > MAX_ANGLE_RADIANS)
					A = MAX_ANGLE_RADIANS - studAngleTransformed;
				else if (studAngleTransformed - A < -MAX_ANGLE_RADIANS)
					A = MAX_ANGLE_RADIANS + studAngleTransformed;

				clicks.push_back(ClickInfo(studAngleTransformed, A));
			}
		}
	};

	std::ostream& operator<<(std::ostream &os, const MovingStud& ms);

	struct TurningSingleBrick {
		Brick blocks[2];
		Brick blockAbove;
		Fan fans[6];
		MovingStud movingStuds[NUMBER_OF_STUDS];
		geometry::Point studTranslation;

		void createBricksAndStudTranslation(const Model &model, const IConnectionPair &connectionPair, const RectilinearBrick &b);
		void createMovingStuds();

		template <int ADD_XY>
		void /*TurningSingleBrick::*/createFans() {
			geometry::Point pois1[NUMBER_OF_POIS_FOR_BOX_INTERSECTION];
			blocks[0].getBoxPOIs<ADD_XY>(pois1);
			geometry::Point pois2[NUMBER_OF_POIS_FOR_BOX_INTERSECTION];
			blocks[1].getBoxPOIs<ADD_XY>(pois2);
			for (int i = 0; i < 6; ++i) {
				double minAngle = geometry::angleOfPoint(pois1[i]);
				double maxAngle = geometry::angleOfPoint(pois2[i]);
				double radius = geometry::norm(pois1[i]);
				assert(geometry::abs(geometry::norm(pois2[i]) - radius) < EPSILON);
				fans[i] = Fan(radius, minAngle, maxAngle);
			}
		}

		template <int ADD_XY>
		void /*TurningSingleBrick::*/allowableAnglesForBrickTurningBelow(const Brick &brick, geometry::IntervalList &l, std::vector<ClickInfo> &clicks) const {
			l.push_back(geometry::Interval(-MAX_ANGLE_RADIANS, MAX_ANGLE_RADIANS));
			// Turning below:
			for (int i = 0; i < NUMBER_OF_STUDS; ++i) {
				geometry::IntervalList listForStud;
				movingStuds[i].allowableAnglesForBlock<ADD_XY>(brick, i >= 4, listForStud, clicks); // The last 4 studs are outer and thus allowing clicking.
				geometry::IntervalList copyL(l); l.clear();
				geometry::intervalAnd(copyL, listForStud, l);
				if (l.empty())
					break;
			}
#ifdef _TRACE
			std::cout << "AAFB BELOW returns " << l << std::endl;
#endif
		}

		template <int ADD_XY>
		void /*TurningSingleBrick::*/allowableAnglesForBrickTurningAbove(const Brick &brick, geometry::IntervalList &l, std::vector<ClickInfo> &clicks) const {
			assert(l.empty());
			l.push_back(geometry::Interval(-MAX_ANGLE_RADIANS, MAX_ANGLE_RADIANS));

			geometry::Point studsOfBrick[NUMBER_OF_STUDS];
			brick.getStudPositions(studsOfBrick);
			// Create new moving studs for the brick and check those:
			for (int i = 0; i < NUMBER_OF_STUDS; ++i) {
				// Create moving stud:
				double midAngle = geometry::angleOfPoint(studsOfBrick[i]);
				double minAngle = midAngle - MAX_ANGLE_RADIANS;
				if (minAngle < -M_PI)
					minAngle += 2 * M_PI;
				double maxAngle = midAngle + MAX_ANGLE_RADIANS;
				if (maxAngle > M_PI)
					maxAngle -= 2 * M_PI;
				MovingStud ms(geometry::norm(studsOfBrick[i]), minAngle, maxAngle);

				geometry::IntervalList listForStud;
				std::vector<ClickInfo> reversedClicks;
				ms.allowableAnglesForBlock<ADD_XY>(blockAbove, i >= 4, listForStud, reversedClicks); // The last 4 studs are outer and thus allowing clicking.
				for (std::vector<ClickInfo>::const_iterator it = reversedClicks.begin(); it != reversedClicks.end(); ++it) {
					clicks.push_back(ClickInfo(-(it->first), it->second));
				}
				geometry::IntervalList copyL(l); l.clear();
				geometry::intervalAnd(copyL, listForStud, l);
			}
			geometry::intervalReverse(l);
#ifdef _TRACE
			std::cout << "AAFB ABOVE returns " << l << std::endl;
#endif
		}

		template <int ADD_XY>
		void /*TurningSingleBrick::*/allowableAnglesForBrickTurningAtSameLevel(const Brick &brick, geometry::IntervalList &l) const {
			assert(l.empty());
#ifdef _TRACE
			std::cout << "AAFBTSL brick " << brick << " VS TSB " << blocks[0] << "-->" << blocks[1] << std::endl;
#endif
			l.push_back(geometry::Interval(-MAX_ANGLE_RADIANS, MAX_ANGLE_RADIANS)); // Initially all angles allowed.

																					// First test own fans against the brick:
			for (int i = 0; i < 6; ++i) {
				geometry::IntervalList listForFan;
				fans[i].allowableAnglesForBlock<ADD_XY>(brick, listForFan);
				geometry::IntervalList copyL(l); l.clear();
				geometry::intervalAnd(copyL, listForFan, l);
				if (l.empty())
					return;
			}

			// Secondly test fans of other brick against this:
			geometry::Point pois[NUMBER_OF_POIS_FOR_BOX_INTERSECTION];
			brick.getBoxPOIs<ADD_XY>(pois);
			pois[4] = brick.center;
			// Create new moving studs for the brick and check those:
			for (int i = 0; i < 6; ++i) {
				// Create moving stud:
				double midAngle = geometry::angleOfPoint(pois[i]);
				double minAngle = midAngle - MAX_ANGLE_RADIANS;
				if (minAngle < -M_PI)
					minAngle += 2 * M_PI;
				double maxAngle = midAngle + MAX_ANGLE_RADIANS;
				if (maxAngle > M_PI)
					maxAngle -= 2 * M_PI;
				Fan f(geometry::norm(pois[i]), minAngle, maxAngle);

				geometry::IntervalList listForFan;
				f.allowableAnglesForBlock<ADD_XY>(blockAbove, listForFan);

#ifdef _TRACE
				if (!geometry::intervalEquals(l, geometry::intervalAnd(l, listForFan))) {
					std::cout << " AAFBTSL REVERSE fan reduction: " << f << ", " << blockAbove << ": " << listForFan << std::endl;
				}
#endif
				geometry::intervalReverse(listForFan);
				geometry::IntervalList copyL(l); l.clear();
				geometry::intervalAnd(copyL, listForFan, l);
				if (l.empty())
					return;
			}
#ifdef _TRACE
			std::cout << "AAFBTSL returns " << l << std::endl;
#endif
		}

		template <int ADD_XY>
		void /*TurningSingleBrick::*/allowableAnglesForBrick(const Brick &brick, geometry::IntervalList &l, std::vector<ClickInfo> &clicks) const {
			assert(l.empty());
#ifdef _TRACE
			std::cout << "AAFB (allowableAnglesForBrick) " << brick << " STARTING!" << std::endl;
#endif
			int8_t level = blocks[0].level;
			if (brick.level == level) {
				allowableAnglesForBrickTurningAtSameLevel<ADD_XY>(brick, l);
			}
			else if (level + 1 == brick.level) {
				allowableAnglesForBrickTurningBelow<ADD_XY>(brick, l, clicks);
			}
			else if (level - 1 == brick.level) {
				allowableAnglesForBrickTurningAbove<ADD_XY>(brick, l, clicks);
			}
			else {
				l.push_back(geometry::Interval(-MAX_ANGLE_RADIANS, MAX_ANGLE_RADIANS));
			}
		}

		template <int ADD_XY>
		bool /*TurningSingleBrick::*/intersectsBrick(const Brick &brick) const {
			int8_t level = blocks[0].level;
			if (brick.level == level) {
				// Same level:
				for (int i = 0; i < 2; ++i) {
					if (blocks[i].boxesIntersect<ADD_XY>(brick)) {
						return true;
					}
				}
				for (int i = 0; i < 4; ++i) {
					if (fans[i].intersectsBox<ADD_XY>(brick)) {
						return true;
					}
				}
			}
			else if (level + 1 == brick.level) {
				// Turning below:
				for (int i = 0; i < NUMBER_OF_STUDS; ++i) {
					if (movingStuds[i].intersectsBox<ADD_XY>(brick)) {
						return true;
					}
				}
			}
			else if (level - 1 == brick.level) {
				// Turning above:
				geometry::Point studsOfB[NUMBER_OF_STUDS];
				brick.getStudPositions(studsOfB);
				for (int j = 0; j < NUMBER_OF_STUDS; ++j) {
					geometry::Point &stud = studsOfB[j];

					for (int i = 0; i < 2; ++i) {
						geometry::Point pTranslated(stud);
						blocks[i].movePointSoThisIsAxisAlignedAtOrigin(pTranslated);
						if (blocks[i].boxIntersectsInnerStud<ADD_XY>(pTranslated)) {
							return true;
						}
					}
					for (int i = 0; i < 4; ++i) {
						if (fans[i].intersectsStud(stud)) {
							return true;
						}
					}
				}
			}
			return false;
		}
	};

	struct TurningBlockInvestigator {
		Model baseModel;
		IConnectionPair connectionPair;
		FatBlock block;

		/*TurningBlockInvestigator::*/TurningBlockInvestigator() {}
		/*TurningBlockInvestigator::*/TurningBlockInvestigator(const TurningBlockInvestigator &b) : baseModel(b.baseModel), connectionPair(b.connectionPair), block(b.block) {}
		/*TurningBlockInvestigator::*/TurningBlockInvestigator(const Model &baseModel, const FatBlock &block, int modelBlockI, const IConnectionPair &connectionPair) : baseModel(baseModel), connectionPair(connectionPair), block(block) {
			if (modelBlockI == connectionPair.P1.first.modelBlockI) {
				std::swap(this->connectionPair.P1, this->connectionPair.P2);
			}
		}

		template <int ADD_XY>
		void /*TurningBlockInvestigator::*/allowableAnglesForBricks(const util::TinyVector<int, 5> &possibleCollisions, geometry::IntervalList &l) const {
			geometry::IntervalList ret;
			ret.push_back(geometry::Interval(-MAX_ANGLE_RADIANS, MAX_ANGLE_RADIANS));

			RectilinearBrick b;
			for (int i = 0; i < block.size; b = block.otherBricks[i++]) {
#ifdef _TRACE
				std::cout << "----------------- INITIATING ALLOWABLE ANGLES <" << ADD_XY << "> ----------------------" << std::endl;
#endif
				// Create TurningSingleBrick:
				TurningSingleBrick tsb;
				tsb.createBricksAndStudTranslation(baseModel, connectionPair, b);
				tsb.createFans<ADD_XY>();
				tsb.createMovingStuds();

				// Check all possible collision bricks:
				for (const int* it = possibleCollisions.begin(); it != possibleCollisions.end(); ++it) {
					Brick brick = baseModel.bricks[*it].b;
					brick.center.X -= tsb.studTranslation.X;
					brick.center.Y -= tsb.studTranslation.Y;

					geometry::IntervalList joiner;
					std::vector<ClickInfo> clicks;
					tsb.allowableAnglesForBrick<ADD_XY>(brick, joiner, clicks);
#ifdef _TRACE
					std::cout << "Joining allowable angles for " << brick << ": " << ret << " & " << joiner << " = " << geometry::intervalAnd(ret, joiner) << std::endl;
#endif
					geometry::IntervalList copyRet(ret); ret.clear();
					geometry::intervalAnd(copyRet, joiner, ret);
					// Add clicks by investigating realizable on each info.
					for (std::vector<ClickInfo>::const_iterator it2 = clicks.begin(); it2 != clicks.end(); ++it2) {
#ifdef _TRACE
						std::cout << " Checking click, angle=" << it2->first << ", dist=" << it2->second << std::endl;
#endif
						Model c2(baseModel);
						StepAngle stepAngle(it2->first);
						AngledConnection connection(connectionPair, stepAngle);
						c2.add(block, connectionPair.P2.first.modelBlockI, connection);
						if (c2.isRealizable<ADD_XY>(possibleCollisions, block.size)) {
							const double angleOfSnapRadius = it2->second;
#ifdef _TRACE
							std::cout << "  Adding stud interval " << Interval(it2->first - angleOfSnapRadius, it2->first + angleOfSnapRadius) << std::endl;
#endif
							geometry::IntervalList studInterval;
							studInterval.push_back(geometry::Interval(it2->first - angleOfSnapRadius, it2->first + angleOfSnapRadius));
#ifdef _TRACE
							std::cout << " AAFB RET " << ret << " | " << studInterval << " = " << geometry::intervalOr(ret, studInterval) << std::endl;
#endif

							geometry::IntervalList copyRet2(ret); ret.clear();
							geometry::intervalOr(copyRet2, studInterval, ret);
						}
#ifdef _TRACE
						else {
							std::cout << " NOT REALIZABLE: " << c2 << std::endl;
							MPDPrinter h;
							h.add("Not realizable", &c2);
							h.print("unrealizable");
						}
#endif
					}
				}
			}
			l = ret;
#ifdef _TRACE
			std::cout << "TSB Investigator returns " << ret << std::endl;
			std::cout << "----------------- ENDING ALLOWABLE ANGLES <" << ADD_XY << "> ----------------------" << std::endl;
#endif
		}

		/*
		The TSB is clear form the bricks indicated in possibleCollisions if none of these bricks intersect the TSB.
		*/
		template <int ADD_XY>
		bool /*TurningBlockInvestigator::*/isClear(const util::TinyVector<int, 5> &possibleCollisions) const {
			RectilinearBrick b;
			for (int i = 0; i < block.size; b = block.otherBricks[i++]) {
				// Create TurningSingleBrick:
				TurningSingleBrick tsb;
				tsb.createBricksAndStudTranslation(baseModel, connectionPair, b);
				tsb.createFans<ADD_XY>();
				tsb.createMovingStuds();

				// Check all possible collision bricks:
				for (const int* it = possibleCollisions.begin(); it != possibleCollisions.end(); ++it) {
					Brick brick = baseModel.bricks[*it].b;
					brick.center.X -= tsb.studTranslation.X;
					brick.center.Y -= tsb.studTranslation.Y;

					if (tsb.intersectsBrick<ADD_XY>(brick)) {
						return false;
					}
				}
			}
			return true;
		}
	};
}

#endif // MODELLING_TURNING_SINGLE_BRICK_H
