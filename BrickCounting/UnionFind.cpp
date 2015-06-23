#include "UnionFind.h"

#include <iostream>
#include <assert.h> 
#include <time.h>
#include "Brick.h"

SimpleUnionFind::SimpleUnionFind(unsigned int numDimensions, unsigned short const * const dimensionSizes, bool const * const M) : numDimensions(numDimensions), numUnions(0), sizeV(1), numReducedUnions(0) {
  time_t startTime, endTime;
  time(&startTime);

  unsigned short maxDimensionSize = 0;

  // Initialize members:
  for(unsigned int i = 0; i < numDimensions; ++i) {
    this->dimensionSizes[i] = dimensionSizes[i];
    sizeV *= dimensionSizes[i];
    if(dimensionSizes[i] > maxDimensionSize)
      maxDimensionSize = dimensionSizes[i];
  }
  v = new uint32_t[sizeV];

  // Initially fill v:
  Position position;
  initialFillV(0, position, M);

  // Join unions (perform union-find):
  unions = new std::set<uint32_t>[numUnions];
  buildUnions(0, position, M);

  // Initially fill unions:
  mins = new uint32_t[numUnions];
  createMins();

  // Create reduced unions for quick lookup:
  createReducedUnions();

  time(&endTime);
  double seconds = difftime(endTime,startTime);
  if(seconds > 2)
    std::cout << "(Simple) Union find performed in " << seconds << " seconds." << std::endl;
}

SimpleUnionFind::~SimpleUnionFind() {
  delete[] v;
  delete[] mins;
  delete[] unions;
}

void SimpleUnionFind::createMins() {
  bool *indexed = new bool[numUnions];
  for(unsigned int i = 0; i < numUnions; ++i)
    indexed[i] = false;

  for(unsigned int i = 0; i < numUnions; ++i) {
    if(indexed[i])
      continue;
    indexed[i] = true;
    mins[i] = i;
    for(std::set<uint32_t>::const_iterator it = unions[i].begin(); it != unions[i].end(); ++it) {
      indexed[*it] = true;
      mins[*it] = i;
    }
  }

  delete[] indexed;
}

uint64_t SimpleUnionFind::indexOf(const Position &position) const {
  uint64_t index = position.p[0];
  for(unsigned int i = 1; i < numDimensions; ++i)
    index = (index * dimensionSizes[i]) + position.p[i];
  return index;
}

uint32_t SimpleUnionFind::get(const Position &position) const {
  const uint64_t indexOfPosition = indexOf(position);
  return get(indexOfPosition);
}

uint32_t SimpleUnionFind::get(uint64_t indexOfPosition) const {
  assert(indexOfPosition < sizeV);
  const uint32_t unionIndex = v[indexOfPosition];
  assert(unionIndex < numUnions);
  return mins[unionIndex];
}

void SimpleUnionFind::getRepresentative(unsigned int unionI, Position &rep) const {
  rep = unionRepresentatives[unionI];
}

void SimpleUnionFind::createReducedUnions() {
  assert(numReducedUnions == 0);
  bool *indexed = new bool[numUnions];
  for(unsigned int i = 0; i < numUnions; ++i)
    indexed[i] = false;

  for(unsigned int i = 0; i < numUnions; ++i) {
    uint32_t unionI = mins[i];
    assert(unionI < numUnions);
    if(indexed[unionI])
      continue;
    indexed[unionI] = true;
    reducedUnions[numReducedUnions++] = unionI;
  }

  if(numUnions > numReducedUnions*100)
    std::cout << "Reducing " << numUnions << " to " << numReducedUnions << "!" << std::endl;

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
    if(M[neighbourPositionIndex] && v[neighbourPositionIndex] < unionI) {
      unionI = v[neighbourPositionIndex];
      ++position.p[i];
      break;
    }
    ++position.p[i];
  }
  v[positionIndex] = unionI;

  if(unionI == numUnions) { // Create new union:
    unionRepresentatives.push_back(position);
    ++numUnions;
  }
}

void SimpleUnionFind::join(uint32_t a, uint32_t b) {
  if(a > b) {
    join(b, a);
    return;
  }
  if(unions[a].find(b) == unions[a].end()) {
    unions[a].insert(b);
  }
}

void SimpleUnionFind::buildUnions(unsigned int positionI, Position &position, bool const * const M) {
  if(positionI < numDimensions) {
    for(unsigned short i = 0; i < dimensionSizes[positionI]; ++i) {
      position.p[positionI] = i;
      buildUnions(positionI+1, position, M);
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
  //std::cout << "Initial union for position index " << positionIndex << ": " << unionI << std::endl;
  assert(unionI < numUnions);
  for(unsigned int i = 0; i < numDimensions; ++i) {
    int oldPosition = position.p[i];
    for(int addI = -1; addI <= 1; addI+=2) {
      int newPosition = oldPosition + addI;
      if(newPosition < 0 || newPosition >= dimensionSizes[i])
        continue;

      position.p[i] = (unsigned short)newPosition;

      uint64_t neighbourPositionIndex = indexOf(position); // TODO: Speed upby pre-computing indexOf.
      if(M[neighbourPositionIndex] && v[neighbourPositionIndex] != unionI) {
	join(v[neighbourPositionIndex], unionI);
      }
    }
    position.p[i] = (unsigned short)oldPosition;
  }
}
