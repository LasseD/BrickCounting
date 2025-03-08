#include <iostream>
#include <stdlib.h>
#include "bfs.h"

#ifdef PROFILING
  InvocationMap *invocationCounts;
#endif

/*
  This code base uses an approach inspired by Eilers (2016) to compute all models of n bricks:
  For a given brick in the first (base) layer:
  Let the base brick be the first "BFS Wave" or "wave"
  For each wave:
    Pick 1..|wave| bricks from wave:
      Find next wave and recurse until model contains n bricks.
*/
int main(int argc, char** argv) {
  if(argc != 2) {
    std::cout << "Usage: Run without arguments to count models up to size given as parameter." << std::endl;
    return 1;
  }

  int n = 0;
  char c;
  for(int i = 0; (c = argv[1][i]); i++) {
    n += (c-'0');
  }

  std::cout << "Building models for size " << n << std::endl;
  rectilinear::Combination combination;
  rectilinear::CombinationBuilder b(combination, 0, 1, n);
  b.build();
  b.report();

#ifdef PROFILING
  Profiler::reportInvocations();
#endif
  
  return 0;
}
