#ifndef UNION_FIND_H
#define UNION_FIND_H

#include <stdint.h>

#define MAX_DIMENSIONS 5
#define MAX_UNIONS 1024

class SimpleUnionFind {
private:
  unsigned int numDimensions;
  unsigned short dimensionSizes[MAX_DIMENSIONS];
  unsigned short unionRepresentatives[MAX_UNIONS*MAX_DIMENSIONS]; // found unions
  uint16_t unions[MAX_UNIONS]; // references min in same union.
  uint32_t numUnions;
  uint16_t *v;
  uint64_t sizeV;
  
  void initialFillV(unsigned int positionI, unsigned short *position, bool const * const M);
  void joinUnions(unsigned int positionI, unsigned short *position, bool const * const M);
  void createReducedUnions();

public:
  SimpleUnionFind(unsigned int numDimensions, unsigned short const * const dimensionSizes, bool const * const M);
  ~SimpleUnionFind();

  uint64_t indexOf(unsigned short const * const position) const;
  uint16_t get(unsigned short const * const position) const;
  void getRepresentative(unsigned int unionI, unsigned short * rep) const;
  uint16_t reducedUnions[MAX_UNIONS]; // index -> original index for unionRepresentatives
  unsigned int numReducedUnions;
};

#endif // UNION_FIND_H
