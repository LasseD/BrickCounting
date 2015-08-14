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
    assert(a < b);
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

        for(std::set<uint32_t>::const_iterator it = joins[top].begin(); it != joins[top].end(); ++it)
          s.push(*it);
      }
    }

    delete[] handled;
    delete[] joins;
    flattened = true;
  }

  /*
    unsigned int numStepDimensions;
    unsigned short dimensionSizes[MAX_DIMENSIONS-1]; // "-1" because last dimension is not a step dimension.
    std::vector<std::pair<uint64_t,double>> unionRepresentatives; // Of found unions
    UnionFindStructure *ufs;
    uint32_t numUnions;
  */
  IntervalUnionFind::IntervalUnionFind(unsigned int numDimensions, unsigned short const * const dimensionSizes, const math::IntervalListVector &M) {
    // TODO: Implement
  }
  IntervalUnionFind::~IntervalUnionFind() {
    // TODO: Implement
  }
  void IntervalUnionFind::initialFillV(bool const * const M) {
    // TODO: Implement
  }
  void IntervalUnionFind::buildUnions(unsigned int positionI, MixedPosition &position, const math::IntervalListVector &M) {
    // TODO: Implement
  }
  uint64_t IntervalUnionFind::indexOf(const MixedPosition &position) const {
    // TODO: Implement
    return 0;
  }
  uint32_t IntervalUnionFind::get(const MixedPosition &position) const {
    // TODO: Implement
    return 0;
  }
  void IntervalUnionFind::getPosition(uint64_t index, MixedPosition &position) const {
    // TODO: Implement
  }
  uint32_t IntervalUnionFind::get(uint64_t positionIndex) const {
    // TODO: Implement
    return 0;
  }
  void IntervalUnionFind::getRepresentative(unsigned int unionI, MixedPosition &position) const {
    // TODO: Implement
  }
  std::vector<uint32_t>::const_iterator IntervalUnionFind::rootsBegin() const {
    return ufs->roots.begin();
  }
  std::vector<uint32_t>::const_iterator IntervalUnionFind::rootsEnd() const {
    return ufs->roots.end();
  }


  /*
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

    // Initially fill v:
    v = new uint32_t[sizeV];
    initialFillV(M);

    // Perform joins:
    ufs = new UnionFindStructure(numUnions);
    Position position;
    buildUnions(0, position, M);

    // Flatten:
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

  std::vector<uint32_t>::const_iterator SimpleUnionFind::rootsBegin() const {
    return ufs->roots.begin();
  }
  std::vector<uint32_t>::const_iterator SimpleUnionFind::rootsEnd() const {
    return ufs->roots.end();
  }

  uint64_t SimpleUnionFind::indexOf(const Position &position) const {
    uint64_t index = position.p[0];
    for(unsigned int i = 1; i < numDimensions; ++i)
      index = (index * dimensionSizes[i]) + position.p[i];
    return index;
  }

  void SimpleUnionFind::getPosition(uint64_t index, Position &position) const {
#ifdef _DEBUG
    const uint64_t save = index;
#endif
    for(unsigned int i = 0; i < numDimensions; ++i) {
      int revDim = numDimensions-i-1;
      position.p[revDim] = index % dimensionSizes[revDim];
      index /= dimensionSizes[revDim];
    }
#ifdef _DEBUG
    if(save != indexOf(position)) {
      std::cerr << "Assertion error on index " << index << "!= " << indexOf(position) << std::endl;
      assert(false);
    }
#endif
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
    uint64_t representativeIndex = unionRepresentatives[unionI];
    getPosition(representativeIndex, rep);
  }

  void SimpleUnionFind::initialFillV(bool const * const M) {
    assert(numUnions == 0);
    unsigned short lastDimensionSize = dimensionSizes[numDimensions-1];
    for(uint64_t i = 0; i < sizeV; i+=lastDimensionSize) {
      if(M[i]) {
        v[i] = numUnions;
        unionRepresentatives.push_back(i);
        ++numUnions;
      }

      for(uint64_t j = 1; j < lastDimensionSize; ++j) {
        if(!M[i+j]) 
          continue;
        if(M[i+j-1]) { 
          v[i+j] = v[i+j-1];
          continue;
        }

        v[i+j] = numUnions;
        unionRepresentatives.push_back(i+j);
        ++numUnions;
      }
    }
  }

  void SimpleUnionFind::buildUnions(unsigned int positionI, Position &position, bool const * const M) {
    if(positionI < numDimensions-1) {
      for(unsigned short i = 0; i < dimensionSizes[positionI]; ++i) {
        position.p[positionI] = i;
        buildUnions(positionI+1, position, M);
      }
      return;
    }
    assert(positionI == numDimensions-1);
    position.p[positionI] = 0;

    uint64_t positionIndex = indexOf(position);
    assert(positionIndex < sizeV);

    // Run and join for each lower dimension:

    for(unsigned int i = 0; i < numDimensions-1; ++i) {
      unsigned short oldPosition = position.p[i];
      if(oldPosition == 0)
        continue; // Can't further decrease in this dimension.
      position.p[i] = oldPosition - 1;
      uint64_t neighbourPositionIndex = indexOf(position);
      position.p[i] = oldPosition;

      // Join all in dimension:
      for(unsigned short i = 0; i < dimensionSizes[numDimensions-1]; ++i) {
        uint64_t pos = positionIndex + i;
        if(!M[pos]) {
          continue;
        }
        uint64_t lowerPos = neighbourPositionIndex + i;
        if(!M[lowerPos]) {
          continue;
        }
        if(i == 0 || !(M[lowerPos-1] && M[pos-1]))
          ufs->join(v[lowerPos], v[pos]);
      }
    }
  }*/
}
