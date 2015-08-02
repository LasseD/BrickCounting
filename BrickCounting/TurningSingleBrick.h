#ifndef TURNING_SINGLE_BRICK_H
#define TURNING_SINGLE_BRICK_H

#include "Math.h"
#include "Brick.h"
#include "Configuration.hpp"
#include "UnionFind.h"
#include <vector>

typedef std::pair<double,double> ClickInfo; // angle, dist of stud making the click.

namespace math {
  double angleToOriginalInterval(double a, const RadianInterval &interval);
  IntervalList intervalsToOriginalInterval(const IntervalList &l, const RadianInterval &interval);
}

/*
Fan: A "pizza slice" of a circle. Centered at 0,0.
Angles: There is always a continuous interval between min and max, and min < max.
Angle: 0 is horizontal ->
*/
struct Fan {
  double radius;
  RadianInterval interval;

  /*Fan::*/Fan();
  /*Fan::*/Fan(const Fan &f);
  /*Fan::*/Fan(double radius, double min, double max);
  /*Fan::*/Fan(double radius, const RadianInterval &interval);

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

  template <int ADD_XY>
  void /*Fan::*/allowableAnglesForBlock(const Brick &block, IntervalList &ret) const {
    // Compute intersections without interval information:
    if(radius < EPSILON)
      ret = block.blockIntersectionWithRotatingPoint<ADD_XY>(interval.P1, interval.P2);
    else {
      ret = block.blockIntersectionWithMovingPoint<ADD_XY>(radius, interval.P1, interval.P2);
    }
#ifdef _TRACE
    if(!ret.empty()) {
      std::cout << "  FAN intersection with block " << block << ": " << ret << std::endl;
      std::cout << "  FAN inverse: " << math::intervalInverseRadians(ret, interval) << std::endl;
      std::cout << "  FAN orig interval: " << math::intervalsToOriginalInterval(math::intervalInverseRadians(ret, interval), interval) << std::endl;
    }
#endif
    ret = math::intervalInverseRadians(ret, interval);
    ret = math::intervalsToOriginalInterval(ret, interval);
  }
};

std::ostream& operator<<(std::ostream &os, const Fan& f);

/*
The shape of a moving stud consists of a tract and two circles at the ends (end circles).
*/
struct MovingStud {
  double radius;
  RadianInterval interval;

  /*MovingStud::*/MovingStud();
  /*MovingStud::*/MovingStud(const MovingStud &f);
  /*MovingStud::*/MovingStud(double radius, double min, double max);

private:
  bool /*MovingStud::*/tractIntersectsLineSegment(const LineSegment &l) const;
  Point /*MovingStud::*/minPoint() const;
  Point /*MovingStud::*/maxPoint() const;
public:
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
  void /*MovingStud::*/allowableAnglesForBlock(const Brick &block, bool allowClick, IntervalList &ret, std::vector<ClickInfo> &clicks) const {
#ifdef _TRACE
    std::cout << " " << block << " VS MS r=" << radius << ", [" << interval << "], AC=" << allowClick << " STARTED " << std::endl;
#endif
    // Compute intersections without interval information:
    if(radius < SNAP_DISTANCE) {
#ifdef _TRACE
      std::cout << "  R=0, SO FIND QUICK!" << std::endl;
#endif
      ret = block.blockIntersectionWithRotatingStud<ADD_XY>(interval.P1, interval.P2, allowClick);
    }
    else
      ret = block.blockIntersectionWithMovingStud<ADD_XY>(radius, interval.P1, interval.P2);
#ifdef _TRACE
    if(!ret.empty()) {
      std::cout << "  " << block << ":" << std::endl << "  Intersection: " << ret << std::endl;
      std::cout << "  Inversed: " << math::intervalInverseRadians(ret, interval) << std::endl;
      std::cout << "  Transformed: " << math::intervalsToOriginalInterval(math::intervalInverseRadians(ret, interval), interval) << std::endl;
    }
#endif

    // Find intersect between min/max and intersection points:
    // Inverse ret:
    ret = math::intervalInverseRadians(ret, interval);
    // Transform ret to interval [-MAX_ANGLE_RADIANS;MAX_ANGLE_RADIANS[
    ret = math::intervalsToOriginalInterval(ret, interval);

    if(!allowClick || radius < EPSILON)
      return;

    // Add intervals for studs:
    double studAngle;
    bool anyStudAngles = block.getStudIntersectionWithMovingStud(radius, interval.P1, interval.P2, studAngle);
    if(!anyStudAngles)
      return;
#ifdef _TRACE
    std::cout << "  Adding stud at angle " << studAngle << ", in original interval: " << math::angleToOriginalInterval(studAngle, interval) << std::endl;
#endif
    double studAngleTransformed = math::angleToOriginalInterval(studAngle, interval);
    clicks.push_back(ClickInfo(studAngleTransformed, radius));
    return;
  }
};

std::ostream& operator<<(std::ostream &os, const MovingStud& ms);

struct TurningSingleBrick {
  Brick blocks[2];
  Brick blockAbove;
  Fan fans[4];
  MovingStud movingStuds[NUMBER_OF_STUDS];
  Point studTranslation;

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
  void /*TurningSingleBrick::*/allowableAnglesForBrickTurningBelow(const Brick &brick, IntervalList &l, std::vector<ClickInfo> &clicks) const {
    IntervalList ret;
    ret.push_back(Interval(-MAX_ANGLE_RADIANS,MAX_ANGLE_RADIANS));
    // Turning below:
    for(int i = 0; i < NUMBER_OF_STUDS; ++i) {
      IntervalList listForStud;
      movingStuds[i].allowableAnglesForBlock<ADD_XY>(brick, i >= 4, listForStud, clicks); // The last 4 studs are outer and thus allowing clicking.
#ifdef _TRACE
      std::cout << "||| AAFB& " << movingStuds[i] << ": " << std::endl << "  " << ret << " & " << listForStud << " = " << std::endl << "  " <<  math::intervalAnd(ret, listForStud) << "|||" << std::endl;
      if(!math::intervalEquals(math::intervalAnd(ret, listForStud), ret))
        std::cout << "AAFB REDUCTION PERFORMED!" << std::endl;
#endif
      ret = math::intervalAnd(ret, listForStud);
      if(ret.empty())
        break;
    }
    l = ret;//math::intervalReverse(ret);
#ifdef _TRACE
    std::cout << "AAFB BELOW returns " << l << std::endl;
#endif
  }

  template <int ADD_XY>
  void /*TurningSingleBrick::*/allowableAnglesForBrickTurningAbove(const Brick &brick, IntervalList &l, std::vector<ClickInfo> &clicks) const {
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

      IntervalList listForStud;
      std::vector<ClickInfo> reversedClicks;
      ms.allowableAnglesForBlock<ADD_XY>(blockAbove, i >= 4, listForStud, reversedClicks); // The last 4 studs are outer and thus allowing clicking.
      for(std::vector<ClickInfo>::const_iterator it = reversedClicks.begin(); it != reversedClicks.end(); ++it) {
        clicks.push_back(ClickInfo(-(it->first), it->second));
      }
      ret = math::intervalAnd(ret, listForStud);
    }
    l = math::intervalReverse(ret);
#ifdef _TRACE
    std::cout << "AAFB ABOVE returns " << l << std::endl;
#endif
  }

  template <int ADD_XY>
  void /*TurningSingleBrick::*/allowableAnglesForBrickTurningAtSameLevel(const Brick &brick, IntervalList &l) const {
#ifdef _TRACE
    std::cout << "AAFBTSL brick " << brick << " VS TSB " << blocks[0] << "-->" << blocks[1] << std::endl;
#endif
    l.push_back(Interval(-MAX_ANGLE_RADIANS,MAX_ANGLE_RADIANS)); // Initially all angles allowed.

    // First test own fans against the brick:
    for(int i = 0; i < 4; ++i) {
      IntervalList listForFan;
      fans[i].allowableAnglesForBlock<ADD_XY>(brick, listForFan);
#ifdef _TRACE
      if(!math::intervalEquals(l, math::intervalAnd(l, math::intervalReverse(listForFan))))
        std::cout << " AAFBTSL fan reduction: " << fans[i] << ", " << brick << ": " << listForFan << std::endl;
#endif
      l = math::intervalAnd(l, listForFan);
      if(l.empty())
        return;
    }

    // Secondly test fans of other brick against this:
    Point pois[NUMBER_OF_POIS_FOR_BOX_INTERSECTION];
    brick.getBoxPOIs<ADD_XY>(pois);
    // Create new moving studs for the brick and check those:
    for(int i = 0; i < 4; ++i) {
      // Create moving stud:
      double midAngle = math::angleOfPoint(pois[i]);
      double minAngle = midAngle - MAX_ANGLE_RADIANS;
      if(minAngle < -M_PI)
        minAngle += 2*M_PI;
      double maxAngle = midAngle + MAX_ANGLE_RADIANS;
      if(maxAngle > M_PI)
        maxAngle -= 2*M_PI;
      Fan f(math::norm(pois[i]), minAngle, maxAngle);

      IntervalList listForFan;
      f.allowableAnglesForBlock<ADD_XY>(blockAbove, listForFan);
      
#ifdef _TRACE
      if(!math::intervalEquals(l, math::intervalAnd(l, listForFan))) {
        std::cout << " AAFBTSL REVERSE fan reduction: " << f << ", " << blockAbove << ": " << listForFan << std::endl;
        std::cout << " AAFBTSL REVERSE fan reduction reversed: " << math::intervalReverse(listForFan) << std::endl;
      }
#endif
      listForFan = math::intervalReverse(listForFan);
      l = math::intervalAnd(l, listForFan);
      if(l.empty())
        return;
    }
#ifdef _TRACE
    std::cout << "AAFBTSL returns " << l << std::endl;
#endif
  }

  template <int ADD_XY>
  void /*TurningSingleBrick::*/allowableAnglesForBrick(const Brick &brick, IntervalList &l, std::vector<ClickInfo> &clicks) const {
#ifdef _TRACE
    std::cout << "AAFB (allowableAnglesForBrick) " << brick << " STARTING!" << std::endl;
#endif
    int8_t level = blocks[0].level;
    if(brick.level == level) {
      allowableAnglesForBrickTurningAtSameLevel<ADD_XY>(brick, l);
    }
    else if(level + 1 == brick.level) {
      allowableAnglesForBrickTurningBelow<ADD_XY>(brick, l, clicks);
    }
    else if(level - 1 == brick.level) {
      allowableAnglesForBrickTurningAbove<ADD_XY>(brick, l, clicks);
    }
    else {
      l.push_back(Interval(-MAX_ANGLE_RADIANS,MAX_ANGLE_RADIANS));
    }
  }

  template <int ADD_XY>
  bool /*TurningSingleBrick::*/intersectsBrick(const Brick &brick) const {
    int8_t level = blocks[0].level;
    if(brick.level == level) {
      // Same level:
      for(int i = 0; i < 2; ++i) {
        if(blocks[i].boxesIntersect<ADD_XY>(brick)) {
          return true;
        }
      }
      for(int i = 0; i < 4; ++i) {
        if(fans[i].intersectsBlock<ADD_XY>(brick)) {
          return true;
        }
      }
    }
    else if(level + 1 == brick.level) {
      // Turning below:
      for(int i = 0; i < NUMBER_OF_STUDS; ++i) {
        if(movingStuds[i].intersectsBlock<ADD_XY>(brick)) {
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
            return true;
          }
        }
        for(int i = 0; i < 4; ++i) {
          if(fans[i].intersectsStud(stud)) {
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
  void /*TurningSingleBrickInvestigator::*/allowableAnglesForBricks(const std::vector<int> &possibleCollisions, IntervalList &l) const {
#ifdef _TRACE
    std::cout << "----------------- INITIATING ALLOWABLE ANGLES <" << ADD_XY <<  "> ----------------------" << std::endl;
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
      std::vector<ClickInfo> clicks;
      tsb.allowableAnglesForBrick<ADD_XY>(b, joiner, clicks);
#ifdef _TRACE
      std::cout << "Joining allowable angles for " << b << ": " << ret << " & " << joiner << " = " << math::intervalAnd(ret,joiner) << std::endl;
#endif
      ret = math::intervalAnd(ret, joiner);
      // Add clicks by investigating realizable on each info.
      for(std::vector<ClickInfo>::const_iterator it2 = clicks.begin(); it2 != clicks.end(); ++it2) {
#ifdef _TRACE
        std::cout << " Checking click, angle=" << it2->first << ", dist=" << it2->second << std::endl;
#endif
        if(it2->second <= SNAP_DISTANCE) {
#ifdef _TRACE
          std::cout << "  Ignoring local (dist < " << SNAP_DISTANCE << ")" << std::endl;
#endif
          continue; // Clicking locally at all angles - do nothing.
        }
        Configuration c2(configuration);
        FatSCC scc; // Initializes for single brick SCC.
        StepAngle stepAngle((short)math::round(it2->first*10000/MAX_ANGLE_RADIANS), 10000);
        Connection connection(connectionPair, stepAngle);
        c2.add(scc, connectionPair.P2.first.configurationSCCI, connection);
        if(c2.isRealizable<ADD_XY>(possibleCollisions, 1)) {
          double angleOfSnapRadius = atan(SNAP_DISTANCE/it2->second)/2;
#ifdef _TRACE
          std::cout << "  Adding stud interval " << Interval(it2->first-angleOfSnapRadius, it2->first+angleOfSnapRadius) << std::endl;
#endif
          IntervalList studInterval;
          studInterval.push_back(Interval(it2->first-angleOfSnapRadius, it2->first+angleOfSnapRadius));
#ifdef _TRACE
          std::cout << " AAFB RET " << ret << " | " << studInterval << " = " << math::intervalOr(ret, studInterval) << std::endl;
#endif
          ret = math::intervalOr(ret, studInterval);
        }
#ifdef _TRACE
        else {
          std::cout << " NOT REALIZABLE: " << c2 << std::endl;
          LDRPrinterHandler h;
          h.add(&c2);
          h.print("unrealizable");
        }
#endif
      }
    }
    l = ret;
#ifdef _TRACE
    std::cout << "TSB Investigator returns " << ret << std::endl;
    std::cout << "----------------- ENDING ALLOWABLE ANGLES <" << ADD_XY <<  "> ----------------------" << std::endl;
#endif
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
