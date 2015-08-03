#include "UnionFind.h"

#include <iostream>
#include <assert.h> 
#include <time.h>
#include <stack>
#include "Brick.h"

namespace UnionFind {
  /*
  uint32_t numUnions;
  bool flattened;
  std::set<uint32_t> *joins; // When join(a,b), a<b, b is inserted into joins[a].
  uint32_t *minInUnions; // After flattening: Each index references min element in union. Might be element itself!
  */
  UnionFindStructure::UnionFindStructure(uint32_t numUnions) : numUnions(numUnions), flattened(false) {
    joins = new std::set<uint32_t>[numUnions];
  }
  UnionFindStructure::~UnionFindStructure() {
    assert(flattened);
    delete[] minInUnions;
  }

  void UnionFindStructure::join(uint32_t a, uint32_t b) {
    assert(!flattened);
    if(a > b) {
      join(b,a);
      return;
    }
    if(joins[a].find(b) == joins[a].end()) {
      joins[a].insert(b);
    }
  }
  uint32_t UnionFindStructure::getMinInUnion(uint32_t a) const {
    assert(flattened);
    assert(a < numUnions);
    return minInUnions[a];
  }
  void UnionFindStructure::flatten() {
    assert(!flattened);
    bool *handled = new bool[numUnions];
    for(uint32_t i = 0; i < numUnions; ++i)
      handled[i] = false;
    minInUnions = new uint32_t[numUnions];

    for(uint32_t i = 0; i < numUnions; ++i) {
      // Handle union with representative "i"
      if(handled[i])
        continue;
      minInUnions[i] = i;
      roots.push_back(i);

      std::stack<uint32_t> s;
      for(std::set<uint32_t>::const_iterator it = joins[i].begin(); it != joins[i].end(); ++it)
        s.push(*it);

      while(!s.empty()) {
        uint32_t top = s.top();
        s.pop();
        if(handled[top])
          continue;
        minInUnions[top] = i;
        handled[top] = true;

        for(std::set<uint32_t>::const_iterator it = joins[i].begin(); it != joins[i].end(); ++it)
          s.push(*it);
      }
    }

    delete[] handled;
    delete[] joins;
  }

  SimpleUnionFind::SimpleUnionFind(unsigned int numDimensions, unsigned short const * const dimensionSizes, bool const * const M) : numDimensions(numDimensions), numUnions(0), sizeV(1) {
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

    ufs = new UnionFindStructure(numUnions);
    buildUnions(0, position, M);
    ufs->flatten();

    time(&endTime);
    double seconds = difftime(endTime,startTime);
    if(seconds > 2)
      std::cout << "(Simple) Union find performed in " << seconds << " seconds." << std::endl;
  }

  SimpleUnionFind::~SimpleUnionFind() {
    delete[] v;
    delete ufs;
  }

  uint32_t SimpleUnionFind::numRoots() const {
    assert(ufs->flattened);
    return ufs->roots.size();
  }
  uint32_t SimpleUnionFind::getRoot(uint32_t i) const {
    assert(ufs->flattened);
    return ufs->roots[i];
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
    return ufs->getMinInUnion(unionIndex);
  }

  void SimpleUnionFind::getRepresentative(unsigned int unionI, Position &rep) const {
    rep = unionRepresentatives[unionI];
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
      uint64_t neighbourPositionIndex = indexOf(position); // TODO! FIXME! Speed upby pre-computing indexOf.
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
    assert(unionI < numUnions);
    for(unsigned int i = 0; i < numDimensions; ++i) {
      int oldPosition = position.p[i];
      for(int addI = -1; addI <= 1; addI+=2) {
        int newPosition = oldPosition + addI;
        if(newPosition < 0 || newPosition >= dimensionSizes[i])
          continue;

        position.p[i] = (unsigned short)newPosition;

        uint64_t neighbourPositionIndex = indexOf(position); // TODO! FIXME! Speed upby pre-computing indexOf.
        if(M[neighbourPositionIndex] && v[neighbourPositionIndex] != unionI) {
          ufs->join(v[neighbourPositionIndex], unionI);
        }
      }
      position.p[i] = (unsigned short)oldPosition;
    }
  }

}
