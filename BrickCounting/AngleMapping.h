#ifndef ANGLEMAPPING_H
#define ANGLEMAPPING_H

#include <stdint.h>
#include "ConnectionPoint.h"
#include "Configuration.hpp"
#include "ConfigurationEncoder.h"
#include "UnionFind.h"
#include "Util.hpp"

#define STEPS_0 0
#define STEPS_1 203
#define STEPS_2 370
#define STEPS_3 538
#define BOOST_STAGES 4
#define MAX_LOAD_FACTOR 4
// Precision boost multiplier can at most be 60 as UF-structure uses unsigned shorts to manage union-find indices: 65535/(538*2+1) ~= 60.8
#define PRECISION_BOOST_MULTIPLIER 2

typedef unsigned long long counter;

struct SIsland; // forward declaration.
struct MIsland; // forward declaration.

namespace math {
  void intervalToArray(const IntervalList &l, bool *array, unsigned int sizeArray);
}

/*
Mapping angles (See README):
Size 0 => Granularity 0 - 0 => 1 step.
Size 1 => Granularity -203 - 203 => 407 steps.
Size 2 => Granularity -370 - 370 => 741 steps.
Size 3 => Granularity -538 - 538 => 1077 steps.
*/
class AngleMapping {
public:
  unsigned int numAngles, numBricks; //, numScc = numAngles+1;
  uint32_t sizeMappings; 
  FatSCC sccs[6];
  math::IntervalListVector *SS, *MM, *LL;

  IConnectionPoint points[10];
  unsigned int angleTypes[5]; // Connection(aka. angle) -> 0, 1, 2, or 3.
  unsigned short angleSteps[5]; // Connection(aka. angle) -> 1, 203, 370, or 538.
  const ConfigurationEncoder &encoder;
  uint32_t rectilinearIndex;
  MixedPosition rectilinearPosition;
  counter boosts[BOOST_STAGES];
private:
  bool singleFreeAngle, findExtremeAnglesOnly;
  std::ofstream &os;
  bool boostPrecision;

public:
  AngleMapping(FatSCC const * const sccs, int numScc, const util::TinyVector<IConnectionPair, 5> &cs, const ConfigurationEncoder &encoder, std::ofstream &os, bool findExtremeAnglesOnly);
  AngleMapping& operator=(const AngleMapping &tmp) {
    assert(false); // Assignment operator should not be used.
    util::TinyVector<IConnectionPair, 5> cs;
    AngleMapping *ret = new AngleMapping(NULL, 0, cs, tmp.encoder, tmp.os, false); // Not deleted - fails.
    return *ret;
  }
  ~AngleMapping();

  /*
  1) For all possible angles: Comput S,M,L.
  2) Combine regions in S,M,L in order to determine new models.
  */
  void findNewConfigurations(std::set<uint64_t> &nonCyclic, std::set<Encoding> &cyclic, std::vector<util::TinyVector<Connection, 5> > &manual, std::vector<Configuration> &modelsToPrint, counter &models, std::vector<std::pair<Configuration,MIsland> > &newRectilinear, bool stopEarlyIfAnyProblematic, bool &anyProblematic);
  void findNewExtremeConfigurations(std::set<uint64_t> &nonCyclic, std::set<Encoding> &cyclic, std::vector<Configuration> &modelsToPrint, counter &models, counter &rect, std::vector<std::pair<Configuration,Encoding> > &newRectilinear);
  Configuration getConfiguration(const MixedPosition &p) const;
  void setBoostPrecision();

private:
  void reportProblematic(const MixedPosition &p, int mIslandI, int mIslandTotal, int lIslandTotal, std::vector<util::TinyVector<Connection, 5> > &manual, bool includeMappingFile) const;
  void addFoundConfiguration(const Configuration &c, bool rectilinear, std::set<uint64_t> &nonCyclic, std::set<Encoding> &cyclic, std::vector<Configuration> &modelsToPrint, counter &models, counter &rect, std::vector<std::pair<Configuration,Encoding> > &newRectilinear);
  void evalExtremeConfigurations(unsigned int angleI, const Configuration &c, bool rectilinear, std::set<uint64_t> &nonCyclic, std::set<Encoding> &cyclic, std::vector<Configuration> &modelsToPrint, counter &models, counter &rect, std::vector<std::pair<Configuration,Encoding> > &newRectilinear);
  void evalSML(unsigned int angleI, uint32_t smlIndex, const Configuration &c, bool noS, bool noM, bool noL);
  void findIslands(std::vector<SIsland> &sIslands, bool &anyProblematic, const UnionFind::IntervalUnionFind &ufS, const UnionFind::IntervalUnionFind &ufM, const UnionFind::IntervalUnionFind &ufL);
  void setupAngleTypes();
  Configuration getConfiguration(const Configuration &baseConfiguration, double lastAngle) const;
  Configuration getConfiguration(const Configuration &baseConfiguration, int angleI, unsigned short angleStep) const;
  void getConfigurationConnections(const MixedPosition &p, util::TinyVector<Connection, 5> &result) const;
  void init();
};

struct MIsland {
  int lIslands;
  bool isRectilinear, isCyclic;
  unsigned int sizeRep;
  MixedPosition representative;
  Encoding encoding;

  MIsland(AngleMapping *a, uint32_t unionFindIndex, const MixedPosition &p, Encoding encoding, bool isCyclic, const UnionFind::IntervalUnionFind &ufM, const UnionFind::IntervalUnionFind &ufL) : lIslands(0), isRectilinear(false), isCyclic(isCyclic), sizeRep(a->numAngles), representative(p), encoding(encoding) {
    assert(unionFindIndex == ufM.getRootForPosition(p));
    bool encodingUpdated = false;
    IntervalList rectilinearList;
    a->MM->get(a->rectilinearIndex, rectilinearList);
    isRectilinear = math::intervalContains(rectilinearList, 0) && ufM.getRootForPosition(a->rectilinearPosition) == unionFindIndex;
    if(isRectilinear) {
      // Update members to ensure correct encoding:
      util::TinyVector<IConnectionPair, 8> found;
      a->getConfiguration(a->rectilinearPosition).isRealizable<-MOLDING_TOLERANCE_MULTIPLIER>(found);
      this->encoding = a->encoder.encode(found);
      this->isCyclic = found.size() > a->numAngles;
      this->representative = a->rectilinearPosition;
      encodingUpdated = true;
#ifdef _TRACE
      std::cout << "Encoding updated for rectilinear position!" << std::endl;
#endif
    }

    // Add all L-islands:
    for(std::vector<uint32_t>::const_iterator it = ufL.rootsBegin(); it != ufL.rootsEnd(); ++it) {
      const uint32_t unionI = *it;
      MixedPosition rep;
      ufL.getRepresentativeOfUnion(unionI, rep);
      if(ufM.getRootForPosition(rep) == unionFindIndex) {
        if(!encodingUpdated) {
          // Update members to ensure correct encoding:
          util::TinyVector<IConnectionPair, 8> found;
          a->getConfiguration(rep).isRealizable<-MOLDING_TOLERANCE_MULTIPLIER>(found);
          this->encoding = a->encoder.encode(found);
          this->isCyclic = found.size() > a->numAngles;
          this->representative = rep;
          encodingUpdated = true;
#ifdef _TRACE
          std::cout << "Encoding updated!" << std::endl;
#endif
        }

        ++lIslands;
      }
    }
  }
  MIsland() {}
  MIsland(const MIsland &l) : lIslands(l.lIslands), isRectilinear(l.isRectilinear), isCyclic(l.isCyclic), sizeRep(l.sizeRep), representative(l.representative), encoding(l.encoding) {}
};
inline std::ostream& operator<<(std::ostream &os, const MIsland& m) {
  os << "M-Island[encoding=" << m.encoding.first << "/" << m.encoding.second << ",|l-islands|=" << m.lIslands << ",representative=";
  for(unsigned int i = 0; i < m.sizeRep-1; ++i)
    os << m.representative.p[i] << "/";
  os << m.representative.lastAngle;
  if(m.isCyclic)
    os << ",cyclic";
  if(m.isRectilinear)
    os << ",rectilinear";
  os << "]";
  return os;
}

struct SIsland {
  std::vector<MIsland> mIslands;
  unsigned int sizeRep;
  MixedPosition representative;

  bool isProblematic() {
    if(mIslands.size() != 1)
      return true;
    for(std::vector<MIsland>::const_iterator it = mIslands.begin(); it != mIslands.end(); ++it) {
      if(it->lIslands != 1)
        return true;
    }
    return false;
  }

  SIsland(AngleMapping *a, uint32_t unionFindIndex, const MixedPosition &p, const UnionFind::IntervalUnionFind &ufS, const UnionFind::IntervalUnionFind &ufM, const UnionFind::IntervalUnionFind &ufL) : sizeRep(a->numAngles), representative(p) {
    assert(unionFindIndex == ufS.getRootForPosition(p));
    // Add all M-islands:
    for(std::vector<uint32_t>::const_iterator it = ufM.rootsBegin(); it != ufM.rootsEnd(); ++it) {
      const uint32_t unionI = *it;
      MixedPosition rep;
      ufM.getRepresentativeOfUnion(unionI, rep);

      Configuration c = a->getConfiguration(rep);
      util::TinyVector<IConnectionPair, 8> found;
      c.isRealizable<-MOLDING_TOLERANCE_MULTIPLIER>(found);
      bool isCyclic = found.size() > a->numAngles;
      Encoding encoding = a->encoder.encode(found);

      if(ufS.getRootForPosition(rep) == unionFindIndex) {
        MIsland mIsland(a, unionI, rep, encoding, isCyclic, ufM, ufL);
        mIslands.push_back(mIsland);
      }
    }
  }
  SIsland() {}
  SIsland(const SIsland &l) : mIslands(l.mIslands), sizeRep(l.sizeRep), representative(l.representative) {}
};
inline std::ostream& operator<<(std::ostream &os, const SIsland& s) {
  os << "S-Island[representative=";
  for(unsigned int i = 0; i < s.sizeRep-1; ++i)
    os << s.representative.p[i] << "/";
  os << s.representative.lastAngle;
  os << ",M-islands:" << std::endl;
  for(std::vector<MIsland>::const_iterator it = s.mIslands.begin(); it != s.mIslands.end(); ++it)
    os << " " << *it << std::endl;
  os << "]";
  return os;
}

#endif // ANGLEMAPPING_H
