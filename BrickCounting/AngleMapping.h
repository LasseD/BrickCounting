#ifndef ANGLEMAPPING_H
#define ANGLEMAPPING_H

#include <stdint.h>
#include "ConnectionPoint.h"
#include "Configuration.hpp"
#include "ConfigurationEncoder.h"
#include "UnionFind.h"

#define STEPS_0 0
#define STEPS_1 203
#define STEPS_2 370
#define STEPS_3 538

typedef unsigned long long counter;

struct SIsland; // forward declaration.

/*
Mapping angles (See README):
Size 0 => Granularity 0 - 0 => 1 step.
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
  unsigned int angleTypes[5]; // Connection(aka. angle) -> 0, 1, 2, or 3.
  unsigned short angleSteps[5]; // Connection(aka. angle) -> 1, 203, 370, or 538.
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
  uint64_t smlIndex(const Position &p) const;

private:
  void reportProblematic(const Position &p, int mIslandI, int mIslandTotal, int lIslandTotal, std::vector<std::vector<Connection> > &toLdr) const;
  void findNewExtremeConfigurations(std::set<Encoding> &rect, std::set<Encoding> &nonRect, std::vector<std::vector<Connection> > &toLdr);
  void evalSML(unsigned int angleI, Position &p);
  void findIslands(std::multimap<Encoding, SIsland> &sIslands, std::set<Encoding> &keys);
  void findExtremeConfigurations(unsigned int angleI, Position &p, bool allZero, std::set<Encoding> &rect, std::set<Encoding> &nonRect, std::vector<std::vector<Connection> > &toLdr);
  void setupAngleTypes();
  Configuration getConfiguration(const Position &p) const;
  std::vector<Connection> getConfigurationConnections(const Position &p) const;
  bool isRectilinear(const Position &p) const;
};

struct LIsland {
  unsigned int sizeRep;
  Position representative;

  LIsland(unsigned int sizeRep, const Position &p) : sizeRep(sizeRep), representative(p) {}
  LIsland() {}
  LIsland(const LIsland &l) : sizeRep(l.sizeRep), representative(l.representative) {}
};
struct MIsland {
  std::vector<LIsland> lIslands;
  bool rectilinear;
  unsigned int sizeRep;
  Position representative;

  MIsland(AngleMapping *a, uint16_t unionFindIndex, const Position &p) : sizeRep(a->numAngles), representative(p) {
    rectilinear = a->ufM->get(p) == unionFindIndex;
    // Add all L-islands:
    for(unsigned int i = 0; i < a->ufL->numReducedUnions; ++i) {
      const uint16_t unionI = a->ufL->reducedUnions[i];
      Position rep;
      a->ufL->getRepresentative(unionI, rep);
      assert(a->M[a->ufL->indexOf(rep)]);
      if(a->ufM->get(rep) == unionFindIndex) {
        lIslands.push_back(LIsland(sizeRep, rep));      
      }
    }
  }
  MIsland() {}
  MIsland(const MIsland &l) : lIslands(l.lIslands), rectilinear(l.rectilinear), sizeRep(l.sizeRep), representative(l.representative) {}
};
struct SIsland {
  std::vector<MIsland> mIslands;
  unsigned int sizeRep;
  Position representative;

  SIsland(AngleMapping *a, uint16_t unionFindIndex, const Position &p) : sizeRep(a->numAngles), representative(p) {
    // Add all M-islands:
    for(unsigned int i = 0; i < a->ufM->numReducedUnions; ++i) {
      const uint16_t unionI = a->ufM->reducedUnions[i];
      Position rep;
      a->ufM->getRepresentative(unionI, rep);
      assert(a->S[a->ufM->indexOf(rep)]);
      if(a->ufS->get(rep) == unionFindIndex) {
        mIslands.push_back(MIsland(a, unionI, rep));      
      }
    }
  }
  SIsland() {}
  SIsland(const SIsland &l) : mIslands(l.mIslands), sizeRep(l.sizeRep), representative(l.representative) {}
};

#endif // ANGLEMAPPING_H
