#include "ConnectionPoint.h"
#include "Configuration.hpp"
#include "ConfigurationEncoder.h"

#define STEPS_1 203
#define STEPS_2 370
#define STEPS_3 538

typedef unsigned long long counter;

/*
Mapping angles (See README):
Size 1 => Granularity -203 - 203 => 407 steps.
Size 2 => Granularity -370 - 370 => 741 steps.
Size 2 => Granularity -538 - 538 => 1076 steps.
 */
class AngleMapping {
  unsigned int numAngles, numBricks; //, numScc = numAngles+1;
  unsigned long long sizeMappings; 
  FatSCC sccs[6];
  bool *S, *M, *L; 
  IConnectionPoint points[10];
  unsigned int angleTypes[5]; // Connection(aka. angle) -> 1, 2, or 3.
  unsigned short angleSteps[5]; // Connection(aka. angle) -> 203, 370, or 538.
  const ConfigurationEncoder &encoder;

public:
  AngleMapping(FatSCC const * const sccs, int numScc, const std::vector<IConnectionPair> &cs, const ConfigurationEncoder &encoder);
  ~AngleMapping();

  /*
    1) For all possible angles: Comput S,M,L.
    2) Combine regions in S,M,L in order to determine new models.
   */
  void findNewConfigurations(std::set<Encoding> &rect, std::set<Encoding> &nonRect, std::vector<Configuration> &toLdr, counter &attempts);

private:
  void evalSML(unsigned int angleI, short *angleSteps, counter &attempts);
  void findExtremeConfigurations(unsigned int angleI, short *angleStep, bool allZero, std::set<Encoding> &rect, std::set<Encoding> &nonRect, std::vector<Configuration> &toLdr, counter &attempts);
  bool firstExtreme(unsigned int angleI, short *angleStep, bool allZero, counter &attempts, Configuration &c);
  void setupAngleTypes();
  uint64_t smlIndex(short *angleStep) const;
  Configuration getConfiguration(short *angleStep) const;
  bool isRectilinear(short *angleStep) const;
};
