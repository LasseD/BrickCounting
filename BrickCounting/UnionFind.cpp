#include "UnionFind.h"

#include <iostream>
#include <assert.h> 

SimpleUnionFind::SimpleUnionFind(unsigned int numDimensions, unsigned short const * const dimensionSizes, bool const * const M) : numDimensions(numDimensions), numUnions(0), sizeV(1), numReducedUnions(0) {
  // Initialize members:
  for(unsigned int i = 0; i < numDimensions; ++i) {
    this->dimensionSizes[i] = dimensionSizes[i];
    sizeV *= dimensionSizes[i];
  }
  //std::cout << "Size v=" << sizeV;
  v = new uint16_t[sizeV];
  //std::cout << " OK" << std::endl;

  // Initially fill v:
  unsigned short position[MAX_DIMENSIONS];
  initialFillV(0, position, M);

  // Initially fill unions:
  for(uint32_t i = 0; i < numUnions; ++i)
    unions[i] = i;

  // Join unions (perform union-find):
  //joinUnions(0, position, M);

  // Create reduced unions for quick lookup:
  createReducedUnions();
}

SimpleUnionFind::~SimpleUnionFind() {
  delete[] v;
}

uint64_t SimpleUnionFind::indexOf(unsigned short const * const position) const {
  uint64_t index = position[0];
  for(unsigned int i = 1; i < numDimensions; ++i)
    index = (index * dimensionSizes[i]) + position[i];
  return index;
}

uint16_t SimpleUnionFind::get(unsigned short const * const position) const {
  const uint64_t indexOfPosition = indexOf(position);
  assert(indexOfPosition < sizeV);
  const uint16_t unionIndex = v[indexOfPosition];
  return unions[unionIndex];
}

void SimpleUnionFind::getRepresentative(unsigned int unionI, unsigned short * rep) const {
  for(unsigned int i = 0; i < numDimensions; ++i)
    rep[i] = unionRepresentatives[MAX_DIMENSIONS*unionI + i];
}

void SimpleUnionFind::createReducedUnions() {
  //std::cout << "Creating reduced unions" << std::endl;
  assert(numReducedUnions == 0);
  assert(numUnions <= MAX_UNIONS);
  bool indexed[MAX_UNIONS];
  for(unsigned int i = 0; i < numUnions; ++i)
    indexed[i] = false;

  for(unsigned int i = 0; i < numUnions; ++i) {
    uint16_t unionI = unions[i];
    assert(unionI < numUnions);
    //std::cout << " unions[" << i << "]: " << unionI << ", indexed: " << indexed[unionI] << std::endl;
    if(indexed[unionI])
      continue;
    indexed[unionI] = true;
    //std::cout << "  reducedUnions[" << numReducedUnions << "] becomes " << unionI << std::endl;
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
    return;
  }

  if(numUnions == MAX_UNIONS) {
    //Die!
    std::cerr << "Assertion error: Number of unions exceed " << MAX_UNIONS << std::endl;
    assert(false); int *die = NULL; die[0] = 42; // Dereferencing null always works... Always!
  }
  uint16_t unionI = (uint16_t)numUnions;

  // Compute position from already seen positions:
  for(unsigned int i = 0; i < numDimensions; ++i) {
    unsigned short val = position[i];
    if(val == 0)
      continue;
    --position[i];
    uint64_t neighbourPositionIndex = indexOf(position); // TODO: Speed upby pre-computing indexOf.
    if(M[neighbourPositionIndex] && v[neighbourPositionIndex] < unionI)
      unionI = v[neighbourPositionIndex];
    ++position[i];
  }
  v[positionIndex] = unionI;

  if(unionI == numUnions) { // Create new union:
    for(unsigned int i = 0; i < numDimensions; ++i)
      unionRepresentatives[MAX_DIMENSIONS*unionI + i] = position[i];
    ++numUnions;
  }
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
  assert(positionIndex < sizeV);
  if(!M[positionIndex]) {
    return; // No union work to be done.
  }

  // Compute min in region:
  uint16_t unionI = v[positionIndex];
  assert(unionI < numUnions);
  uint16_t minInRegion = unions[unionI];
  for(unsigned int i = 0; i < numDimensions; ++i) {
    int oldPosition = position[i];
    for(int addI = -1; addI <= 1; addI+=2) {
      int newPosition = oldPosition + addI;
      if(newPosition < 0 || newPosition >= dimensionSizes[i])
        continue;

      position[i] = (unsigned short)newPosition;

      uint64_t neighbourPositionIndex = indexOf(position); // TODO: Speed upby pre-computing indexOf.
      if(M[neighbourPositionIndex] && unions[v[neighbourPositionIndex]] < minInRegion)
        minInRegion = unions[v[neighbourPositionIndex]];
    }
    position[i] = (unsigned short)oldPosition;
  }

  // Join region (Push minInRegion to self and all neighbours):
  unions[v[positionIndex]] = minInRegion;
  for(unsigned int i = 0; i < numDimensions; ++i) {
    int oldPosition = position[i];
    for(int addI = -1; addI <= 1; addI+=2) {
      int newPosition = oldPosition + addI;
      if(newPosition < 0 || newPosition >= dimensionSizes[i])
        continue;

      position[i] = (unsigned short)newPosition;

      uint64_t neighbourPositionIndex = indexOf(position); // TODO: Speed upby pre-computing indexOf.
      unions[v[neighbourPositionIndex]] = minInRegion;

      position[i] = (unsigned short)oldPosition;
    }
  }
}
