#include "SingleConfigurationManager.h"

#include <sstream>
#include <set>
#include <map>
#include <algorithm>

SingleConfigurationManager::SingleConfigurationManager(const std::vector<FatSCC> &combination) : combinationSize(combination.size()), encoder(combination), attempts(0), rectilinear(0), nonRectilinearConnectionLists(0), models(0), problematic(0)
 {
   /*std::cout << "Building SingleConfigurationManager on combination of size " << combinationSize << ": " << std::endl;
  for(std::vector<FatSCC>::const_iterator it = combination.begin(); it != combination.end(); ++it) 
    std::cout << *it << " ";
  std::cout << std::endl;//*/

  for(unsigned int i = 0; i < combinationSize; ++i) {
    prevMustBeChosen[i] = i > 0 && combination[i-1] == combination[i];
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
  for(std::set<TinyConnection>::const_iterator it = l.begin(); it != l.end(); ++it) {
    const IConnectionPoint &cp1 = it->first;
    const IConnectionPoint &cp2 = it->second;
    pointsForSccs[cp1.first.configurationSCCI].push_back(cp1.second);
    pointsForSccs[cp2.first.configurationSCCI].push_back(cp2.second);
  }
  // Step two: check all Sccs:
  for(unsigned int i = 0; i < combinationSize; ++i) {
    std::sort(pointsForSccs[i].begin(), pointsForSccs[i].end());
    if(!combination[i].isRotationallyMinimal(pointsForSccs[i])) {
      //std::cout << " NOT ROT MIN: " << combination[i] << std::endl;
      return false;
    }
  }
  return true;
}

void SingleConfigurationManager::run(std::vector<Connection> &l, const std::vector<IConnectionPoint> &abovePool, const std::vector<IConnectionPoint> &belowPool, bool *remaining, int remainingSize) {
  if(remainingSize == 0) {
    ++attempts;
    Configuration c(combination[0]);
    // Build Configuration and investigate
    //std::cout << " Investigating for: " << std::endl;
    for(std::vector<Connection>::const_iterator it = l.begin(); it != l.end(); ++it) {
      //std::cout << " - " << *it << std::endl;
      c.add(combination[it->p2.first.configurationSCCI], *it);
    }
    
    // Investigate!
    ConnectionList found;
    if(c.isRealizable(found)) {
      if(!isRotationallyMinimal(found)) {
	return; // The rotationally minimal is eventually found.
      }

      encoder.testCodec(found); // TODO: REM

      // Check if new:
      //std::cout << "Encoding " << found << std::endl;

      uint64_t encoded = encoder.encode(found);

      //std::cout << "Encoding found: " << encoded << std::endl;
      if(foundConnectionsEncoded.find(encoded) == foundConnectionsEncoded.end()) {
	++rectilinear;

	foundConnectionsEncoded.insert(encoded);
	//foundConnectionLists.insert(found);
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
	IConnectionPoint ip2(BrickIdentifier(combination[i].index, it_cp2->brickI, i), *it_cp2);
	Connection c(*it_cp1, ip2, 0);
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
	IConnectionPoint ip2(BrickIdentifier(combination[i].index, it_cp2->brickI, i), *it_cp2);
	Connection c(*it_cp1, ip2, 0);
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
  // Print:
  //printLDRFile();
}

void SingleConfigurationManager::printLDRFile() const {
  LDRPrinterHandler h;
  std::vector<Configuration> v;
  
  for(std::set<ConnectionList>::const_iterator it = foundConnectionLists.begin(); it != foundConnectionLists.end(); ++it) {
    const ConnectionList &l = *it;
    
    // First extract proper connections:
    std::map<std::pair<int,int>, TinyConnection> sortedConnections;
    std::set<int> seenSCCs;
    for(std::set<TinyConnection>::const_iterator it2 = l.begin(); it2 != l.end(); ++it2) {
      TinyConnection cc = *it2;
      if(cc.second.first.configurationSCCI < cc.first.first.configurationSCCI)
	std::swap(cc.first, cc.second);
      int i1 = cc.first.first.configurationSCCI;
      int i2 = cc.second.first.configurationSCCI;
      if(seenSCCs.find(i2) != seenSCCs.end())
	continue; // already seen.
      seenSCCs.insert(i2);
      sortedConnections.insert(std::make_pair(std::make_pair(i1, i2), cc));
    }

    Configuration c(combination[0]);
    // Build Configuration and investigate
    for(std::map<std::pair<int,int>, TinyConnection>::const_iterator it2 = sortedConnections.begin(); it2 != sortedConnections.end(); ++it2) {
      c.add(combination[it2->first.second], Connection(it2->second));
    }
    v.push_back(c);
  }//*/

  /*
  //std::cout << "Printing FatSCCs. Count=" << foundSCCs.size() << std::endl;
  for(std::set<FatSCC>::const_iterator it = foundSCCs.begin(); it != foundSCCs.end(); ++it) {    
    Configuration c(*it);
    v.push_back(c);
  }//*/

  if(v.size() == 0)
    return;
  
  // Actual printing:
  for(std::vector<Configuration>::const_iterator it = v.begin(); it != v.end(); ++it)
    h.add(&(*it));
  std::stringstream ss;
  int size = 0;
  for(unsigned int i = 0; i < combinationSize; ++i)
    size += combination[i].size;
  ss << "rc\\";
  ss << "size" << size << "_sccs" << combinationSize << "_sccsizes";  
  for(unsigned int i = 0; i < combinationSize; ++i)
    ss << "_" << combination[i].size;
  ss << "_sccindices";
  for(unsigned int i = 0; i < combinationSize; ++i)
    ss << "_" << combination[i].index;
  h.print(ss.str());
}
