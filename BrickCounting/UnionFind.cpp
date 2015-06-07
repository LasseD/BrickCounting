#include "UnionFind.h"

#include <iostream>
#include <assert.h> 

SimpleUnionFind::SimpleUnionFind(unsigned int numDimensions, unsigned short const * const dimensionSizes, bool const * const M) : numDimensions(numDimensions), numUnions(0), sizeV(0), numReducedUnions(0) {
  // Initialize members:
  for(unsigned int i = 0; i < numDimensions; ++i) {
    this->dimensionSizes[i] = dimensionSizes[i];
    sizeV *= dimensionSizes[i];
  }
  
  // Initially fill v:
  unsigned short position[MAX_DIMENSIONS];
  for(unsigned int i = 0; i < numDimensions; ++i)
    position[i] = 0;
  initialFillV(0, position, M);

  // Initially fill unions:
  for(unsigned int i = 0; i < numUnions; ++i)
    unions[i] = i;

  // Join unions (perform union-find):
  for(unsigned int i = 0; i < numDimensions; ++i)
    position[i] = 0;
  joinUnions(0, position, M);

  // Create reduced unions for quick lookup:
  createReducedUnions();
}

SimpleUnionFind::~SimpleUnionFind() {
  delete[] v;
}

uint64_t SimpleUnionFind::indexOf(unsigned short const * const position) const {
  unsigned long index = position[0];
  for(unsigned int i = 1; i < numDimensions; ++i)
    index = (index * dimensionSizes[i-1]) + position[i];
  return index;
}

uint8_t SimpleUnionFind::get(unsigned short const * const position) const {
  return unions[v[indexOf(position)]];
}

void SimpleUnionFind::getRepresentative(unsigned int unionI, unsigned short * rep) const {
  for(unsigned int i = 0; i < numDimensions; ++i)
    rep[i] = unionRepresentatives[MAX_UNIONS*unionI + i];
}

void SimpleUnionFind::createReducedUnions() {
  bool indexed[MAX_UNIONS];
  for(unsigned int i = 0; i < numUnions; ++i)
    indexed[i] = false;
  
  for(unsigned int i = 0; i < numUnions; ++i) {
    uint8_t unionI = unions[i];
    if(indexed[unionI])
      continue;
    indexed[unionI] = true;
    reducedUnions[numReducedUnions++] = unionI;
  }
}

/*
Computes unionRepresentatives, unions, numUnions, v.
 */
void SimpleUnionFind::initialFillV(unsigned int positionI, unsigned short *position, bool const * const M) {
  if(positionI < numDimensions) {
    for(unsigned short i = 0; i < dimensionSizes[positionI]; ++i) {
      position[positionI] = i;
      initialFillV(positionI+1, position, M);
    }
    return;
  }
  uint64_t positionIndex = indexOf(position);
  if(!M[positionIndex]) {
    v[positionIndex] = 0;
    return;
  }    

  uint8_t unionI = numUnions;
  if(unionI == MAX_UNIONS) {
    //Die!
    std::cerr << "Assertion error: Number of unions exceed " << MAX_UNIONS << std::endl;
    assert(false); int *die = NULL; die[0] = 42; // Dereferencing null always works... Always!
  }

  // Compute position from already seen positions:
  for(unsigned int i = 0; i < numDimensions; ++i) {
    unsigned short val = position[i];
    if(val == 0)
      continue;
    --position[i];
    uint64_t neighbourPositionIndex = indexOf(position); // TODO: Speed upby pre-computing indexOf.
    if(M[neighbourPositionIndex] && v[neighbourPositionIndex] < unionI)
      unionI = v[neighbourPositionIndex];

    // TODO: Add dimension j here!
    ++position[i];
  }
  v[positionIndex] = unionI;
}

void SimpleUnionFind::joinUnions(unsigned int positionI, unsigned short *position, bool const * const M) {
  if(positionI < numDimensions) {
    for(unsigned short i = 0; i < dimensionSizes[positionI]; ++i) {
      position[positionI] = i;
      joinUnions(positionI+1, position, M);
    }
    return;
  }
  uint64_t positionIndex = indexOf(position);
  if(!M[positionIndex]) {
    return; // No union work to be done.
  }

  // Compute min in region:
  uint8_t minInRegion = unions[v[positionIndex]];
  for(unsigned int i = 0; i < numDimensions; ++i) {
    unsigned short val = position[i];
    if(val > 0) {
      --position[i];
      uint64_t neighbourPositionIndex = indexOf(position); // TODO: Speed upby pre-computing indexOf.
      if(M[neighbourPositionIndex] && unions[v[neighbourPositionIndex]] < minInRegion)
	minInRegion = unions[v[neighbourPositionIndex]];
      ++position[i];
    }
    if(val < dimensionSizes[i]-1) {
      ++position[i];
      uint64_t neighbourPositionIndex = indexOf(position); // TODO: Speed upby pre-computing indexOf.
      if(M[neighbourPositionIndex] && unions[v[neighbourPositionIndex]] < minInRegion)
	minInRegion = unions[v[neighbourPositionIndex]];
      --position[i];
    }
  }

  // Join region:
  for(unsigned int i = 0; i < numDimensions; ++i) {
    unsigned short val = position[i];
    if(val > 0) {
      --position[i];
      uint64_t neighbourPositionIndex = indexOf(position); // TODO: Speed upby pre-computing indexOf.
      unions[v[neighbourPositionIndex]] = minInRegion;
      ++position[i];
    }
    if(val < dimensionSizes[i]-1) {
      ++position[i];
      uint64_t neighbourPositionIndex = indexOf(position); // TODO: Speed upby pre-computing indexOf.
      unions[v[neighbourPositionIndex]] = minInRegion;
      --position[i];
    }
  }
}
