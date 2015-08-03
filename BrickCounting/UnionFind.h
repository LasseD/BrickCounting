#ifndef UNION_FIND_H
#define UNION_FIND_H

#include <stdint.h>
#include <vector>
#include <set>

#define MAX_DIMENSIONS 5
#define MAX_UNIONS 100

struct Position {
  unsigned short p[MAX_DIMENSIONS];

  Position() {}
  Position(const Position &o) {
    for(int i = 0 ; i < MAX_DIMENSIONS; ++i)
      p[i] = o.p[i];
  }
  void init() {
    for(int i = 0 ; i < MAX_DIMENSIONS; ++i)
      p[i] = 0;
  }
};

namespace UnionFind {
  struct UnionFindStructure {
    uint32_t numUnions;
    bool flattened;
    std::set<uint32_t> *joins; // When join(a,b), a<b, b is inserted into joins[a].
    uint32_t *minInUnions; // After flattening: Each index references min element in union. Might be element itself!
    std::vector<int> roots; // all minInUnions-indices that reference themselves.

    UnionFindStructure(uint32_t numUnions);
    ~UnionFindStructure();

    void join(uint32_t a, uint32_t b);
    uint32_t getMinInUnion(uint32_t a) const;
    void flatten(); // Removes joins-structures. Creates minInUnions-list.
  };

  class SimpleUnionFind {
  private:
    unsigned int numDimensions;
    unsigned short dimensionSizes[MAX_DIMENSIONS];
    std::vector<Position> unionRepresentatives; // Of found unions

    UnionFindStructure *ufs; // TODO: FIXME: Replace the stuff below:
    //std::set<uint32_t> *unions;
    //uint32_t *mins; // references min in same union.
    uint32_t numUnions;

    uint32_t *v;
    uint64_t sizeV;
  
    void initialFillV(unsigned int positionI, Position &position, bool const * const M);
    void buildUnions(unsigned int positionI, Position &position, bool const * const M);

  public:
    SimpleUnionFind(unsigned int numDimensions, unsigned short const * const dimensionSizes, bool const * const M);
    ~SimpleUnionFind();

    uint64_t indexOf(const Position &position) const;
    uint32_t get(const Position &position) const;
    uint32_t get(uint64_t positionIndex) const;
    void getRepresentative(unsigned int unionI, Position &position) const;
    uint32_t numRoots() const;
    uint32_t getRoot(uint32_t i) const;
  };
}

#endif // UNION_FIND_H
