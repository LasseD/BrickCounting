#include "SingleConfigurationManager.h"

SingleConfigurationManager::SingleConfigurationManager(const std::vector<FatSCC> &combination) : combinationSize(combination.size()), attempts(0), rectilinear(0), nonRectilinearConnectionLists(0), models(0), problematic(0)
 {
  std::cout << "Building SingleConfigurationManager on combination: " << std::endl;
  for(std::vector<FatSCC>::const_iterator it = combination.begin(); it != combination.end(); ++it) 
    std::cout << *it << " ";
  std::cout << std::endl;

  for(unsigned int i = 0; i < combinationSize; ++i) {
    prevMustBeChosen[i] = i > 0 && combination[i-1] == combination[i];
    if(prevMustBeChosen[i])
      std::cout << " Prev must be chosen for i=" << i << std::endl;
    this->combination[i] = combination[i];

    // Get all above and below:
    combination[i].getConnectionPoints(above[i], below[i]);
  }  
}

void mergePools(std::vector<IConnectionPoint> &newPool, const std::vector<IConnectionPoint> &oldPool, const std::vector<IConnectionPoint>::const_iterator &itIgnoreFromOld, const std::set<ConnectionPoint> &newElements, unsigned long newSCCI, int newConfigurationSCCI) {
  for(std::vector<IConnectionPoint>::const_iterator it = oldPool.begin(); it != oldPool.end(); ++it) {
    if(it != itIgnoreFromOld)
      newPool.push_back(*it);
  }
  for(std::set<ConnectionPoint>::const_iterator it = newElements.begin(); it != newElements.end(); ++it) {
    newPool.push_back(IConnectionPoint(BrickIdentifier(newSCCI, it->brickI, newConfigurationSCCI),*it));
  }
}

void mergePools(std::vector<IConnectionPoint> &newPool, const std::vector<IConnectionPoint> &oldPool, const std::set<ConnectionPoint>::const_iterator &itIgnoreFromNew, const std::set<ConnectionPoint> &newElements, unsigned long newSCCI, int newConfigurationSCCI) {
  for(std::vector<IConnectionPoint>::const_iterator it = oldPool.begin(); it != oldPool.end(); ++it) {
    newPool.push_back(*it);
  }
  for(std::set<ConnectionPoint>::const_iterator it = newElements.begin(); it != newElements.end(); ++it) {
    if(it != itIgnoreFromNew)
      newPool.push_back(IConnectionPoint(BrickIdentifier(newSCCI, it->brickI, newConfigurationSCCI),*it));
  }
}

bool SingleConfigurationManager::isRotationallyMinimal(const ConnectionList &l) const {
  // Find out if ConnectionList created on rotated SCC is "smaller":
  // Step one: Divide COnnectionPoints for the SCCs:
  std::vector<ConnectionPoint> pointsForSccs[6];
  for(std::set<Connection>::const_iterator it = l.begin(); it != l.end(); ++it) {
    const IConnectionPoint &cp1 = it->p1;
    const IConnectionPoint &cp2 = it->p2;
    pointsForSccs[cp1.first.configurationSCCI].push_back(cp1.second);
    pointsForSccs[cp2.first.configurationSCCI].push_back(cp2.second);
  }
  // Step two: check all Sccs:
  for(unsigned int i = 0; i < combinationSize; ++i) {
    std::sort(pointsForSccs[i].begin(), pointsForSccs[i].end());
    if(!combination[i].isRotationallyMinimal(pointsForSccs[i]))
      return false;
  }
  return true;
}
bool SingleConfigurationManager::isRotationallyMinimal(const std::vector<Connection> &l) const {
  // Find out if ConnectionList created on rotated SCC is "smaller":
  // Step one: Divide COnnectionPoints for the SCCs:
  std::vector<ConnectionPoint> pointsForSccs[6];
  for(std::vector<Connection>::const_iterator it = l.begin(); it != l.end(); ++it) {
    const IConnectionPoint &cp1 = it->p1;
    const IConnectionPoint &cp2 = it->p2;
    pointsForSccs[cp1.first.configurationSCCI].push_back(cp1.second);
    pointsForSccs[cp2.first.configurationSCCI].push_back(cp2.second);
  }
  // Step two: check all Sccs:
  for(unsigned int i = 0; i < combinationSize; ++i) {
    std::sort(pointsForSccs[i].begin(), pointsForSccs[i].end());
    if(!combination[i].isRotationallyMinimal(pointsForSccs[i]))
      return false;
  }
  return true;
}

bool SingleConfigurationManager::connectionListIsUnknown(const ConnectionList &l, PermutationHandler &ph, int* perm, int* colors, unsigned int i, int color) const {
  if(i == combinationSize) {
    /*std::cout << "Combination found: ";
    for(unsigned int i = 0; i < combinationSize; ++i)
      std::cout << perm[i] << " ";
    std::cout << std::endl;//*/
    /*std::cout << "Known lists: ";
    for(std::set<ConnectionList>::const_iterator it = foundConnectionLists.begin(); it != foundConnectionLists.end(); ++it)
      std::cout << "   " << *it << std::endl;//*/

    // Build permuted list:
    ConnectionList permutedList;
    for(std::set<Connection>::const_iterator it = l.begin(); it != l.end(); ++it) {
      Connection c = *it;
      c.p1.first.configurationSCCI = perm[c.p1.first.configurationSCCI];
      c.p2.first.configurationSCCI = perm[c.p2.first.configurationSCCI];
      permutedList.insert(c);
    }
    // test permuted list:
    //std::cout << "Testing permutation: " << std::endl << "   " << permutedList << std::endl;
    return foundConnectionLists.find(permutedList) == foundConnectionLists.end();
  }

  // try all combinations:
  PermutationNode *node = ph.root();
  while(node->next != NULL) {
    PermutationNode *n = node->next;    
    // remove n from permutation:
    node->next = n->next;
    
    perm[i] = n->val;
    int currColor = colors[n->val];
    if(currColor == color) {
      if(!connectionListIsUnknown(l, ph, perm, colors, i+1, currColor)) {
	node->next = n;
	return false;
      }
    }
    else {
      bool ret = connectionListIsUnknown(l, ph, perm, colors, i+1, currColor);
      node->next = n;
      return ret;
    }

    // re-insert in permutation and go to next:
    node->next = n; // n->next is already set (never changes)
    node = n;
  }
  return true;
}

bool SingleConfigurationManager::connectionListIsUnknown(const ConnectionList &l) const {
  // Try all combinations of FatSCC index shuffling:
  int map[6];
  int colors[6];
  colors[0] = 0;
  for(unsigned int i = 0; i < combinationSize; ++i) {
    map[i] = i;
    if(i > 0) {
      if(combination[i].size == combination[i-1].size && combination[i].index == combination[i-1].index)
	colors[i] = colors[i-1];
      else
	colors[i] = colors[i-1]+1;
    }
  }
  PermutationHandler ph(combinationSize);
  return connectionListIsUnknown(l, ph, map, colors, 0, 0);
}

void SingleConfigurationManager::run(std::vector<Connection> &l, const std::vector<IConnectionPoint> &abovePool, const std::vector<IConnectionPoint> &belowPool, bool *remaining, int remainingSize) {
  if(remainingSize == 0) {
    if(!isRotationallyMinimal(l)) {
      //std::cout << "  Not even rotationally minimal!" << std::endl;
      return;
    }
    //std::cout << " Investigating for connections: " << l << std::endl;
    ++attempts;
    Configuration c(combination[0]);
    // Build Configuration and investigate
    for(std::vector<Connection>::const_iterator it = l.begin(); it != l.end(); ++it) {
      c.add(combination[it->p2.first.configurationSCCI], *it);
    }
    
    // Investigate!
    ConnectionList found;
    if(c.isRealizable(found) && isRotationallyMinimal(found)) {
      //std::cout << " Found: " << found << std::endl;
      // Success! 
      // Check if new:
      if(connectionListIsUnknown(found)) {
	++rectilinear;
	foundConnectionLists.insert(found);
	//std::cout << "   Yes! Now found " << foundConnectionLists.size() << " models for this combination!" << std::endl;
      }
    }
    
    return;
  }

  for(unsigned int i = 1; i < combinationSize; ++i) { // i is remaining FatSCC to be used.
    if(!remaining[i] || (prevMustBeChosen[i] && remaining[i-1]))
      continue;
    remaining[i] = false;
    // Add a connection and recurse:
    // All above:
    for(std::vector<IConnectionPoint>::const_iterator it_cp1 = abovePool.begin(); it_cp1 != abovePool.end(); ++it_cp1) {
      for(std::set<ConnectionPoint>::const_iterator it_cp2 = below[i].begin(); it_cp2 != below[i].end(); ++it_cp2) {
	Connection c(*it_cp1, IConnectionPoint(BrickIdentifier(combination[i].index, it_cp2->brickI, i), *it_cp2), 0);
	l.push_back(c);
	std::vector<IConnectionPoint> newAbovePool, newBelowPool;
	mergePools(newAbovePool, abovePool, it_cp1, above[i], combination[i].index, i);
	mergePools(newBelowPool, belowPool, it_cp2, below[i], combination[i].index, i);
	run(l, newAbovePool, newBelowPool, remaining, remainingSize-1);
	l.pop_back();
      }
    }
    
    // All below:
    for(std::vector<IConnectionPoint>::const_iterator it_cp1 = belowPool.begin(); it_cp1 != belowPool.end(); ++it_cp1) {
      for(std::set<ConnectionPoint>::const_iterator it_cp2 = above[i].begin(); it_cp2 != above[i].end(); ++it_cp2) {
	Connection c(*it_cp1, IConnectionPoint(BrickIdentifier(combination[i].index, it_cp2->brickI, i), *it_cp2), 0);
	l.push_back(c);
	std::vector<IConnectionPoint> newAbovePool, newBelowPool;
	mergePools(newBelowPool, belowPool, it_cp1, below[i], combination[i].index, i);
	mergePools(newAbovePool, abovePool, it_cp2, above[i], combination[i].index, i);
	run(l, newAbovePool, newBelowPool, remaining, remainingSize-1);
	l.pop_back();
      }
    }

    remaining[i] = true;
  }
}

void SingleConfigurationManager::run() {
  // Try all combinations!
  std::vector<IConnectionPoint> abovePool, belowPool;
  std::vector<Connection> l;
  bool remaining[6];
  int remainingSize = combinationSize-1;

  remaining[0] = false;
  for(unsigned int i = 1; i < combinationSize; ++i) {
    remaining[i] = true;
  }  
  
  for(std::set<ConnectionPoint>::const_iterator it = above[0].begin(); it != above[0].end(); ++it)
    abovePool.push_back(IConnectionPoint(BrickIdentifier(combination[0].index, it->brickI, 0), *it));
  for(std::set<ConnectionPoint>::const_iterator it = below[0].begin(); it != below[0].end(); ++it)
    belowPool.push_back(IConnectionPoint(BrickIdentifier(combination[0].index, it->brickI, 0), *it));

  run(l, abovePool, belowPool, remaining, remainingSize);
}

void SingleConfigurationManager::printLDRFile(const std::vector<Configuration> &s) const {
  LDRPrinterHandler h;
  
  std::vector<Configuration>::const_iterator it = s.begin();
  for(int i = 0; it != s.end(); ++it, ++i) {
    LDRPrinter const * p = &(*it);
    h.add(p);
  }
  
  h.print("rc/Test");
}

void SingleConfigurationManager::test() {
  // Setup:
  RectilinearBrick b1;
  StronglyConnectedConfiguration<1> scc1;
  RectilinearBrick b2(3, -1, 0, true); // true for horizontal
  StronglyConnectedConfiguration<2> scc2(scc1, b2);
  RectilinearBrick b3(0, 4, 0, false);
  StronglyConnectedConfiguration<3> scc3(scc2, b3);//*/

  // NW: Axle, NE: Hose, SE: PIN

  Configuration rc(FatSCC(scc3, 0));
  std::cout << "Constructing and adding yellow brick: " << std::endl;
  Connection connection(IConnectionPoint(BrickIdentifier(0,2,0), ConnectionPoint(NW, b3, true)), IConnectionPoint(BrickIdentifier(0,1,0), ConnectionPoint(SE, b1, false)), 0);
  rc.add(FatSCC(scc1, 0), connection);

  // Print:
  std::vector<Configuration> s;
  s.push_back(rc);

  LDRPrinterHandler h;
  
  std::vector<Configuration>::const_iterator it = s.begin();
  for(int i = 0; it != s.end(); ++it, ++i) {
    LDRPrinter const * p = &(*it);
    h.add(p);
  }
  
  h.print("rc/Test");
}

