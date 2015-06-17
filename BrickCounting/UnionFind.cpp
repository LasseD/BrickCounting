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
  v = new uint32_t[sizeV];
  //std::cout << " OK" << std::endl;

  // Initially fill v:
  Position position;
  initialFillV(0, position, M);

  // Initially fill unions:
  unions = new uint32_t[numUnions];
  for(uint32_t i = 0; i < numUnions; ++i)
    unions[i] = i;

  // Join unions (perform union-find):
  joinUnions(0, position, M);

  // Create reduced unions for quick lookup:
  createReducedUnions();
}

SimpleUnionFind::~SimpleUnionFind() {
  delete[] v;
  delete[] unions;
}

uint64_t SimpleUnionFind::indexOf(const Position &position) const {
  uint64_t index = position.p[0];
  for(unsigned int i = 1; i < numDimensions; ++i)
    index = (index * dimensionSizes[i]) + position.p[i];
  return index;
}

uint32_t SimpleUnionFind::get(const Position &position) const {
  const uint64_t indexOfPosition = indexOf(position);
  assert(indexOfPosition < sizeV);
  const uint32_t unionIndex = v[indexOfPosition];
  return unions[unionIndex];
}

void SimpleUnionFind::getRepresentative(unsigned int unionI, Position &rep) const {
  rep = unionRepresentatives[unionI];
}

void SimpleUnionFind::createReducedUnions() {
  //std::cout << "Creating reduced unions" << std::endl;
  assert(numReducedUnions == 0);
  assert(numUnions <= MAX_UNIONS);
  bool *indexed = new bool[numUnions];
  for(unsigned int i = 0; i < numUnions; ++i)
    indexed[i] = false;

  for(unsigned int i = 0; i < numUnions; ++i) {
    uint32_t unionI = unions[i];
    assert(unionI < numUnions);
    //std::cout << " unions[" << i << "]: " << unionI << ", indexed: " << indexed[unionI] << std::endl;
    if(indexed[unionI])
      continue;
    indexed[unionI] = true;
    //std::cout << "  reducedUnions[" << numReducedUnions << "] becomes " << unionI << std::endl;
    reducedUnions[numReducedUnions++] = unionI;
  }

  delete[] indexed;
}

/*
Computes unionRepresentatives, unions, numUnions, v.
*/
void SimpleUnionFind::initialFillV(unsigned int positionI, Position &position, bool const * const M) {
  if(positionI < numDimensions) {
    for(unsigned short i = 0; i < dimensionSizes[positionI]; ++i) {
      position.p[positionI] = i;
      initialFillV(positionI+1, position, M);
    }
    return;
  }
  uint64_t positionIndex = indexOf(position);
  if(!M[positionIndex]) {
    return;
  }

  uint32_t unionI = numUnions;

  // Compute position from already seen positions:
  for(unsigned int i = 0; i < numDimensions; ++i) {
    unsigned short val = position.p[i];
    if(val == 0)
      continue;
    --position.p[i];
    uint64_t neighbourPositionIndex = indexOf(position); // TODO: Speed upby pre-computing indexOf.
    if(M[neighbourPositionIndex] && v[neighbourPositionIndex] < unionI)
      unionI = v[neighbourPositionIndex];
    ++position.p[i];
  }
  v[positionIndex] = unionI;

  if(unionI == numUnions) { // Create new union:
    unionRepresentatives.push_back(position);
    ++numUnions;
  }
}

void SimpleUnionFind::joinUnions(unsigned int positionI, Position &position, bool const * const M) {
  if(positionI < numDimensions) {
    for(unsigned short i = 0; i < dimensionSizes[positionI]; ++i) {
      position.p[positionI] = i;
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
  uint32_t unionI = v[positionIndex];
  assert(unionI < numUnions);
  uint32_t minInRegion = unions[unionI];
  for(unsigned int i = 0; i < numDimensions; ++i) {
    int oldPosition = position.p[i];
    for(int addI = -1; addI <= 1; addI+=2) {
      int newPosition = oldPosition + addI;
      if(newPosition < 0 || newPosition >= dimensionSizes[i])
        continue;

      position.p[i] = (unsigned short)newPosition;

      uint64_t neighbourPositionIndex = indexOf(position); // TODO: Speed upby pre-computing indexOf.
      if(M[neighbourPositionIndex] && unions[v[neighbourPositionIndex]] < minInRegion)
        minInRegion = unions[v[neighbourPositionIndex]];
    }
    position.p[i] = (unsigned short)oldPosition;
  }

  // Join region (Push minInRegion to self and all neighbours):
  assert(positionIndex < sizeV);
  uint32_t unionsI = v[positionIndex];
  assert(unionsI < numUnions);
  unions[unionsI] = minInRegion;
  for(unsigned int i = 0; i < numDimensions; ++i) {
    int oldPosition = position.p[i];
    for(int addI = -1; addI <= 1; addI+=2) {
      int newPosition = oldPosition + addI;
      if(newPosition < 0 || newPosition >= dimensionSizes[i])
        continue;

      position.p[i] = (unsigned short)newPosition;

      uint64_t neighbourPositionIndex = indexOf(position); // TODO: Speed upby pre-computing indexOf.
      assert(neighbourPositionIndex < sizeV);
      if(!M[neighbourPositionIndex])
	continue;
      uint32_t unionNI = v[neighbourPositionIndex];
      assert(unionNI < numUnions);
      unions[unionNI] = minInRegion;
    }
    position.p[i] = (unsigned short)oldPosition;
  }
}
