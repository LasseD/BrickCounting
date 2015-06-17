#ifndef UNION_FIND_H
#define UNION_FIND_H

#include <stdint.h>
#include <vector>

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

class SimpleUnionFind {
private:
  unsigned int numDimensions;
  unsigned short dimensionSizes[MAX_DIMENSIONS];
  std::vector<Position> unionRepresentatives; // Of found unions
  uint32_t *unions; // references min in same union.
  uint32_t numUnions;
  uint32_t *v;
  uint64_t sizeV;
  
  void initialFillV(unsigned int positionI, Position &position, bool const * const M);
  void joinUnions(unsigned int positionI, Position &position, bool const * const M);
  void createReducedUnions();

public:
  SimpleUnionFind(unsigned int numDimensions, unsigned short const * const dimensionSizes, bool const * const M);
  ~SimpleUnionFind();

  uint64_t indexOf(const Position &position) const;
  uint32_t get(const Position &position) const;
  void getRepresentative(unsigned int unionI, Position &position) const;

  uint32_t reducedUnions[MAX_UNIONS]; // index -> original index for unionRepresentatives
  unsigned int numReducedUnions;
};

#endif // UNION_FIND_H