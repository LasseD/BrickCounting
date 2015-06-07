#ifndef UNION_FIND_H
#define UNION_FIND_H

#include <stdint.h>

#define MAX_DIMENSIONS 5
#define MAX_UNIONS 256

class SimpleUnionFind {
private:
  unsigned int numDimensions;
  unsigned short dimensionSizes[MAX_DIMENSIONS];
  unsigned short unionRepresentatives[MAX_UNIONS*MAX_DIMENSIONS]; // found unions
  uint8_t unions[MAX_UNIONS]; // references min in same union.
  unsigned int numUnions;
  uint8_t *v;
  unsigned long sizeV;
  
  void initialFillV(unsigned int positionI, unsigned short *position, bool const * const M);
  void joinUnions(unsigned int positionI, unsigned short *position, bool const * const M);
  void createReducedUnions();

public:
  SimpleUnionFind(unsigned int numDimensions, unsigned short const * const dimensionSizes, bool const * const M);
  ~SimpleUnionFind();

  uint64_t indexOf(unsigned short const * const position) const;
  uint8_t get(unsigned short const * const position) const;
  void getRepresentative(unsigned int unionI, unsigned short * rep) const;
  uint8_t reducedUnions[MAX_UNIONS]; // index -> original index for unionRepresentatives
  unsigned int numReducedUnions;
};

#endif // UNION_FIND_H
