#include "ConfigurationEncoder.h"

ConfigurationEncoder::ConfigurationEncoder(const std::vector<FatSCC> &combination) { 
  fatSccSize = combination.size();
#ifdef _TRACE
  std::cout << "INIT ConfigurationEncoder(";
  for(unsigned int i = 0; i < fatSccSize; ++i)
    std::cout << combination[i] << ",";
  std::cout << ")" << std::endl;
#endif
  for(unsigned int i = 0; i < fatSccSize; ++i)
    fatSccs[i] = combination[i];
  std::sort(fatSccs, &fatSccs[fatSccSize]);
  // Construct compressed lists:
  int compressedI = 0;
  for(unsigned int i = 0; i < fatSccSize; ++i) {
    FatSCC &fatScc = fatSccs[i];
    for(int j = 0; j < fatScc.size; ++j) {
      compressedToIdentifier[compressedI] = BrickIdentifier(fatScc.index, j, i);
      identifierToCompressed[6*i + j] = compressedI;
      ++compressedI;
    }      
  }
  // Construct duplicate mapping:
  duplicateMapping[0] = 0;
  for(unsigned int i = 1; i < fatSccSize; ++i) {
    if(fatSccs[i-1] == fatSccs[i])
      duplicateMapping[i] = duplicateMapping[i-1];
    else
      duplicateMapping[i] = i;
  }
}

void ConfigurationEncoder::rotateSCC(int i, std::vector<ConnectionPoint> *connectionPoints, std::map<ConnectionPoint,IConnectionPair> *connectionMaps) const {
#ifdef _TRACE
  std::cout << "ROTATE SCC " << i << std::endl;
#endif

  std::vector<ConnectionPoint> v(connectionPoints[i]);
  std::map<ConnectionPoint,IConnectionPair> w(connectionMaps[i]);
  connectionPoints[i].clear();
  connectionMaps[i].clear();

  for(std::vector<ConnectionPoint>::const_iterator it = v.begin(); it != v.end(); ++it) {
    // Fix connection point:
    const ConnectionPoint &cp = *it;
    ConnectionPoint rcp(cp, fatSccs[i].rotationBrickPosition);
    int rcpi = fatSccs[i].getBrickIndex(rcp.brick);

    connectionPoints[i].push_back(rcp);
#ifdef _TRACE
    std::cout << " CP " << cp << "->" << rcp << std::endl;
#endif

    // Fix the two connections:
    IConnectionPair c = w[cp];
    bool rotateFirstComponent = c.first.first.configurationSCCI == i;
    const IConnectionPoint &icp2 = rotateFirstComponent ? c.second : c.first;
    int j = icp2.first.configurationSCCI;
    assert(connectionMaps[j].find(icp2.second) != connectionMaps[j].end());
#ifdef _TRACE
    std::cout << " " << c;
#endif
    if(rotateFirstComponent) {
      c.first.second = rcp;
      c.first.first.sccBrickI = rcpi;
    }
    else {
      c.second.second = rcp;
      c.second.first.sccBrickI = rcpi;
    }
#ifdef _TRACE
    std::cout << "->" << c << std::endl;
    std::cout << "  " << i << "[" << cp << "]: " << w[cp] << std::endl;
    std::cout << "  " << j << "[" << icp2.second << "]: " << connectionMaps[j][icp2.second] << std::endl;
#endif
    connectionMaps[i].insert(std::make_pair(rcp, c));
    connectionMaps[j].erase(icp2.second);
    connectionMaps[j].insert(std::make_pair(icp2.second, c));
  }

  std::sort(connectionPoints[i].begin(), connectionPoints[i].end());
}

/*
When encoding:
- perform permutation so that SCC of same size,diskIndex are ordered by visiting order.
- Rotate rotationally symmetric SCCs so that they are initially visited at the minimally rotated position.
*/
uint64_t ConfigurationEncoder::encode(unsigned int baseIndex, bool rotate, std::vector<ConnectionPoint> *connectionPoints, std::map<ConnectionPoint,IConnectionPair> *connectionMaps) const {    
  // Set up permutations and rotations:
  int perm[6];
  perm[baseIndex] = 0;
  if(rotate) {
    rotateSCC(baseIndex, connectionPoints, connectionMaps);
  }
  // DuplicateMappingCounter to allow incrementing after mapping duplicates:
  unsigned int duplicateMappingCounters[6];
  duplicateMappingCounters[0] = 1; // because of baseIndex already being mapped.
  for(unsigned int i = 1; i < fatSccSize; ++i) {
    duplicateMappingCounters[i] = duplicateMapping[i];
  }
#ifdef _TRACE
  std::cout << " INIT encode(baseIndex=" << baseIndex << ",rotate=" << rotate << ")" << std::endl;
  std::cout << " Initial setup structures:" << std::endl;
  std::cout << "  duplicateMapping: ";
  for(unsigned int i = 0; i < fatSccSize; ++i) {
    std::cout << i << "->" << duplicateMapping[i] << ", ";
  }
  std::cout << std::endl;    
  // connection point structures:
  std::cout << "Connections:" << std::endl;
  for(unsigned int i = 0; i < fatSccSize; ++i) {
    std::cout << " " << i << ": " << std::endl;
    for(std::vector<ConnectionPoint>::const_iterator it = connectionPoints[i].begin(); it != connectionPoints[i].end(); ++it) {
      assert(connectionMaps[i].find(*it) != connectionMaps[i].end());
      IConnectionPair connection = connectionMaps[i][*it];
      std::cout << "  " << *it << ": " << connection << std::endl;
    }
  }
#endif

  // Compute connections to encode + encodedI on sccs:
  std::set<unsigned int> unencoded;
  for(unsigned int i = 0; i < fatSccSize; ++i) {
    if(i != baseIndex)
      unencoded.insert(i);
  }
  std::vector<IConnectionPair> toEncode; // to output
  std::queue<unsigned int> queue; // ready to be visited.
  queue.push(baseIndex);

  while(!unencoded.empty()) {
    assert(!queue.empty());
    unsigned int fatSccI = queue.front();
    queue.pop();
#ifdef _TRACE
    std::cout << "Iterating from queue: " << fatSccI << std::endl;
#endif

    std::vector<ConnectionPoint> &v = connectionPoints[fatSccI];
    for(std::vector<ConnectionPoint>::const_iterator it = v.begin(); it != v.end(); ++it) {
      IConnectionPair connection = connectionMaps[fatSccI][*it];
      if(connection.first.first.configurationSCCI != (int)fatSccI) { // swap if necessary.
#ifdef _TRACE
        std::cout << "(swapping)" << std::endl;
#endif
        std::swap(connection.first, connection.second);
      }
      IConnectionPoint &ip2 = connection.second;
      unsigned int fatSccI2 = ip2.first.configurationSCCI;
      ConnectionPoint &p2 = ip2.second;
#ifdef _TRACE
      std::cout << " IConnectionPair: " << connection.first << ", " << ip2 << std::endl;
      std::cout << " Connecting to configuration scc " << fatSccI2 << std::endl;
#endif

      if(unencoded.find(fatSccI2) != unencoded.end()) { // Connection to new scc found!
        // Check if rotation necessary!
        if(!fatSccs[fatSccI2].isRotationallyMinimal(p2)) {
          rotateSCC(fatSccI2, connectionPoints, connectionMaps);
          // Update already extracted connection data:
          p2 = ConnectionPoint(p2, fatSccs[fatSccI2].rotationBrickPosition);
          ip2.first.sccBrickI = fatSccs[fatSccI2].getBrickIndex(p2.brick);
        }
#ifdef _TRACE
        std::cout << " SCC added to pool: " << fatSccI2 << std::endl;
#endif

        toEncode.push_back(connection);
        queue.push(fatSccI2);
        unencoded.erase(fatSccI2);

        // If same as prev SCC, minimize index (add final index to perm):
        unsigned int mappedI = duplicateMapping[fatSccI2];
        perm[fatSccI2] = duplicateMappingCounters[mappedI]++;
      }
    }
  }

  /*
  Encoding algorithm for a connection: 
  -- aboveBrickI(3 bit), 
  -- aboveConnectionPointType(2 bit) 
  -- -||- below
  */
  uint64_t encoded = 0;
  for(std::vector<IConnectionPair>::iterator it = toEncode.begin(); it != toEncode.end(); ++it) {
    // Ensure above, then below
    IConnectionPair &c = *it;
    if(!c.first.second.above)
      std::swap(c.first, c.second);
    // Blow up the IConnectionPair... again:
    const IConnectionPoint &ip1 = c.first;
    const IConnectionPoint &ip2 = c.second;
    const BrickIdentifier &i1 =ip1.first;
    const BrickIdentifier &i2 =ip2.first;
    // Permute indices!
    int aboveI = 6*perm[i1.configurationSCCI] + i1.sccBrickI;
    int belowI = 6*perm[i2.configurationSCCI] + i2.sccBrickI;
    const ConnectionPoint &cpAbove = ip1.second;
    const ConnectionPoint &cpBelow = ip2.second;
    // Perform encoding:
    encoded = (encoded << 4) + identifierToCompressed[aboveI];
    encoded = (encoded << 2) + (int)(cpAbove.type);
    encoded = (encoded << 4) + identifierToCompressed[belowI];
    encoded = (encoded << 2) + (int)(cpBelow.type);
  }

#ifdef _TRACE
  std::cout << "..ConfigurationEncoder::Encoded " << toEncode.size() << " connections: " << encoded << std::endl;
#endif
  return encoded;
}

/*
Setup:
- The sccs are sorted by size,diskI (done in constructor).
- The connection points of connections are sorted and listed for each scc.

Min finding Algorithms:
- base scc index (encodedI) = 0.
- iterate on connections. Any found index ++.
- Now iterate on found of base. 

Encoding algorithm: 
- Encode connections ordered by brick::encodedI, connectionPoint of brick.
- Encode a connection: 
-- aboveBrickI(3 bit), 
-- aboveConnectionPointType(2 bit) 
-- belowBrickI(3 bit), 
-- belowConnectionPointType(2 bit) 
-- => 5*10 bit < 64 bit.

Additions:
- If multiple min(size,diskI): Try all min.
- If min is rotationally symmetric, try turned 180.    
*/
uint64_t ConfigurationEncoder::encode(const IConnectionPairList &list) const {
#ifdef _TRACE
  std::cout << " INIT encode(" << list << ")" << std::endl;
#endif
  // Setup:
  std::vector<ConnectionPoint> connectionPoints[6];
  std::map<ConnectionPoint,IConnectionPair> connectionMaps[6];

  // Assign connections to lists, then sort them:
  for(std::set<IConnectionPair>::const_iterator it = list.begin(); it != list.end(); ++it) {
    // Blow up the IConnectionPair:
    const IConnectionPair &c = *it;
    const IConnectionPoint &ip1 = c.first;
    const IConnectionPoint &ip2 = c.second;
    int id1 = ip1.first.configurationSCCI;
    int id2 = ip2.first.configurationSCCI;
    const ConnectionPoint &cp1 = ip1.second;
    const ConnectionPoint &cp2 = ip2.second;
    // Add connection points:
    connectionPoints[id1].push_back(cp1);
    connectionPoints[id2].push_back(cp2);
    // Add connection mappings: 
    connectionMaps[id1].insert(std::make_pair(cp1,c));
    connectionMaps[id2].insert(std::make_pair(cp2,c));
  }
  // Sort vectors:
  for(unsigned int i = 0; i < fatSccSize; ++i)
    std::sort(connectionPoints[i].begin(), connectionPoints[i].end());

  uint64_t minEncoded = 0xFFFFFFFFFFFFFFFF;
  for(unsigned int i = 0; i < fatSccSize; ++i) {
    if(!(fatSccs[i] == fatSccs[0]))
      break;
    // Only run if connections are minimal:
    bool minimal = false;
    bool rotSym = false;
    fatSccs[i].getRotationalInformation(connectionPoints[i], minimal, rotSym);
    uint64_t encoded = encode(i, false, connectionPoints, connectionMaps);
    if(encoded < minEncoded)
      minEncoded = encoded;
    // Rotate and run again if necessary:
    if(fatSccs[i].isRotationallySymmetric) {
      uint64_t encoded = encode(i, true, connectionPoints, connectionMaps);
      if(encoded < minEncoded)
        minEncoded = encoded;
    }
#ifdef _DEBUG
    else if(fatSccs[i].isRotationallySymmetric) {
      uint64_t encoded = encode(i, true, connectionPoints, connectionMaps);
      if(encoded < minEncoded) {
        std::cout << "FAIL! Init=" << i << ": " << minEncoded << "->" << encoded << std::endl;
        assert(false);
      }
    }
#endif

  }
  return minEncoded;
}

void ConfigurationEncoder::decode(uint64_t encoded, IConnectionPairList &list) const {
#ifdef _TRACE
  std::cout << "..ConfigurationEncoder::decode(" << encoded << "):" << std::endl;
#endif
  for(unsigned int i = 0; i < fatSccSize-1; ++i) {
    // decode parts:
    ConnectionPointType cp2Type = (ConnectionPointType)(encoded & 0x03);
    encoded >>= 2;
    int cp2Index = encoded & 0x0F;
    encoded >>= 4;
    ConnectionPointType cp1Type = (ConnectionPointType)(encoded & 0x03);
    encoded >>= 2;
    int cp1Index = encoded & 0x0F;
    encoded >>= 4;

    // Reconstruct IConnectionPair:
    const BrickIdentifier &ib1 = compressedToIdentifier[cp1Index];
    RectilinearBrick rb1 = fatSccs[ib1.configurationSCCI][ib1.sccBrickI];
    ConnectionPoint cp1(cp1Type, rb1, true, ib1.sccBrickI);
    const BrickIdentifier &ib2 = compressedToIdentifier[cp2Index];
    RectilinearBrick rb2 = fatSccs[ib2.configurationSCCI][ib2.sccBrickI];
    ConnectionPoint cp2(cp2Type, rb2, false, ib2.sccBrickI);

    IConnectionPair c(IConnectionPoint(ib1,cp1),IConnectionPoint(ib2,cp2));
    list.insert(c);
  }

#ifdef _TRACE
  std::cout << "..decode => " << list << std::endl;
#endif
}

void ConfigurationEncoder::testCodec(const IConnectionPairList &list1) const {
#ifdef _DEBUG
#ifdef _TRACE
  std::cout << "INIT testCodec(" << list1 << ")" << std::endl;
#endif
  std::vector<Connection> cs;
  for(std::set<IConnectionPair>::const_iterator it = list1.begin(); it != list1.end(); ++it)
    cs.push_back(Connection(*it, Angle()));
  Configuration c1(fatSccs, cs);
#ifdef _TRACE
  std::cout << "Configuration to check on: " << c1 << std::endl;
#endif

  FatSCC min1 = c1.toMinSCC();
#ifdef _TRACE
  std::cout << " min1: " << min1 << std::endl;
#endif

  uint64_t encoded = encode(list1);
#ifdef _TRACE
  std::cout << " encoded: " << encoded << std::endl;
#endif
  IConnectionPairList list2;
  decode(encoded, list2);
#ifdef _TRACE
  std::cout << " decoded: " << list2 << std::endl;
#endif

  cs.clear();
  for(std::set<IConnectionPair>::const_iterator it = list2.begin(); it != list2.end(); ++it)
    cs.push_back(Connection(*it, Angle()));
  Configuration c2(fatSccs, cs);
#ifdef _TRACE
  std::cout << " c2: " << c2 << std::endl;
#endif

  FatSCC min2 = c2.toMinSCC();
#ifdef _TRACE
  std::cout << " min1: " << min1 << std::endl;
  std::cout << " min2: " << min2 << std::endl;
#endif

  assert(min1 == min2);
#endif
}