#ifndef ANGLEMAPPING_H
#define ANGLEMAPPING_H

#include <stdint.h>
#include "ConnectionPoint.h"
#include "Configuration.hpp"
#include "ConfigurationEncoder.h"
#include "UnionFind.h"

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
  uint64_t sizeMappings; 
  FatSCC sccs[6];
  bool *S, *M, *L; 
  SimpleUnionFind *ufS, *ufM, *ufL;
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
  void findNewConfigurations(std::set<Encoding> &rect, std::set<Encoding> &nonRect, std::vector<std::vector<Connection> > &toLdr, std::vector<Configuration> &nrcToPrint, std::vector<Configuration> &modelsToPrint, counter &models, counter &problematic);
  uint64_t smlIndex(unsigned short const * const angleStep) const;

private:
  void reportProblematic(unsigned short const * const angleStep, int mIslandI, int mIslandTotal, int lIslandTotal, std::vector<std::vector<Connection> > &toLdr) const;
  void findNewExtremeConfigurations(std::set<Encoding> &rect, std::set<Encoding> &nonRect, std::vector<std::vector<Connection> > &toLdr);
  void evalSML(unsigned int angleI, unsigned short *angleStep);
  void findIslands(std::multimap<Encoding, SIsland> &sIslands, std::set<Encoding> &keys);
  void findExtremeConfigurations(unsigned int angleI, unsigned short *angleStep, bool allZero, std::set<Encoding> &rect, std::set<Encoding> &nonRect, std::vector<std::vector<Connection> > &toLdr);
  void setupAngleTypes();
  Configuration getConfiguration(unsigned short const * const angleStep) const;
  std::vector<Connection> getConfigurationConnections(unsigned short const * const angleStep) const;
  bool isRectilinear(unsigned short *angleStep) const;
};

struct LIsland {
  unsigned int sizeRep;
  unsigned short representative[5];

  LIsland(unsigned int sizeRep, unsigned short const * const init) : sizeRep(sizeRep) {
    for(unsigned int i = 0; i < sizeRep; ++i)
      representative[i] = init[i];
  }
  LIsland() {}
  LIsland(const LIsland &l) : sizeRep(l.sizeRep) {
    for(unsigned int i = 0; i < sizeRep; ++i)
      representative[i] = l.representative[i];
  }
};
struct MIsland {
  std::vector<LIsland> lIslands;
  bool rectilinear;
  unsigned int sizeRep;
  unsigned short representative[5];

  MIsland(AngleMapping *a, uint16_t unionFindIndex, unsigned short const * const init) : sizeRep(a->numAngles) {
    for(unsigned int i = 0; i < sizeRep; ++i)
      representative[i] = init[i];

    rectilinear = a->ufM->get(init) == unionFindIndex;
    // Add all L-islands:
    for(unsigned int i = 0; i < a->ufL->numReducedUnions; ++i) {
      const uint16_t unionI = a->ufL->reducedUnions[i];
      unsigned short rep[5];
      a->ufL->getRepresentative(unionI, rep);
      assert(a->M[a->ufL->indexOf(rep)]);
      if(a->ufM->get(rep) == unionFindIndex) {
        lIslands.push_back(LIsland(sizeRep, rep));      
      }
    }
  }
  MIsland() {}
  MIsland(const MIsland &l) : lIslands(l.lIslands), rectilinear(l.rectilinear), sizeRep(l.sizeRep) {
    for(unsigned int i = 0; i < sizeRep; ++i) {
      representative[i] = l.representative[i];
    }    
  }
};
struct SIsland {
  std::vector<MIsland> mIslands;
  unsigned int sizeRep;
  unsigned short representative[5];

  SIsland(AngleMapping *a, uint16_t unionFindIndex, unsigned short const * const init) : sizeRep(a->numAngles) {
    for(unsigned int i = 0; i < sizeRep; ++i)
      representative[i] = init[i];

    // Add all M-islands:
    for(unsigned int i = 0; i < a->ufM->numReducedUnions; ++i) {
      const uint16_t unionI = a->ufM->reducedUnions[i];
      unsigned short rep[5];
      a->ufM->getRepresentative(unionI, rep);
      assert(a->S[a->ufM->indexOf(rep)]);
      if(a->ufS->get(rep) == unionFindIndex) {
        mIslands.push_back(MIsland(a, unionI, rep));      
      }
    }
  }
  SIsland() {}
  SIsland(const SIsland &l) : mIslands(l.mIslands), sizeRep(l.sizeRep) {
    for(unsigned int i = 0; i < sizeRep; ++i) {
      representative[i] = l.representative[i];
    }    
  }
};

#endif // ANGLEMAPPING_H
