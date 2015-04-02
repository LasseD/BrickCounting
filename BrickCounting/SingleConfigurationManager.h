#ifndef SINGLE_CONFIGURATION_MANAGER_H
#define SINGLE_CONFIGURATION_MANAGER_H

#include "Configuration.hpp"
#include "ConnectionPoint.h"
#include "StronglyConnectedConfiguration.hpp"

#include <vector>
#include <set>

typedef unsigned long long counter;

struct PermutationNode {
  PermutationNode *next;
  int val;  
};
struct PermutationHandler {
  PermutationNode *nodes;
  
  PermutationHandler(int size) {
    nodes = new PermutationNode[size+1];
    for(int i = 0; i < size; ++i) {
      nodes[i+1].val = i;
      nodes[i].next = &(nodes[i+1]);
    }
    nodes[size].next = NULL;
  }

  ~PermutationHandler() {
    delete[] nodes;
  }

  PermutationNode* root() {
    return &(nodes[0]);
  }
};

class SingleConfigurationManager {
  unsigned int combinationSize;
  FatSCC combination[6];
  std::set<ConnectionList> foundConnectionLists;
  std::set<ConnectionPoint> above[6];
  std::set<ConnectionPoint> below[6]; 
  bool prevMustBeChosen[6];

public:
  SingleConfigurationManager(const std::vector<FatSCC> &combination);

  bool connectionListIsUnknown(const ConnectionList &l, int* perm, bool *rotated, unsigned int i) const;
  bool connectionListIsUnknown(const ConnectionList &l, PermutationHandler &ph, int* perm, int* colors, unsigned int i, int color) const;
  bool connectionListIsUnknown(const ConnectionList &l) const;

  void run(std::vector<Connection> &l, const std::vector<IConnectionPoint> &abovePool, const std::vector<IConnectionPoint> &belowPool, bool *remaining, int remainingSize);
  void run();
  void printLDRFile() const;
  bool isRotationallyMinimal(const ConnectionList &l) const;
  bool isRotationallyMinimal(const std::vector<Connection> &l) const;

  static void test1();
  static void test2();

  counter attempts, rectilinear, nonRectilinearConnectionLists, models, problematic;
};

#endif // SINGLE_CONFIGURATION_MANAGER_H
