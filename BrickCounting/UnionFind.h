#ifndef UNION_FIND_H
#define UNION_FIND_H

#include <stdint.h>
#include <vector>
#include <set>
#include "Math.h"

#define MAX_DIMENSIONS 5

struct MixedPosition {
  unsigned short p[MAX_DIMENSIONS-1];
  double lastAngle;

  MixedPosition() {}
  MixedPosition(const MixedPosition &o) : lastAngle(o.lastAngle) {
    for(int i = 0 ; i < MAX_DIMENSIONS-1; ++i)
      p[i] = o.p[i];
  }
};

namespace UnionFind {
  struct UnionFindStructure {
    uint32_t numUnions;
    bool flattened;
    std::set<uint32_t> *joins; // When join(a,b), b is inserted into joins[a] and a into joins[b].
    uint32_t *minInUnions; // After flattening: Each index references min element in union.
    std::vector<uint32_t> roots; // all distinct minInUnions-indices.

    UnionFindStructure(uint32_t numUnions);
    ~UnionFindStructure();

    void join(uint32_t a, uint32_t b);
    void join(const IntervalList &l1, const IntervalList &l2, uint32_t unionStart1, uint32_t unionStart2);
    uint32_t getMinInUnion(uint32_t a) const;
    void flatten(); // Removes joins-structures. Creates minInUnions-list.
  };

  class IntervalUnionFind {
  private:
    unsigned int numStepDimensions;
    unsigned short dimensionSizes[MAX_DIMENSIONS-1]; // "-1" because last dimension is not a step dimension.

    UnionFindStructure *ufs;
    uint32_t *intervalIndicatorToUnion, unionI;
    std::pair<uint32_t,unsigned int> *unionToInterval;
    const math::IntervalListVector &M; // ref only, no ownership.

    void buildIntervalIndicatorToUnion();
    void buildUnions(unsigned int positionI, MixedPosition &position);
    uint32_t indexOf(const MixedPosition &position) const;
  public:
    IntervalUnionFind(unsigned int numDimensions, unsigned short const * const dimensionSizes, const math::IntervalListVector &M);
    ~IntervalUnionFind();

    uint32_t getRootForPosition(const MixedPosition rep) const;
    void getRepresentativeOfUnion(unsigned int unionI, MixedPosition &rep) const;
    std::vector<uint32_t>::const_iterator rootsBegin() const;
    std::vector<uint32_t>::const_iterator rootsEnd() const;
  };

  /*
  class SimpleUnionFind {
  private:
    unsigned int numDimensions;
    unsigned short dimensionSizes[MAX_DIMENSIONS];
    std::vector<uint32_t> unionRepresentatives; // Of found unions

    UnionFindStructure *ufs;
    uint32_t numUnions;

    uint32_t *v;
    uint32_t sizeV;
  
    void initialFillV(bool const * const M);
    void buildUnions(unsigned int positionI, Position &position, bool const * const M);

  public:
    SimpleUnionFind(unsigned int numDimensions, unsigned short const * const dimensionSizes, bool const * const M);
    ~SimpleUnionFind();

    uint32_t indexOf(const Position &position) const;
    uint32_t get(const Position &position) const;
    void getPosition(uint32_t index, Position &position) const;
    uint32_t get(uint32_t positionIndex) const;
    void getRepresentative(unsigned int unionI, Position &position) const;
    std::vector<uint32_t>::const_iterator rootsBegin() const;
    std::vector<uint32_t>::const_iterator rootsEnd() const;
  };*/
}

#endif // UNION_FIND_H
