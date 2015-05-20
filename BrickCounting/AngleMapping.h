#include "ConnectionPoint.h"
#include "Configuration.hpp"

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
public:
  int numAngles, numBricks; //, numScc = numAngles+1;
  unsigned long long sizeMappings; 
  FatSCC sccs[6];
  bool *S, *M, *L; 
  IConnectionPoint points[10];
  int angleTypes[5]; // Connection(aka. angle) -> 1, 2, or 3.

  AngleMapping(FatSCC const * const sccs, int numScc, const std::vector<TinyConnection> &cs);
  ~AngleMapping();

  /*
    1) For all possible angles: Comput S,M,L.
    2) Combine regions in S,M,L in order to determine new models.
   */
  void run(counter &attempts, counter &rectilinear, counter &nonRectilinearConnectionLists, counter &models, counter &problematic);

private:
  void setupAngleTypes();
};