#include "SingleConfigurationManager.h"

#include <sstream>
#include <set>
#include <map>
#include <algorithm>
#include <time.h>

SingleConfigurationManager::SingleConfigurationManager(const std::vector<FatSCC> &combination, std::ofstream &os, bool findExtremeAnglesOnly) : 
    combinationSize((unsigned int)combination.size()), encoder(combination), os(os), attempts(0), models(0), rectilinear(0), findExtremeAnglesOnly(findExtremeAnglesOnly) {
  for(int i = 0; i < BOOST_STAGES; ++i) {
    angleMappingBoosts[i] = 0;
  }
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

void mergePools(std::vector<IConnectionPoint> &newPool, const std::vector<IConnectionPoint> &oldPool, const std::vector<IConnectionPoint>::const_iterator &itIgnoreFromOld, 
                const std::set<ConnectionPoint> &newElements, unsigned long newSCCI, int newConfigurationSCCI) {
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
  // Step one: Divide ConnectionPoints for the SCCs:
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
      return false;
    }
  }
  return true;
}

void SingleConfigurationManager::run(std::vector<IConnectionPair> &l, const std::vector<IConnectionPoint> &abovePool, const std::vector<IConnectionPoint> &belowPool, bool *remaining, int remainingSize) {
  if(remainingSize == 0) {
    // First check that we have at least not run this combination before:
    // If the connections are not even rotationslly minimal, then bail:
    IConnectionPairList list;
    for(std::vector<IConnectionPair>::const_iterator it = l.begin(); it != l.end(); ++it)
      list.insert(*it);
    if(!isRotationallyMinimal(list))
      return;

    // If we have already investigated the encoding (without cycles), then bail:
    uint64_t encoded = encoder.encode(list).first;
    if(investigatedConnectionPairListsEncoded.find(encoded) != investigatedConnectionPairListsEncoded.end())
      return;
    investigatedConnectionPairListsEncoded.insert(encoded);

    // Report status if combination starts with a single brick SCC:
    if(combination[0].size == 1) {
      std::cout << "Single brick string run";
      for(std::vector<IConnectionPair>::const_iterator it = l.begin(); it != l.end(); ++it)
        std::cout << " " << *it;
      std::cout << std::endl;
    }

    ++attempts;
    AngleMapping angleMapping(combination, combinationSize, l, encoder, os, findExtremeAnglesOnly);

    if(!findExtremeAnglesOnly) {
      std::vector<std::pair<Configuration,MIsland> > newRectilinear;
      angleMapping.findNewConfigurations(nonCyclicConfigurations, cyclicConfigurations, manual, modelsToPrint, models, newRectilinear);
      for(int i = 0; i < BOOST_STAGES; ++i) {
        angleMappingBoosts[i] += angleMapping.boosts[i];
      }
      rectilinear += newRectilinear.size();

#ifdef _DEBUG
      MPDPrinter h;
      for(std::vector<std::pair<Configuration,MIsland> >::const_iterator it = newRectilinear.begin(); it != newRectilinear.end(); ++it) {
        const Configuration &c = it->first;
        std::stringstream ss;
        ss << "rc_" << it->second.encoding.first;
        h.add(ss.str(), new Configuration(c)); // OK Be cause we are debugging.
      }
      std::stringstream ss;
      ss << "rc\\rectilinear_configurations_" << encoded;
      h.print(ss.str());
#endif

#ifdef _COMPARE_ALGORITHMS
      for(std::vector<std::pair<Configuration,MIsland> >::const_iterator it = newRectilinear.begin(); it != newRectilinear.end(); ++it) {
        FatSCC min = it->first.toMinSCC();

        if(foundSCCs.find(min) != foundSCCs.end()) {
          std::cout << "Duplicate found (encoding): " << it->second.encoding.first << std::endl;
          std::cout << "Duplicate found (previous encoding): " << foundSCCs.find(min)->second << std::endl;
          std::cout << "Duplicate found (config): " << it->first << std::endl;
          std::cout << "Duplicate found: " << min << std::endl;
          std::cout << "Island info:" << std::endl;
          std::cout << " rectilinear: " << it->second.isRectilinear << std::endl;
          std::cout << " |rep|: " << it->second.sizeRep << std::endl;
          std::cout << " rep: " << std::endl;
          for(unsigned int i = 0; i < it->second.sizeRep; ++i)
            std::cout << "  " << it->second.representative.p[i] << std::endl;
          std::cout << "  " << it->second.representative.lastAngle << std::endl;
          std::cout << "Connections: " << list << std::endl;

          MPDPrinter h;
          Configuration cf(min);
          h.add("duplicate", &cf);
          h.print("duplicate");
          assert(false);std::cerr << "DIE X0010" << std::endl;
          int *die = NULL; die[0] = 42;
        }
        foundSCCs.insert(std::make_pair(min,it->second.encoding.first));
      }
#endif
    }
    else {
      angleMapping.findNewExtremeConfigurations(nonCyclicConfigurations, cyclicConfigurations, models, rectilinear);
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
  time_t startTime, endTime;
  time(&startTime);

#ifdef _DEBUG
  std::cout << "INIT SingleConfigurationManager::run(";
  for(unsigned int i = 0; i < combinationSize; ++i)
    std::cout << combination[i] << " ";
  std::cout << ")" << std::endl;
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
  printManualLDRFiles();
  printLDRFile();
  time(&endTime);
  double seconds = difftime(endTime,startTime);
#ifdef _TRACE
  std::cout << "SingleConfigurationManager::run() INTERNAL RESULTS: " << std::endl;
  std::cout << " investigatedConnectionPairListsEncoded: " << investigatedConnectionPairListsEncoded.size() << std::endl;
  std::cout << " foundRectilinearConfigurationsEncoded: " << rectilinear << std::endl;
  std::cout << " models found: " << models << std::endl;

  // Print boost in debug!
  std::cout << "Boosts performed in AngleMappings:" << std::endl;
  for(int i = 0; i < BOOST_STAGES; ++i) {
    std::cout << " BOOST LEVEL " << (i+1) << ": " << angleMappingBoosts[i] << std::endl;
  }
#endif
  //#ifdef _INFO
  if(seconds <= 1)
    return;
  std::cout << " Single configuration (sizes";
  for(unsigned int i = 0; i < combinationSize; ++i)
    std::cout << " " << combination[i].size;
  std::cout << ") (indices";
  for(unsigned int i = 0; i < combinationSize; ++i)
    std::cout << " " << combination[i].index;
  std::cout << ") handled in " << seconds << " seconds." << std::endl;
  //#endif
}

void SingleConfigurationManager::printLDRFile() const {
  if(modelsToPrint.empty())
    return;

  MPDPrinter h;

  int j = 0;
  for(std::vector<Configuration>::const_iterator it = modelsToPrint.begin(); it != modelsToPrint.end(); ++it, ++j) {
    std::stringstream ss;
    ss << "configuration_" << j;
    h.add(ss.str(), &*it);
  }

  std::stringstream ss;
  int size = 0;
  for(unsigned int i = 0; i < combinationSize; ++i)
    size += combination[i].size;
  ss << "models\\" << size << "\\size_" << size << "_sccsizes";
  for(unsigned int i = 0; i < combinationSize; ++i)
    ss << "_" << combination[i].size;
  ss << "_sccindices";
  for(unsigned int i = 0; i < combinationSize; ++i)
    ss << "_" << combination[i].index;
  h.print(ss.str());    
  std::cout << "Printing to " << ss.str() << std::endl;
}

void SingleConfigurationManager::printManualLDRFile(const std::vector<std::pair<std::string,Configuration> > &v, const std::string &fileName) const {
  MPDPrinter h;  
  for(std::vector<std::pair<std::string,Configuration> >::const_iterator it = v.begin(); it != v.end(); ++it) {
    h.add(it->first, &(it->second));
  }
  h.print(fileName);    
}

void SingleConfigurationManager::printManualLDRFiles() const {
  if(manual.size() == 0)
    return;

  int size = 0;
  for(unsigned int i = 0; i < combinationSize; ++i)
    size += combination[i].size;

  std::string fileName;
  bool first = true;
  std::vector<std::pair<std::string,Configuration> > v;

  // Actual printing:
  for(std::vector<std::vector<Connection> >::const_iterator it = manual.begin(); it != manual.end(); ++it) {
    Configuration c(combination, *it);
    std::stringstream ss1, ss2;
    ss1 << "manual\\" << size << "\\";
    encoder.writeFileName(ss1, *it, false);
    encoder.writeFileName(ss2, *it, true);
    std::string currFileName = ss1.str();

    if(!first && fileName != currFileName) {
      printManualLDRFile(v, fileName);
      v.clear();
    }
    first = false;
    fileName = currFileName;
    v.push_back(std::make_pair(ss2.str(), c));
  }
  printManualLDRFile(v, fileName);
}
