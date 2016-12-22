#ifndef UTIL_UNION_FIND_H
#define UTIL_UNION_FIND_H

#include <stdint.h>
#include <vector>

#include "../geometry/BasicGeometry.h"

#define MAX_DIMENSIONS 5

struct MixedPosition {
	unsigned short p[MAX_DIMENSIONS - 1];
	double lastAngle;

	MixedPosition() {}
	MixedPosition(const MixedPosition &o) : lastAngle(o.lastAngle) {
		for (int i = 0; i < MAX_DIMENSIONS - 1; ++i)
			p[i] = o.p[i];
	}
};

namespace util {
	/*
	Basic UF structure with path compression: https://en.wikipedia.org/wiki/Disjoint-set_data_structure
	*/
	struct UnionFindStructure {
		uint32_t size;
		uint32_t *parents, *ranks; // parents, and sizes (ranks) of trees.
		bool rootsComputed;
		std::vector<uint32_t> roots;

		UnionFindStructure(uint32_t size);
		~UnionFindStructure();

		void performUnion(uint32_t a, uint32_t b);
		void performUnion(const geometry::IntervalList &l1, const geometry::IntervalList &l2, uint32_t unionStart1, uint32_t unionStart2);
		uint32_t find(uint32_t a) const;
		void computeRoots();
	};

	class IntervalUnionFind {
	private:
		unsigned int numStepDimensions;
		unsigned short dimensionSizes[MAX_DIMENSIONS - 1]; // "-1" because last dimension is not a step dimension.

		UnionFindStructure *ufs;
		uint32_t *intervalIndicatorToUnion, unionI;
		std::pair<uint32_t, unsigned short> *unionToInterval;
		const geometry::IntervalListVector &M; // ref only, no ownership.

		void buildIntervalIndicatorToUnion();
		void buildUnions(unsigned int positionI, MixedPosition &position);
		uint32_t indicatorIndexOf(const MixedPosition &position) const;

		IntervalUnionFind(); // Undefined
		IntervalUnionFind& operator=(const IntervalUnionFind &); // Undefined
	public:
		IntervalUnionFind(unsigned int numDimensions, unsigned short const * const dimensionSizes, const geometry::IntervalListVector &M);
		~IntervalUnionFind();

		uint32_t getRootForPosition(const MixedPosition rep) const;
		void getRepresentativeOfUnion(unsigned int unionI, MixedPosition &rep) const;
		std::vector<uint32_t>::const_iterator rootsBegin() const;
		std::vector<uint32_t>::const_iterator rootsEnd() const;
	};
}

#endif // UTIL_UNION_FIND_H
