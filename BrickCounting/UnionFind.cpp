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
    if(joins[a].find(b) == joins[a].end()) {
      assert(joins[b].find(a) == joins[b].end());
      joins[a].insert(b);
      joins[b].insert(a);
    }
  }
  void UnionFindStructure::join(const IntervalList &l1, const IntervalList &l2, uint32_t union1, uint32_t union2) {
    IntervalList::const_iterator it1 = l1.begin();
    IntervalList::const_iterator it2 = l2.begin();
    
    while(it1 != l1.end() && it2 != l2.end()) {
      if(it2->second < it1->first) {
        ++it2;
        ++union2;
        continue;
      }
      if(it1->second < it2->first) {
        ++it1;
        ++union1;
        continue;
      }
      join(union1, union2);
      if(it1->second <= it2->second) {
        ++it1;
        ++union1;
      }
      else {
        ++it2;
        ++union2;
      }
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

    //std::cout << "Start root finding" << std::endl;
    for(uint32_t i = 0; i < numUnions; ++i) {
      // Handle union with representative "i"
      if(handled[i])
        continue;
      minInUnions[i] = i;
      handled[i] = true;
      roots.push_back(i);
      //std::cout << " Root " << i;
      int rootSize = 0;

      std::stack<uint32_t> s;
      for(std::set<uint32_t>::const_iterator it = joins[i].begin(); it != joins[i].end(); ++it)
        s.push(*it);

      while(!s.empty()) {
        ++rootSize;
        uint32_t top = s.top();
        assert(top < numUnions);
        s.pop();
        if(handled[top])
          continue;
        minInUnions[top] = i;
        handled[top] = true;

        for(std::set<uint32_t>::const_iterator it = joins[top].begin(); it != joins[top].end(); ++it) {
          if(!handled[*it])
            s.push(*it);
        }
      }
      //std::cout << " of size " << rootSize << std::endl;
    }
    //std::cout << "Done root finding" << std::endl;

    delete[] handled;
    delete[] joins;
    flattened = true;
  }

  /*
    unsigned int numStepDimensions;
    unsigned short dimensionSizes[MAX_DIMENSIONS-1]; // "-1" because last dimension is not a step dimension.
    UnionFindStructure *ufs;
    uint32_t *intervalIndicatorToUnion;
    std::pair<uint32_t,unsigned int> *unionToInterval;
    const math::IntervalListVector &M; // ref only, no ownership.
  */
  IntervalUnionFind::IntervalUnionFind(unsigned int numDimensions, unsigned short const * const dimensionSizes, const math::IntervalListVector &M) : numStepDimensions(numDimensions-1), unionI(0), M(M) {
    time_t startTime, endTime;
    time(&startTime);

    // Initialize members:
    for(unsigned int i = 0; i < numDimensions; ++i) {
      this->dimensionSizes[i] = dimensionSizes[i];
    }

    // Initially fill basic vectors:
    buildIntervalIndicatorToUnion();

    // Perform joins:
    ufs = new UnionFindStructure(unionI);
    MixedPosition position;
    buildUnions(0, position);

    // Flatten:
    ufs->flatten();

    time(&endTime);
    double seconds = difftime(endTime,startTime);
    if(seconds > 2)
      std::cout << "Union find performed in " << seconds << " seconds." << std::endl;
  }

  IntervalUnionFind::~IntervalUnionFind() {
    delete ufs;
    delete[] intervalIndicatorToUnion;
    delete[] unionToInterval;
  }
  void IntervalUnionFind::buildIntervalIndicatorToUnion() {
    intervalIndicatorToUnion = new uint32_t[M.sizeIndicator()];
    unionToInterval = new std::pair<uint32_t,unsigned int>[M.sizeNonEmptyIntervals()];

    for(uint32_t i = 0; i < M.sizeIndicator(); ++i) {
      uint32_t intervalSize = M.intervalSizeForIndicator(i);
      if(intervalSize == 0)
        continue;
      intervalIndicatorToUnion[i] = unionI;
      for(uint32_t j = 0; j < intervalSize; ++j) {
        unionToInterval[unionI+j].first = i;
        unionToInterval[unionI+j].second = j;
      }
      unionI += intervalSize;
    }
  }
  void IntervalUnionFind::buildUnions(unsigned int positionI, MixedPosition &position) {
    if(positionI < numStepDimensions) {
      for(unsigned short i = 0; i < dimensionSizes[positionI]; ++i) {
        position.p[positionI] = i;
        buildUnions(positionI+1, position);
      }
      return;
    }
    position.p[numStepDimensions] = 0;

    uint32_t positionIndex = indexOf(position);
    IntervalList l1;
    M.get(positionIndex, l1);
    if(l1.empty())
      return;
    const uint32_t unionStart1 = intervalIndicatorToUnion[positionIndex];

    // Run and join for each lower dimension:
    for(unsigned int i = 0; i < numStepDimensions; ++i) {
      if(position.p[i] == 0)
        continue; // Can't further decrease in this dimension.
      --position.p[i];
      uint32_t neighbourPositionIndex = indexOf(position);
      ++position.p[i];

      // Join all in dimension:
      IntervalList l2;
      M.get(neighbourPositionIndex, l2);
      if(l2.empty())
        continue;
      const uint32_t unionStart2 = intervalIndicatorToUnion[neighbourPositionIndex];

      ufs->join(l1, l2, unionStart1, unionStart2);
    }
  }
  uint32_t IntervalUnionFind::indexOf(const MixedPosition &position) const {
    uint32_t index = numStepDimensions == 0 ? 0 : position.p[0];
    for(unsigned int i = 1; i < numStepDimensions; ++i)
      index = (index * dimensionSizes[i]) + position.p[i];
    return index;
  }
  uint32_t IntervalUnionFind::getRootForPosition(const MixedPosition rep) const {
    uint32_t indicatorIndex = indexOf(rep);
    assert(indicatorIndex < M.sizeIndicator());
    uint32_t firstUnion = intervalIndicatorToUnion[indicatorIndex];

    IntervalList l;
    M.get(indicatorIndex, l);
    uint32_t index = 0;
    for(IntervalList::const_iterator it = l.begin(); it != l.end(); ++it, ++index) {
      if(it->first <= rep.lastAngle && rep.lastAngle <= it->second) {
        return ufs->getMinInUnion(firstUnion + index);
      }
    }
    assert(false);std::cerr << "DIE X007" << std::endl;
    int *die = NULL; die[0] = 42;
    return 0;
  }
  void IntervalUnionFind::getRepresentativeOfUnion(unsigned int unionI, MixedPosition &rep) const {
    assert(unionI < M.sizeNonEmptyIntervals());
    std::pair<uint32_t,unsigned int> intervalInfo = unionToInterval[unionI];

    // Construct position from interval index:
    uint32_t encodedPosition = intervalInfo.first;
    for(unsigned int i = 0; i < numStepDimensions; ++i) {
      int revDim = numStepDimensions-i-1;
      rep.p[revDim] = encodedPosition % dimensionSizes[revDim];
      encodedPosition /= dimensionSizes[revDim];
    }

    // Get angle from M:
    Interval interval = M.get(intervalInfo.first, intervalInfo.second);
    rep.lastAngle = (interval.first + interval.second)/2;
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

    // Initialize members:
    for(unsigned int i = 0; i < numDimensions; ++i) {
      this->dimensionSizes[i] = dimensionSizes[i];
      sizeV *= dimensionSizes[i];
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

  uint32_t SimpleUnionFind::indexOf(const Position &position) const {
    uint32_t index = position.p[0];
    for(unsigned int i = 1; i < numDimensions; ++i)
      index = (index * dimensionSizes[i]) + position.p[i];
    return index;
  }

  void SimpleUnionFind::getPosition(uint32_t index, Position &position) const {
#ifdef _DEBUG
    const uint32_t save = index;
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
    const uint32_t indexOfPosition = indexOf(position);
    return get(indexOfPosition);
  }

  uint32_t SimpleUnionFind::get(uint32_t indexOfPosition) const {
    assert(indexOfPosition < sizeV);
    const uint32_t unionIndex = v[indexOfPosition];
    assert(unionIndex < numUnions);
    return ufs->getMinInUnion(unionIndex);
  }

  void SimpleUnionFind::getRepresentative(unsigned int unionI, Position &rep) const {
    uint32_t representativeIndex = unionRepresentatives[unionI];
    getPosition(representativeIndex, rep);
  }

  void SimpleUnionFind::initialFillV(bool const * const M) {
    assert(numUnions == 0);
    unsigned short lastDimensionSize = dimensionSizes[numDimensions-1];
    for(uint32_t i = 0; i < sizeV; i+=lastDimensionSize) {
      if(M[i]) {
        v[i] = numUnions;
        unionRepresentatives.push_back(i);
        ++numUnions;
      }

      for(uint32_t j = 1; j < lastDimensionSize; ++j) {
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

    uint32_t positionIndex = indexOf(position);
    assert(positionIndex < sizeV);

    // Run and join for each lower dimension:

    for(unsigned int i = 0; i < numDimensions-1; ++i) {
      unsigned short oldPosition = position.p[i];
      if(oldPosition == 0)
        continue; // Can't further decrease in this dimension.
      position.p[i] = oldPosition - 1;
      uint32_t neighbourPositionIndex = indexOf(position);
      position.p[i] = oldPosition;

      // Join all in dimension:
      for(unsigned short i = 0; i < dimensionSizes[numDimensions-1]; ++i) {
        uint32_t pos = positionIndex + i;
        if(!M[pos]) {
          continue;
        }
        uint32_t lowerPos = neighbourPositionIndex + i;
        if(!M[lowerPos]) {
          continue;
        }
        if(i == 0 || !(M[lowerPos-1] && M[pos-1]))
          ufs->join(v[lowerPos], v[pos]);
      }
    }
  }*/
}
