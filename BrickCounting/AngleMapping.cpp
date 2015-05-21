#include "AngleMapping.h"

#include "Brick.h"

AngleMapping::AngleMapping(FatSCC const * const sccs, int numScc, const std::vector<TinyConnection> &cs) : numAngles(numScc-1), numBricks(0) {
  // Simple copying:
  for(int i = 0; i < numScc; ++i) {
    this->sccs[i] = sccs[i];
    numBricks += sccs[i].size;
  }
  // Connection info:
  int i = 0;
  for(std::vector<TinyConnection>::const_iterator it = cs.begin(); it != cs.end(); ++it) {
    const TinyConnection &c = *it;
    const IConnectionPoint &icp1 = c.first;
    const IConnectionPoint &icp2 = c.second;
    points[i++] = icp1;
    points[i++] = icp2;
  }
  
  setupAngleTypes();

  // sizeMappings:
  sizeMappings = 1;
  for(i = 0; i < numAngles; ++i) {
    switch(angleTypes[i]) {
    case 1:
      sizeMappings *= STEPS_1;
    case 2:
      sizeMappings *= STEPS_2;
    case 3:
      sizeMappings *= STEPS_3;
    }
  }
#ifdef _INFO
  std::cout << "Angle mappings of size " << sizeMappings << std::endl;
#endif
  
  // S, M & L:
  S = new bool[sizeMappings];
  M = new bool[sizeMappings];
  L = new bool[sizeMappings];
}

AngleMapping::~AngleMapping() {
  delete[] S;
  delete[] M;
  delete[] L;
}

void AngleMapping::setupAngleTypes() {
  // First set up a counter for number of connections to sccs:
  int fatSccConnectionCounts[6] = {0,0,0,0,0,0}; // scc -> connections to it.
  int lastConnectedTo[6]; // scc -> last seen connected scc index.
  for(int i = 0; i < numAngles; ++i) {
    int i1 = points[2*i].first.configurationSCCI;
    int i2 = points[2*i+1].first.configurationSCCI;
    fatSccConnectionCounts[i1]++;
    fatSccConnectionCounts[i2]++;
    lastConnectedTo[i1] = i2;
    lastConnectedTo[i2] = i1;
  }

  // Set up sizes (to be reduced later):
  int fatSccSizes[6] = {0,0,0,0,0,0}; // scc -> size of it.
  for(int i = 0; i < numAngles+1; ++i) {
    fatSccSizes[i] = sccs[i].size;
  }

  // Angle types computed based on size of components on side of turn point:
  for(int i = 0; i < numAngles; ++i)
    angleTypes[i] = 3;
  // Minimize angles connecting leaves:
  for(int i = 0; i < numAngles*2; ++i) {
    int angleI = i/2;
    int sccI = points[i].first.configurationSCCI;
    if(fatSccConnectionCounts[sccI] == 1)
      angleTypes[angleI] = MIN(fatSccSizes[sccI], numBricks-fatSccSizes[sccI]);
  }
  // Reduce graph by merging leaves into parents:
  for(int i = 0; i < numAngles+1; ++i) {
    if(fatSccConnectionCounts[i] == 1) {
      int otherI = lastConnectedTo[i];
      fatSccSizes[otherI] += fatSccSizes[i];
      fatSccSizes[i] = 0;
      --fatSccConnectionCounts[otherI];
      fatSccConnectionCounts[i] = 0; // same as --
    }
  }
  // Perform initial minimizing again:
  for(int i = 0; i < numAngles*2; ++i) {
    int angleI = i/2;
    int sccI = points[i].first.configurationSCCI;
    if(fatSccConnectionCounts[sccI] == 1)
      angleTypes[angleI] = MIN(fatSccSizes[sccI], numBricks-fatSccSizes[sccI]);
  }
}

void AngleMapping::run(counter &attempts, counter &rectilinear, counter &nonRectilinearConnectionLists, counter &models, counter &problematic) {
  // TODO!
  // 1: Evaluate S,M,L
  // - TODO: Create SML-index -> angles function

  // 2: Walk through SML for reporting
  // TODO: Any additional structures required?
  // TODO: Update codec to include angles - use separate codec for this.
  // TODO: Add remaining steps from README.md
}
