#ifndef GEOMETRY_BASIC_GEOMETRY_H
#define GEOMETRY_BASIC_GEOMETRY_H

#include <utility>
#include <iostream>
#define _USE_MATH_DEFINES
#include <math.h>
#include <assert.h>
#include <stdint.h>

#include "../util/TinyVector.hpp"

#define X first
#define Y second
#define P1 first
#define P2 second

namespace geometry {
	typedef std::pair<double, double> Point;

	typedef std::pair<double, double> Interval;
	typedef Interval RadianInterval; // Counter clockwise from P1 to P2.
	typedef util::TinyVector<Interval, 8> IntervalList;

	typedef std::pair<Point, Point> LineSegment;

	bool inInterval(int8_t min, int8_t max, int8_t a);
	bool distLessThan2(int8_t a, int8_t b);
	unsigned int diff(int8_t a, int8_t b);

	bool eqEpsilon(double a, double b);
	bool eqEpsilon(const Point &p1, const Point &p2);
	double round(double number);
	int signum(double d);
	double abs(double d);
	double normSq(const Point &p);
	double norm(const Point &p);
	double distSq(const Point &p1, const Point &p2);
	double dist(const Point &p1, const Point &p2);
	bool between(double a, double b, double c);
	bool betweenEpsilon(double a, double b, double c);
	bool inRadianInterval(double a, const RadianInterval &interval);
	bool betweenEndPointsOfLineSegmentEpsilon(const Point &a, const Point &b, const Point &c);
	double angleOfPoint(const Point &p);
	/*
	  Normalize an angle to the interval [-PI;PI[ in radians.
	 */
	double normalizeAngle(double a);

	bool intervalEquals(const IntervalList &a, const IntervalList &b);
	bool intervalContains(const IntervalList &a, double d);
	bool isFullInterval(const IntervalList &a, double min, double max);

	void intervalAnd(const IntervalList &a, const IntervalList &b, IntervalList &result);
	void intervalAndRadians(const RadianInterval &a, const RadianInterval &b, IntervalList &result);
	void intervalInverseRadians(const IntervalList &l, const RadianInterval &minmax, IntervalList &result);
	void intervalOr(const IntervalList &a, const IntervalList &b, IntervalList &result);
	void intervalReverse(IntervalList &l);
	void collapseIntervals(IntervalList &l);
	void toIntervalsRadians(const RadianInterval &interval, IntervalList &result);

	bool rightTurn(const Point &lineStart, const Point &lineEnd, const Point &p);

	int findCircleCircleIntersections(const double r, const Point &p, const double pr, Point &i1, Point &i2); // Actual primitive
	bool findCircleCircleIntersections(const double r, const Point &p, const double pr, double &ai1, double &ai2); // Actual primitive
	void findCircleCircleIntersection(double r, const Point &p, double pr, IntervalList &result); // Returns intersections as IntervalList

	int findCircleLineIntersections(double r, const LineSegment &l, Point &i1, Point &i2);
	bool findCircleLineIntersections(double r, const LineSegment &l, double &ai1, double &ai2);
	bool circleCutoutIntersectsLineSegment(double r, const RadianInterval &circleInterval, const LineSegment &l);

	/*
	  The half plane is divided by the line
	 */
	bool findCircleHalfPlaneIntersection(double radius, const LineSegment &line, RadianInterval &intersection);

	typedef std::pair<unsigned long, unsigned short> IntervalIndicator; // location, size

	struct IntervalListVector {
	private:
		Interval *intervals;
		IntervalIndicator *indicators;
		const uint32_t intervalsSize, indicatorSize;
		uint32_t intervalsI;
		IntervalListVector(); // Unused
		IntervalListVector& operator=(const IntervalListVector &); // Unused
	public:
		IntervalListVector(uint32_t indicatorSize, unsigned int maxLoadFactor);
		~IntervalListVector();
		void insert(uint32_t location, const IntervalList &intervalList);
		void insertEmpty(uint32_t location);
		void get(uint32_t location, IntervalList &intervalList) const;
		Interval get(uint32_t location, unsigned short intervalIndex) const;
		uint32_t sizeIndicator() const;
		uint32_t sizeNonEmptyIntervals() const;
		unsigned short intervalSizeForIndicator(uint32_t i) const;
	};

	uint32_t encodeCombinationType(const util::TinyVector<int, 6> &l);
}

std::ostream& operator<<(std::ostream &os, const geometry::Point& p);
std::ostream& operator<<(std::ostream &os, const geometry::IntervalList& p);
std::ostream& operator<<(std::ostream &os, const geometry::LineSegment& l);

#endif // GEOMETRY_BASIC_GEOMETRY_H
