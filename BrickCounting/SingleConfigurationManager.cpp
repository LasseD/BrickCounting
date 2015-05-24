#include "SingleConfigurationManager.h"

#include <sstream>
#include <set>
#include <map>
#include <algorithm>

SingleConfigurationManager::SingleConfigurationManager(const std::vector<FatSCC> &combination) : combinationSize(combination.size()), encoder(combination), attempts(0), rectilinear(0), nonRectilinearIConnectionPairLists(0), models(0), problematic(0)
{
#ifdef _TRACE
  std::cout << "Building SingleConfigurationManager on combination of size " << combinationSize << ": " << std::endl;
  for(std::vector<FatSCC>::const_iterator it = combination.begin(); it != combination.end(); ++it) 
    std::cout << *it << " ";
  std::cout << std::endl;
#endif

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

bool SingleConfigurationManager::isRotationallyMinimal(const IConnectionPairList &l) const {
  // Find out if IConnectionPairList created on rotated SCC is "smaller":
  // Step one: Divide COnnectionPoints for the SCCs:
  std::vector<ConnectionPoint> pointsForSccs[6];
  for(std::set<IConnectionPair>::const_iterator it = l.begin(); it != l.end(); ++it) {
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

void SingleConfigurationManager::run(std::vector<IConnectionPair> &l, const std::vector<IConnectionPoint> &abovePool, const std::vector<IConnectionPoint> &belowPool, bool *remaining, int remainingSize) {
  if(remainingSize == 0) {
    // First check that we have at least not run this combination before:
    IConnectionPairList list;
    for(std::vector<IConnectionPair>::const_iterator it = l.begin(); it != l.end(); ++it)
      list.insert(*it);
    if(!isRotationallyMinimal(list))
      return;

    uint64_t encoded = encoder.encode(list);
    if(investigatedConnectionPairListsEncoded.find(encoded) != investigatedConnectionPairListsEncoded.end())
      return;

    investigatedConnectionPairListsEncoded.insert(encoded);

    AngleMapping angleMapping(combination, combinationSize, l, encoder);
    std::vector<Configuration> newConfigurations = angleMapping.findNewConfigurations(foundCircularConfigurationsEncoded, attempts, rectilinear, nonRectilinearIConnectionPairLists, models, problematic);
    for(std::vector<Configuration>::const_iterator it = newConfigurations.begin(); it != newConfigurations.end(); ++it)
      foundConfigurations.push_back(*it);
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
        l.push_back(IConnectionPair(*it_cp1, ip2));
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
        l.push_back(IConnectionPair(*it_cp1, ip2));
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
#ifdef _TRACE
  std::cout << "INIT SingleConfigurationManager::run()" << std::endl;
#endif

  // Try all combinations!
  std::vector<IConnectionPoint> abovePool, belowPool;
  std::vector<IConnectionPair> l;
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
  if(foundConfigurations.size() == 0)
    return;

  LDRPrinterHandler h;

  std::vector<Configuration> v;
  for(std::vector<Configuration>::const_iterator it = foundConfigurations.begin(); it != foundConfigurations.end(); ++it)
    v.push_back(*it);

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
