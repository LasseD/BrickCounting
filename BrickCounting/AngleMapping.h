#include "ConnectionPoint.h"
#include "Configuration.hpp"
#include "ConfigurationEncoder.h"

#define STEPS_1 203
#define STEPS_2 370
#define STEPS_3 538

typedef unsigned long long counter;

struct SIsland; // forward declaration.

/*
Mapping angles (See README):
Size 1 => Granularity -203 - 203 => 407 steps.
Size 2 => Granularity -370 - 370 => 741 steps.
Size 2 => Granularity -538 - 538 => 1076 steps.
 */
class AngleMapping {
public:
  unsigned int numAngles, numBricks; //, numScc = numAngles+1;
  unsigned long long sizeMappings; 
  FatSCC sccs[6];
  bool *S, *M, *L; 
  IConnectionPoint points[10];
  unsigned int angleTypes[5]; // Connection(aka. angle) -> 1, 2, or 3.
  unsigned short angleSteps[5]; // Connection(aka. angle) -> 203, 370, or 538.
  const ConfigurationEncoder &encoder;
  uint64_t rectilinearIndex;

public:
  AngleMapping(FatSCC const * const sccs, int numScc, const std::vector<IConnectionPair> &cs, const ConfigurationEncoder &encoder);
  ~AngleMapping();

  /*
    1) For all possible angles: Comput S,M,L.
    2) Combine regions in S,M,L in order to determine new models.
   */
  void findNewConfigurations(std::set<Encoding> &rect, std::set<Encoding> &nonRect, std::vector<Configuration> &toLdr, counter &problematic);
  uint64_t smlIndex(short const * const angleStep) const;

private:
  void findNewExtremeConfigurations(std::set<Encoding> &rect, std::set<Encoding> &nonRect, std::vector<Configuration> &toLdr);
  void evalSML(unsigned int angleI, short *angleStep);
  void findIslands(unsigned int angleI, short *angleStep, std::vector<SIsland> &sIslands);
  void findExtremeConfigurations(unsigned int angleI, short *angleStep, bool allZero, std::set<Encoding> &rect, std::set<Encoding> &nonRect, std::vector<Configuration> &toLdr);
  void setupAngleTypes();
  Configuration getConfiguration(short const * const angleStep) const;
  bool isRectilinear(short *angleStep) const;
};

struct LIsland {
private:
  AngleMapping *a;
  uint64_t clear(short *position) const {
    assert(a->L[a->smlIndex(position)]);
    //std::cout << " M Island add " << a->smlIndex(position) << std::endl;
    a->L[a->smlIndex(position)] = false; // Perform actual clearing.
    uint64_t res = 1;
    for(unsigned int i = 0; i < a->numAngles; ++i) {
      if(position[i] > -a->angleSteps[i]) {
        --position[i];
        if(a->L[a->smlIndex(position)])
          res += clear(position);
        ++position[i];
      }
      if(position[i] < a->angleSteps[i]) {
        ++position[i];
        if(a->L[a->smlIndex(position)])
          res += clear(position);
        --position[i];
      }
    }
    return res;
  }
public:
  short representative[5];
  uint64_t size;
  LIsland(AngleMapping *a, short const * const init) : a(a) {
    short position[5];
    for(unsigned int i = 0; i < a->numAngles; ++i) {
      representative[i] = init[i];
      position[i] = init[i];
    }
    size = clear(position);
  }
  LIsland() {}
  LIsland(const LIsland &l) : a(l.a), size(l.size) {
    for(unsigned int i = 0; i < a->numAngles; ++i) {
      representative[i] = l.representative[i];
    }    
  }
};
struct MIsland {
  std::vector<LIsland> lIslands;
  bool rectilinear;
private:
  AngleMapping *a;
  uint64_t clear(short *position) {
    uint64_t index = a->smlIndex(position);
    //std::cout << " M Island add " << index << std::endl;
    assert(a->M[index]);
    if(a->rectilinearIndex == index) {
      rectilinear = true;
      //std::cout << " rectilinear!" << std::endl;
    }
    if(a->L[index])
      lIslands.push_back(LIsland(a, position));
    a->M[index] = false; // Perform actual clearing.
    uint64_t res = 1;
    for(unsigned int i = 0; i < a->numAngles; ++i) {
      if(position[i] > -a->angleSteps[i]) {
        --position[i];
        if(a->M[a->smlIndex(position)])
          res += clear(position);
        ++position[i];
      }
      if(position[i] < a->angleSteps[i]) {
        ++position[i];
        if(a->M[a->smlIndex(position)])
          res += clear(position);
        --position[i];
      }
    }
    return res;
  }
public:
  short representative[5];
  uint64_t size;
  MIsland(AngleMapping *a, short const * const init) : rectilinear(false), a(a) {
    short position[5];
    for(unsigned int i = 0; i < a->numAngles; ++i) {
      representative[i] = init[i];
      position[i] = init[i];
    }
    size = clear(position);
  }
  MIsland() {}
  MIsland(const MIsland &l) : rectilinear(l.rectilinear), a(l.a), lIslands(l.lIslands), size(l.size) {
    for(unsigned int i = 0; i < a->numAngles; ++i) {
      representative[i] = l.representative[i];
    }    
  }
};
struct SIsland {
  std::vector<MIsland> mIslands;
private:
  AngleMapping *a;
  uint64_t clear(short *position) {
    uint64_t index = a->smlIndex(position);
    assert(a->S[index]);
    if(a->M[index])
      mIslands.push_back(MIsland(a, position));
    a->S[index] = false; // Perform actual clearing.
    uint64_t res = 1;
    for(unsigned int i = 0; i < a->numAngles; ++i) {
      if(position[i] > -a->angleSteps[i]) {
        --position[i];
        if(a->S[a->smlIndex(position)])
          res += clear(position);
        ++position[i];
      }
      if(position[i] < a->angleSteps[i]) {
        ++position[i];
        if(a->S[a->smlIndex(position)])
          res += clear(position);
        --position[i];
      }
    }
    return res;
  }
public:
  short representative[5];
  uint64_t size;
  SIsland(AngleMapping *a, short const * const init) : a(a) {
    //std::cout << " S Island add " << a->smlIndex(init) << std::endl;
    short position[5];
    for(unsigned int i = 0; i < a->numAngles; ++i) {
      representative[i] = init[i];
      position[i] = init[i];
    }
    size = clear(position);
  }
  SIsland() {}
  SIsland(const SIsland &l) : a(l.a), mIslands(l.mIslands), size(l.size) {
    for(unsigned int i = 0; i < a->numAngles; ++i) {
      representative[i] = l.representative[i];
    }
  }
};

