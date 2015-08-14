#ifndef UNION_FIND_H
#define UNION_FIND_H

#include <stdint.h>
#include <vector>
#include <set>
#include "Math.h"

#define MAX_DIMENSIONS 5
#define MAX_UNIONS 100

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
    std::set<uint32_t> *joins; // When join(a,b), a<b, b is inserted into joins[a].
    uint32_t *minInUnions; // After flattening: Each index references min element in union. Might be element itself!
    std::vector<uint32_t> roots; // all minInUnions-indices that reference themselves.

    UnionFindStructure(uint32_t numUnions);
    ~UnionFindStructure();

    void join(uint32_t a, uint32_t b);
    uint32_t getMinInUnion(uint32_t a) const;
    void flatten(); // Removes joins-structures. Creates minInUnions-list.
  };

  class IntervalUnionFind {
  private:
    unsigned int numStepDimensions;
    unsigned short dimensionSizes[MAX_DIMENSIONS-1]; // "-1" because last dimension is not a step dimension.
    std::vector<std::pair<uint64_t,double>> unionRepresentatives; // Of found unions

    UnionFindStructure *ufs;
    uint32_t numUnions;
  
    void initialFillV(bool const * const M);
    void buildUnions(unsigned int positionI, MixedPosition &position, const math::IntervalListVector &M);

  public:
    IntervalUnionFind(unsigned int numDimensions, unsigned short const * const dimensionSizes, const math::IntervalListVector &M);
    ~IntervalUnionFind();

    uint64_t indexOf(const MixedPosition &position) const;
    uint32_t get(const MixedPosition &position) const;
    void getPosition(uint64_t index, MixedPosition &position) const;
    uint32_t get(uint64_t positionIndex) const;
    void getRepresentative(unsigned int unionI, MixedPosition &position) const;
    std::vector<uint32_t>::const_iterator rootsBegin() const;
    std::vector<uint32_t>::const_iterator rootsEnd() const;
  };

  /*
  class SimpleUnionFind {
  private:
    unsigned int numDimensions;
    unsigned short dimensionSizes[MAX_DIMENSIONS];
    std::vector<uint64_t> unionRepresentatives; // Of found unions

    UnionFindStructure *ufs;
    uint32_t numUnions;

    uint32_t *v;
    uint64_t sizeV;
  
    void initialFillV(bool const * const M);
    void buildUnions(unsigned int positionI, Position &position, bool const * const M);

  public:
    SimpleUnionFind(unsigned int numDimensions, unsigned short const * const dimensionSizes, bool const * const M);
    ~SimpleUnionFind();

    uint64_t indexOf(const Position &position) const;
    uint32_t get(const Position &position) const;
    void getPosition(uint64_t index, Position &position) const;
    uint32_t get(uint64_t positionIndex) const;
    void getRepresentative(unsigned int unionI, Position &position) const;
    std::vector<uint32_t>::const_iterator rootsBegin() const;
    std::vector<uint32_t>::const_iterator rootsEnd() const;
  };*/
}

#endif // UNION_FIND_H
