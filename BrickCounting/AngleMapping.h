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
#define BOOST_STAGES 4
#define MAX_LOAD_FACTOR 3

typedef unsigned long long counter;

struct SIsland; // forward declaration.

namespace math {
  void intervalToArray(const IntervalList &l, bool *array, unsigned int sizeArray);
}

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
  uint32_t sizeMappings; 
  FatSCC sccs[6];
  math::IntervalListVector *SS, *MM, *LL;

  UnionFind::IntervalUnionFind *ufS, *ufM, *ufL;
  IConnectionPoint points[10];
  unsigned int angleTypes[5]; // Connection(aka. angle) -> 0, 1, 2, or 3.
  unsigned short angleSteps[5]; // Connection(aka. angle) -> 1, 203, 370, or 538.
  const ConfigurationEncoder &encoder;
  uint32_t rectilinearIndex;
  MixedPosition rectilinearPosition;
  counter boosts[BOOST_STAGES];
private:
  std::ofstream &os;

public:
  AngleMapping(FatSCC const * const sccs, int numScc, const std::vector<IConnectionPair> &cs, const ConfigurationEncoder &encoder, std::ofstream &os);
  AngleMapping& operator=(const AngleMapping &tmp) {
    assert(false); // Assignment operator should not be used.
    std::vector<IConnectionPair> cs;
    AngleMapping *ret = new AngleMapping(NULL, 0, cs, tmp.encoder, tmp.os);
    return *ret;
  }
  ~AngleMapping();

  /*
  1) For all possible angles: Comput S,M,L.
  2) Combine regions in S,M,L in order to determine new models.
  */
  void findNewConfigurations(std::set<uint64_t> &nonCyclic, std::set<uint64_t> &cyclic, std::vector<std::vector<Connection> > &manual, std::vector<Configuration> &modelsToPrint, counter &models, std::vector<std::pair<Configuration,uint64_t> > &newRectilinear);

private:
  void reportProblematic(const MixedPosition &p, int mIslandI, int mIslandTotal, int lIslandTotal, std::vector<std::vector<Connection> > &toLdr, bool includeMappingFile) const;
  //void findNewExtremeConfigurations(std::set<Encoding> &rect, std::set<Encoding> &nonRect, std::vector<std::vector<Connection> > &toLdr);
  void evalSML(unsigned int angleI, uint32_t smlIndex, const Configuration &c, bool noS, bool noM, bool noL);
  void findIslands(std::vector<SIsland> &sIslands);
  //void findExtremeConfigurations(unsigned int angleI, Position &p, bool allZero, std::set<Encoding> &rect, std::set<Encoding> &nonRect, std::vector<std::vector<Connection> > &toLdr);
  void setupAngleTypes();
  //Configuration getConfiguration(const Position &p) const;
  Configuration getConfiguration(const MixedPosition &p) const;
  Configuration getConfiguration(const Configuration &baseConfiguration, int angleI, unsigned short angleStep) const;
  std::vector<Connection> getConfigurationConnections(const MixedPosition &p) const;
};

struct MIsland {
  int lIslands;
  bool isRectilinear;
  unsigned int sizeRep;
  MixedPosition representative;

  MIsland(AngleMapping *a, uint32_t unionFindIndex, const MixedPosition &p) : lIslands(0), sizeRep(a->numAngles), representative(p) {
    IntervalList rectilinearList;
    a->MM->get(a->rectilinearIndex, rectilinearList);
    isRectilinear = math::intervalContains(rectilinearList, 0) && a->ufM->getRootForPosition(a->rectilinearPosition) == a->ufM->getRootForPosition(p);
    // Add all L-islands:
    for(std::vector<uint32_t>::const_iterator it = a->ufL->rootsBegin(); it != a->ufL->rootsEnd(); ++it) {
      const uint32_t unionI = *it;
      MixedPosition rep;
      a->ufL->getRepresentativeOfUnion(unionI, rep);
      if(a->ufM->getRootForPosition(rep) == unionFindIndex) {
        ++lIslands;
      }
    }
  }
  MIsland() {}
  MIsland(const MIsland &l) : lIslands(l.lIslands), isRectilinear(l.isRectilinear), sizeRep(l.sizeRep), representative(l.representative) {}
};
struct SIsland {
  std::vector<MIsland> mIslands;
  unsigned int sizeRep;
  MixedPosition representative;
  uint64_t encoding;
  bool isRectilinear, isCyclic;

  SIsland(AngleMapping *a, uint32_t unionFindIndex, const MixedPosition &p, uint64_t encoding, bool isCyclic) : sizeRep(a->numAngles), representative(p), encoding(encoding), isRectilinear(false), isCyclic(isCyclic) {
    // Add all M-islands:
    for(std::vector<uint32_t>::const_iterator it = a->ufM->rootsBegin(); it != a->ufM->rootsEnd(); ++it) {
      const uint32_t unionI = *it;
      MixedPosition rep;
      a->ufM->getRepresentativeOfUnion(unionI, rep);
      if(a->ufS->getRootForPosition(rep) == unionFindIndex) {
        MIsland mIsland(a, unionI, rep);
        mIslands.push_back(mIsland);
        if(mIsland.isRectilinear)
          isRectilinear = true;
      }
    }
  }
  SIsland() {}
  SIsland(const SIsland &l) : mIslands(l.mIslands), sizeRep(l.sizeRep), representative(l.representative), encoding(l.encoding), isRectilinear(l.isRectilinear), isCyclic(l.isCyclic) {}
};

#endif // ANGLEMAPPING_H
