#ifndef TURNING_SINGLE_BRICK_H
#define TURNING_SINGLE_BRICK_H

#include "Math.h"
#include "Brick.h"
#include "Configuration.hpp"
#include "UnionFind.h"
#include <vector>

/*
Plan:
1: Initially simply check if a TurningSingleBrick (TSB) intersects other bricks. If yes, try all.
2: Make a TSB report using a bitvector (bool array).
3: Make a TSB report using list of intervals (this causes union-find enhancements). - move typedef std::pair<double,double> Interval; to UnionFind.h
*/

/*
Fan: A "pizza slice" of a circle. Centered at 0,0.
Angles: There is always a continuous interval between min and max, and min < max.
Angle: 0 is horizontal ->
*/
struct Fan {
  double radius, minAngle, maxAngle;

  /*Fan::*/Fan();
  /*Fan::*/Fan(const Fan &f);
  /*Fan::*/Fan(double radius, double min, double max);

  bool /*Fan::*/intersectsLineSegment(const LineSegment &l) const;    
  bool /*Fan::*/intersectsStud(const Point &stud) const;

  /*
  A fan intersects a block if one of the sides (line segments) of the block intersects the curve.
  */
  template <int ADD_XY>
  bool /*Fan::*/intersectsBlock(const Brick &block) const  {
    LineSegment segments[4];
    block.getBoxLineSegments<ADD_XY,0>(segments);
    for(int i = 0; i < 4; ++i) {
      if(intersectsLineSegment(segments[i])) {
        return true;
      }
    }
    return false;
  }
};

std::ostream& operator<<(std::ostream &os, const Fan& f);

/*
The shape of a moving stud consists of a tract and two circles at the ends (end circles).
*/
struct MovingStud {
  double radius, minAngle, maxAngle;

  /*MovingStud::*/MovingStud();
  /*MovingStud::*/MovingStud(const MovingStud &f);
  /*MovingStud::*/MovingStud(double radius, double min, double max);

private:
  bool /*MovingStud::*/tractIntersectsLineSegment(const LineSegment &l) const;
  Point /*MovingStud::*/minPoint() const;
  Point /*MovingStud::*/maxPoint() const;
public:
  double /*MovingStud::*/angleToOriginalInterval(double a) const;
  IntervalList /*MovingStud::*/intervalsToOriginalInterval(const IntervalList &l) const;
  bool /*MovingStud::*/intersectsStud(const Point &stud) const;

  /*
  The moving stud intersects a block if it intersects one of the sides (line segments), or if an end circle intersects the brick.
  */
  template <int ADD_XY>
  bool /*MovingStud::*/intersectsBlock(const Brick &block) const {
    // Check tract:
    LineSegment segments[4];
    block.getBoxLineSegments<ADD_XY,0>(segments);
    for(int i = 0; i < 4; ++i) {
      if(tractIntersectsLineSegment(segments[i]))
        return true;
    }
    // Check end circles:
    Point endPoint1 = minPoint();
    block.movePointSoThisIsAxisAlignedAtOrigin(endPoint1);
    if(block.boxIntersectsInnerStud<ADD_XY>(endPoint1))
      return true;
    Point endPoint2 = maxPoint();
    block.movePointSoThisIsAxisAlignedAtOrigin(endPoint2);
    return block.boxIntersectsInnerStud<ADD_XY>(endPoint2);
  }

  template <int ADD_XY>
  IntervalList /*MovingStud::*/allowableAnglesForBlock(const Brick &block, bool allowClick) const {
#ifdef _TRACE
    std::cout << " " << block << " VS MS r=" << radius << ", [" << minAngle << ";" << maxAngle << "], AC=" << allowClick << " STARTED " << std::endl;
#endif
    // Compute intersections without interval information:
    IntervalList ret = block.blockIntersectionWithMovingStud<ADD_XY>(radius, minAngle, maxAngle);
#ifdef _TRACE
    std::cout << "  " << block << ":" << std::endl << "  Intersection: " << ret << std::endl;
#endif

    // Find intersect between min/max and intersection points:
    // Inverse ret:
    ret = math::intervalInverseRadians(ret, minAngle, maxAngle);
#ifdef _TRACE
    std::cout << "  Inversed: " << ret << std::endl;
#endif
    // Transform ret to interval [-MAX_ANGLE_RADIANS;MAX_ANGLE_RADIANS[
    ret = intervalsToOriginalInterval(ret);
#ifdef _TRACE
    std::cout << "  Transformed: " << ret << std::endl;
#endif

    if(!allowClick)
      return ret;

    if(radius < EPSILON) {
      // If the MS has radius=0, then it can turn anywhere in case of click:
      if(block.outerStudIntersectsStudAtOrigin()) {
        return math::toIntervalsRadians(minAngle, maxAngle);
      }
    }

    // Add intervals for studs:
    std::vector<double> studAngles;
    block.getStudIntersectionsWithMovingStud(radius, minAngle, maxAngle, studAngles);
    std::vector<double> studAnglesTransformed;
    for(std::vector<double>::const_iterator it = studAngles.begin(); it != studAngles.end(); ++it) {
#ifdef _TRACE
      std::cout << "  Adding stud at angle " << *it << ", in original interval: " << angleToOriginalInterval(*it) << std::endl;
#endif
      studAnglesTransformed.push_back(angleToOriginalInterval(*it));
    }
    std::sort(studAnglesTransformed.begin(), studAnglesTransformed.end());
    IntervalList studIntervals;
    double angleOfSnapRadius = atan(SNAP_DISTANCE/radius);
    for(std::vector<double>::const_iterator it = studAnglesTransformed.begin(); it != studAnglesTransformed.end(); ++it) {
#ifdef _TRACE
      std::cout << "  Adding stud interval " << Interval(*it-angleOfSnapRadius, *it+angleOfSnapRadius) << std::endl;
#endif
      studIntervals.push_back(Interval(*it-angleOfSnapRadius, *it+angleOfSnapRadius));
    }

#ifdef _TRACE
    std::cout << " AAFB RET " << ret << " | " << studIntervals << " = " << math::intervalOr(ret, studIntervals) << std::endl;
#endif
    return math::intervalOr(ret, studIntervals);
  }
};

std::ostream& operator<<(std::ostream &os, const MovingStud& ms);

struct TurningSingleBrick {
  Brick blocks[2];
  Brick blockAbove;
  Fan fans[4];
  MovingStud movingStuds[NUMBER_OF_STUDS];
  Point studTranslation;
  //MovingStud movingAntiStuds[4];

  void createBricksAndStudTranslation(const Configuration &configuration, const IConnectionPair &connectionPair);
  void createMovingStuds();

  template <int ADD_XY>
  void /*TurningSingleBrick::*/createFans() {
    Point pois1[NUMBER_OF_POIS_FOR_BOX_INTERSECTION];
    blocks[0].getBoxPOIs<ADD_XY>(pois1);
    Point pois2[NUMBER_OF_POIS_FOR_BOX_INTERSECTION];
    blocks[1].getBoxPOIs<ADD_XY>(pois2);
    for(int i = 0; i < 4; ++i) {
      double minAngle = math::angleOfPoint(pois1[i]);
      double maxAngle = math::angleOfPoint(pois2[i]);
      double radius = math::norm(pois1[i]);
      assert(math::abs(math::norm(pois2[i]) - radius) < EPSILON);
      fans[i] = Fan(radius, minAngle, maxAngle);
    }
  }

  template <int ADD_XY>
  void /*TurningSingleBrick::*/allowableAnglesForBrickTurningAbove(const Brick &brick, IntervalList &l) const {
    IntervalList ret;
    ret.push_back(Interval(-MAX_ANGLE_RADIANS,MAX_ANGLE_RADIANS));

    Point studsOfBrick[NUMBER_OF_STUDS];
    brick.getStudPositions(studsOfBrick);
    // Create new moving studs for the brick and check those:
    for(int i = 0; i < NUMBER_OF_STUDS; ++i) {
      // Create moving stud:
      double midAngle = math::angleOfPoint(studsOfBrick[i]);
      double minAngle = midAngle - MAX_ANGLE_RADIANS;
      if(minAngle < -M_PI)
        minAngle += 2*M_PI;
      double maxAngle = midAngle + MAX_ANGLE_RADIANS;
      if(maxAngle > M_PI)
        maxAngle -= 2*M_PI;
      MovingStud ms(math::norm(studsOfBrick[i]), minAngle, maxAngle);

      IntervalList listForStud = ms.allowableAnglesForBlock<ADD_XY>(blockAbove, i >= 4); // The last 4 studs are outer and thus allowing clicking.
      ret = math::intervalAnd(ret, listForStud);
    }
    l = ret;
  }

  template <int ADD_XY>
  bool /*TurningSingleBrick::*/allowableAnglesForBrick(const Brick &brick, IntervalList &l) const {
#ifdef _TRACE
    std::cout << "AAFB (allowableAnglesForBrick) " << brick << " STARTING!" << std::endl;
#endif
    IntervalList ret;
    ret.push_back(Interval(-MAX_ANGLE_RADIANS,MAX_ANGLE_RADIANS));

    int8_t level = blocks[0].level;
    if(brick.level == level) {
      // Same level:
      // TODO! FIXME! USE 2 * BRICK POIS VS FANS
      return false;
    }
    else if(level + 1 == brick.level) {
      // Turning below:
      for(int i = 0; i < NUMBER_OF_STUDS; ++i) {
        IntervalList listForStud = movingStuds[i].allowableAnglesForBlock<ADD_XY>(brick, i >= 4); // The last 4 studs are outer and thus allowing clicking.
#ifdef _TRACE
        std::cout << "||| AAFB& " << movingStuds[i] << ": " << std::endl << "  " << ret << " & " << listForStud << " = " << std::endl << "  " <<  math::intervalAnd(ret, listForStud) << "|||" << std::endl;
        if(!math::intervalEquals(math::intervalAnd(ret, listForStud), ret))
          std::cout << "AAFB REDUCTION PERFORMED!" << std::endl;
#endif
        ret = math::intervalAnd(ret, listForStud);
        if(ret.empty())
          break;
      }
      l = ret;
      return true;
    }
    else if(level - 1 == brick.level) {
      // Turning above:
      allowableAnglesForBrickTurningAbove<ADD_XY>(brick, l);
      return true;
    }
    l.push_back(Interval(-MAX_ANGLE_RADIANS,MAX_ANGLE_RADIANS));
    return true;
  }

  template <int ADD_XY>
  bool /*TurningSingleBrick::*/intersectsBrick(const Brick &brick) const {
    int8_t level = blocks[0].level;
    if(brick.level == level) {
      // Same level:
      for(int i = 0; i < 2; ++i) {
        if(blocks[i].boxesIntersect<ADD_XY>(brick)) {
#ifdef _TRACE
          std::cout << ":( Same level, block " << i << ": " << blocks[i] << " <> " << brick << std::endl;
#endif
          return true;
        }
      }
      for(int i = 0; i < 4; ++i) {
        if(fans[i].intersectsBlock<ADD_XY>(brick)) {
#ifdef _TRACE
          std::cout << ":( Same level, fan " << i << ": " << fans[i] << " intersects " << brick << std::endl;
#endif
          return true;
        }
      }
    }
    else if(level + 1 == brick.level) {
      // Turning below:
      for(int i = 0; i < NUMBER_OF_STUDS; ++i) {
        if(movingStuds[i].intersectsBlock<ADD_XY>(brick)) {
#ifdef _TRACE
          std::cout << ":( below, moving stud " << i << ": " << movingStuds[i] << " intersects " << brick << std::endl;
#endif
          return true;
        }
      }
    }
    else if(level - 1 == brick.level) {
      // Turning above:
      Point studsOfB[NUMBER_OF_STUDS];
      brick.getStudPositions(studsOfB);
      for(int j = 0; j < NUMBER_OF_STUDS; ++j) {
        Point &stud = studsOfB[j];

        for(int i = 0; i < 2; ++i) {
          Point pTranslated(stud);
          blocks[i].movePointSoThisIsAxisAlignedAtOrigin(pTranslated);
          if(blocks[i].boxIntersectsInnerStud<ADD_XY>(pTranslated)) {
#ifdef _TRACE
            std::cout << ":( above, block " << i << ": " << blocks[i] << " <> " << brick << std::endl;
#endif
            return true;
          }
        }
        for(int i = 0; i < 4; ++i) {
          if(fans[i].intersectsStud(stud)) {
#ifdef _TRACE
            std::cout << "Above, fan " << i << ": " << fans[i] << " intersects " << brick << std::endl;
#endif
            return true;
          }
        }
      }
    }
    return false;
  }
};

struct TurningSingleBrickInvestigator {
  Configuration configuration;
  IConnectionPair connectionPair;

  /*TurningSingleBrickInvestigator::*/TurningSingleBrickInvestigator() {}
  /*TurningSingleBrickInvestigator::*/TurningSingleBrickInvestigator(const TurningSingleBrickInvestigator &b) : configuration(b.configuration), connectionPair(b.connectionPair) {}
  /*TurningSingleBrickInvestigator::*/TurningSingleBrickInvestigator(const Configuration &configuration, int configurationSCCI, const IConnectionPair &connectionPair) : configuration(configuration), connectionPair(connectionPair) {
    if(configurationSCCI == connectionPair.P1.first.configurationSCCI) {
      std::swap(this->connectionPair.P1,this->connectionPair.P2);
    }
    assert(this->connectionPair.P1.first.configurationSCCI <= this->connectionPair.P2.first.configurationSCCI);
  }

  template <int ADD_XY>
  bool /*TurningSingleBrickInvestigator::*/allowableAnglesForBricks(const std::vector<int> &possibleCollisions, IntervalList &l) const {
#ifdef _TRACE
    std::cout << "----------------- INITIATING ALLOWABLE ANGLES ----------------------" << std::endl;
#endif
    // Create TurningSingleBrick:
    TurningSingleBrick tsb;
    tsb.createBricksAndStudTranslation(configuration, connectionPair);
    tsb.createFans<ADD_XY>();
    tsb.createMovingStuds();

    IntervalList ret;
    ret.push_back(Interval(-MAX_ANGLE_RADIANS,MAX_ANGLE_RADIANS));

    // Check all possible collision bricks:
    for(std::vector<int>::const_iterator it = possibleCollisions.begin(); it != possibleCollisions.end(); ++it) {
      Brick b = configuration.bricks[*it].b;
      b.center.X -= tsb.studTranslation.X;
      b.center.Y -= tsb.studTranslation.Y;

      IntervalList joiner;
      if(!tsb.allowableAnglesForBrick<ADD_XY>(b, joiner)) {
#ifdef _TRACE
        std::cout << "----------------- STOPPING ALLOWABLE ANGLES ----------------------" << std::endl;
#endif
        return false;
      }
#ifdef _TRACE
      std::cout << "Joining allowable angles for " << b << ": " << ret << " & " << joiner << " = " << math::intervalAnd(ret,joiner) << std::endl;
#endif
      ret = math::intervalAnd(ret,joiner);
    }
    l = ret;
#ifdef _TRACE
    std::cout << "----------------- ENDING ALLOWABLE ANGLES ----------------------" << std::endl;
#endif
    return true;
  }

  /*
  The TSB is clear form the bricks indicated in possibleCollisions if none of these bricks intersect the TSB. 
  */
  template <int ADD_XY>
  bool /*TurningSingleBrickInvestigator::*/isClear(const std::vector<int> &possibleCollisions) const {
    // Create TurningSingleBrick:
    TurningSingleBrick tsb;
    tsb.createBricksAndStudTranslation(configuration, connectionPair);
    tsb.createFans<ADD_XY>();
    tsb.createMovingStuds();

    // Check all possible collision bricks:
    for(std::vector<int>::const_iterator it = possibleCollisions.begin(); it != possibleCollisions.end(); ++it) {
      Brick b = configuration.bricks[*it].b;
      b.center.X -= tsb.studTranslation.X;
      b.center.Y -= tsb.studTranslation.Y;

      if(tsb.intersectsBrick<ADD_XY>(b)) {
        return false;
      }
    }
    return true;
  }
};

#endif // TURNING_SINGLE_BRICK_H
