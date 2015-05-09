#ifndef CONFIGURATION_HPP
#define CONFIGURATION_HPP

#include "LDRPrinter.h"
#include "RectilinearBrick.h"
#include "ConnectionPoint.h"
#include "StronglyConnectedConfiguration.hpp"

#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <set>
#include <map>
#include <queue>
#include <assert.h> 
#include <iomanip>

typedef std::pair<IConnectionPoint,IConnectionPoint> TinyConnection;

struct Connection {
  IConnectionPoint p1, p2;
  double angle;
  Connection(){}
  Connection(const TinyConnection &c) : p1(c.first), p2(c.second), angle(0) {}
  Connection(const Connection &c) : p1(c.p1), p2(c.p2), angle(c.angle) {}
  Connection(const IConnectionPoint &p1, const IConnectionPoint &p2, double angle) : p1(p1), p2(p2), angle(angle) {} 
  bool operator<(const Connection &c) const {
    if(angle != c.angle)      
      return angle < c.angle;
    if(p1 != c.p1)
      return p1 < c.p1;
    return p2 < c.p2;
  }
  bool operator!=(const Connection &c) const {
    return angle != c.angle || p1 != c.p1 || p2 != c.p2;
  }
  bool operator==(const Connection &c) const {
    return angle == c.angle && p1 == c.p1 && p2 == c.p2;
  }
};

inline std::ostream& operator<<(std::ostream &os, const Connection& c) {
  os << "Connection[" << c.p1 << "-" << c.p2 << ",a=" << c.angle << "]";
  return os;
}

struct ConnectionList {
private:
  std::set<TinyConnection> v;
public:
  ConnectionList() {}
  ConnectionList(const ConnectionList &l) : v(l.v) {}

  bool operator<(const ConnectionList &l) const {
    if(v.size() != l.v.size())      
      return v.size() < l.v.size();
    for(std::set<TinyConnection>::const_iterator it1 = v.begin(), it2 = l.v.begin(); it1 != v.end(); ++it1, ++it2) {
      const TinyConnection &c1 = *it1;
      const TinyConnection &c2 = *it2;
      if(c1 != c2)
	return c1 < c2;
    }
    return false;
  }
  void insert(TinyConnection toInsert) {
    if(toInsert.first.first.configurationSCCI > toInsert.second.first.configurationSCCI) {
      std::swap(toInsert.first, toInsert.second);
    }
    v.insert(toInsert);
  }
  void insert(const Connection &c) {
    TinyConnection toInsert(c.p1, c.p2);
    insert(toInsert);
  }
  std::set<TinyConnection>::const_iterator begin() const {
    return v.begin();
  }
  std::set<TinyConnection>::const_iterator end() const {
    return v.end();
  }
  unsigned int size() const {
    return v.size();
  }
};

inline std::ostream& operator<<(std::ostream &os, const ConnectionList& l) {
  for(std::set<TinyConnection>::const_iterator it = l.begin(); it != l.end(); ++it) {
    os << it->first << "/" << it->second << " ";
  }
  return os;
}

struct IBrick {
  RectilinearBrick rb;
  Brick b;
  BrickIdentifier bi;
  IBrick(const RectilinearBrick &rb, const Brick &b, const BrickIdentifier &bi) : rb(rb), b(b), bi(bi) {}
  IBrick() {}
  IBrick(const IBrick &b) : rb(b.rb), b(b.b), bi(b.bi) {}
};

struct Configuration : public LDRPrinter {
private:
  std::map<int,Brick> origBricks;

public:
  int bricksSize;
  IBrick bricks[6];

  bool isRealizable(ConnectionList &found) const {
    for(int i = 0; i < bricksSize; ++i) {
      const IBrick &ib = bricks[i];
      for(int j = i+1; j < bricksSize; ++j) {
	const IBrick &jb = bricks[j];
	//std::cout << "   isRealizable step " << ib.bi << "/" << ib.b << "<>" << jb.bi << "/" << jb.b << std::endl;
	if(ib.bi.configurationSCCI == jb.bi.configurationSCCI)
	  continue; // from same scc.
	bool connected;
	ConnectionPoint pi, pj;
	if(ib.b.intersects(jb.b, jb.rb, connected, pj, pi, ib.rb)) {
	  if(!connected)
	    return false;
	  pi.brickI = ib.bi.sccBrickI;
	  pj.brickI = jb.bi.sccBrickI;
	  Connection c(IConnectionPoint(ib.bi, pi), IConnectionPoint(jb.bi, pj), 0.0);
	  found.insert(c);
	}
      }
    }
    return true;
  }

  void initSCC(const FatSCC &scc) {
    RectilinearBrick b;
    origBricks.insert(std::make_pair(0,Brick(b)));
    for(int i = 0; i < scc.size; b=scc.otherBricks[i++]) {
      Brick brick(b);
      IBrick ib(b, brick, BrickIdentifier(scc.index, i, 0));
      bricks[bricksSize++] = ib;
    }
  }

  Configuration(const FatSCC &scc) : bricksSize(0) {
    initSCC(scc);
  }

  void add(const FatSCC &scc, const Connection &c) {
    // Get objects of interest:
    int prevBrickI = c.p1.first.configurationSCCI;
    int currBrickI = c.p2.first.configurationSCCI;
    Brick prevOrigBrick = origBricks[prevBrickI];
    const ConnectionPoint &prevPoint = c.p1.second;
    const ConnectionPoint &currPoint = c.p2.second;
    Brick prevBrick(prevOrigBrick, c.p1.second.brick);
    Point prevStud = prevBrick.getStudPosition(prevPoint.type);
    //std::cout << "ADD: Prev: " << prevBrick << " w. stud " << prevStud.X << "," << prevStud.Y << std::endl;

    // Compute position of bricks:
    double angle = prevBrick.angle + M_PI/2*(currPoint.type-prevPoint.type-2) + c.angle;
    int level = prevOrigBrick.level + prevPoint.brick.level() + (prevPoint.above ? 1 : -1);

    //std::cout << "Configuration::add(" << scc << ")" << std::endl;
    RectilinearBrick rb;
    origBricks.insert(std::make_pair(currBrickI, Brick(rb, currPoint, prevStud, angle, level)));
    for(int i = 0; i < scc.size; rb=scc.otherBricks[i++]) {
      Brick brick(rb, currPoint, prevStud, angle, level);
      //std::cout << " ADD brick " << brick << std::endl;

      IBrick ib(rb, brick, BrickIdentifier(scc.index, i, currBrickI));
      bricks[bricksSize++] = ib;
    }
  }

  Configuration(FatSCC const * const sccs, const std::vector<Connection> &cs) : bricksSize(0) {
    initSCC(sccs[0]);

    int unusedConnections = cs.size();
    bool usedConnections[8] = {false,false,false,false,false,false,false,false};
    bool addedSccs[6] = {true,false,false,false,false,false};

    while(unusedConnections > 0) {
      for(unsigned int i = 0; i < cs.size(); ++i) {
	if(usedConnections[i])
	  continue;
	const Connection &c = cs[i];
	int i1 = c.p1.first.configurationSCCI;
	int i2 = c.p2.first.configurationSCCI;
	if(addedSccs[i1]) {
	  usedConnections[i] = true;
	  unusedConnections--;
	  if(!addedSccs[i2]) {
	    add(sccs[i2], c);
	    addedSccs[i2] = true;
	  }
	}
	else if(addedSccs[i2]) {
	  Connection flipped(c);
	  std::swap(flipped.p1, flipped.p2);
	  usedConnections[i] = true;
	  unusedConnections--;
	  add(sccs[i1], flipped);	  
	  addedSccs[i1] = true;
	}
      }
    }
  }

  void toLDR(std::ofstream &os, int x, int y, int ldrColor) const {    
    int colors[6] = {LDR_COLOR_RED, LDR_COLOR_YELLOW, LDR_COLOR_BLUE, 3, 85, LDR_COLOR_BLACK};

    for(int i = 0; i < bricksSize; ++i) {
      bricks[i].b.toLDR(os, x, y, colors[bricks[i].bi.configurationSCCI]);
    }
  }

  FatSCC toMinSCC() const {
    std::vector<Brick> v;
    for(int i = 0; i < bricksSize; ++i)
      v.push_back(bricks[i].b);
    FatSCC ret(v);
    ret = ret.rotateToMin();
    return ret;
  }
};

inline std::ostream& operator<<(std::ostream& os, const Configuration& c) {
  os << "CONF[size=" << c.bricksSize << ",";
  for(int i = 0; i < c.bricksSize; ++i)
    os << c.bricks[i].b;
  os << "]";
  return os;
}

/*
  Used for uniquely hashing a configuration into a uint64_t.
  Performs necessary rotations and permutations on FatSCCs of configuration in order to minimize hash.
 */
struct ConfigurationEncoder {
  FatSCC fatSccs[6];
  unsigned int fatSccSize;
  // Compressed index for bricks enables brick ID saving using 3 bit:
  BrickIdentifier compressedToIdentifier[6];
  int identifierToCompressed[36]; // 6*configurationI + brickI in scc.
  // Duplicate mapping assists in quick lookup for running indices of duplicate SCCs:
  unsigned int duplicateMapping[6];

  ConfigurationEncoder(const std::vector<FatSCC> &combination) { 
    fatSccSize = combination.size();
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

  void rotateSCC(int i, std::vector<ConnectionPoint> *connectionPoints, std::map<ConnectionPoint,TinyConnection> *connectionMaps) const {
#ifdef _TRACE
    std::cout << "INIT rotateSCC(" << i << ", ...)" << std::endl;
#endif

    std::vector<ConnectionPoint> v(connectionPoints[i]);
    connectionPoints[i].clear();
    
    for(std::vector<ConnectionPoint>::const_iterator it = v.begin(); it != v.end(); ++it) {
      // Fix connection point:
      const ConnectionPoint &cp = *it;
      ConnectionPoint rcp(cp, fatSccs[i].rotationBrickPosition);
      connectionPoints[i].push_back(rcp);

      // Fix the two connections:
      TinyConnection c = connectionMaps[i][cp];
      bool rotateFirstComponent = c.first.first.configurationSCCI == i;
      const IConnectionPoint &icp2 = rotateFirstComponent ? c.second : c.first;
      int j = icp2.first.configurationSCCI;
      connectionMaps[i].erase(cp);
      connectionMaps[j].erase(rotateFirstComponent ? c.second.second : c.first.second);
      if(rotateFirstComponent)
	c.first.second = rcp;
      else
	c.second.second = rcp;
      connectionMaps[i].insert(std::make_pair(rcp, c));
      connectionMaps[j].insert(std::make_pair(icp2.second, c));
    }

    std::sort(connectionPoints[i].begin(), connectionPoints[i].end());
  }

  /*
    When encoding:
    - perform permutation so that SCC of same size,diskIndex are ordered by visiting order.
    - Rotate rotationally symmetric SCCs so that they are initially visited at the minimally rotated position.
   */
  uint64_t encode(unsigned int baseIndex, bool rotate, std::vector<ConnectionPoint> *connectionPoints, std::map<ConnectionPoint,TinyConnection> *connectionMaps) const {    
#ifdef _TRACE
    std::cout << " INIT encode(baseIndex=" << baseIndex << ",rotate=" << rotate << ")" << std::endl;
#endif
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
    std::cout << " Initial setup structures:" << std::endl;
    std::cout << "  duplicateMapping: ";
    for(unsigned int i = 0; i < fatSccSize; ++i) {
      std::cout << i << "->" << duplicateMapping[i] << ", ";
    }
    std::cout << std::endl;    
    std::cout << "  duplicateMappingCounters: ";
    for(unsigned int i = 0; i < fatSccSize; ++i) {
      std::cout << i << "->" << duplicateMappingCounters[i] << ", ";
    }
    std::cout << std::endl;    
#endif

    // Compute connections to encode + encodedI on sccs:
    std::set<unsigned int> unencoded;
    for(unsigned int i = 0; i < fatSccSize; ++i) {
      if(i != baseIndex)
	unencoded.insert(i);
    }
    std::vector<TinyConnection> toEncode; // to output
    std::queue<unsigned int> queue; // ready to be visited.
    queue.push(baseIndex);

    while(!unencoded.empty()) {
      unsigned int fatSccI = queue.front();
      assert(!queue.empty());
      queue.pop();
#ifdef _TRACE
      std::cout << "Iterating from queue: " << fatSccI << std::endl;
#endif

      std::vector<ConnectionPoint> &v = connectionPoints[fatSccI];
      for(std::vector<ConnectionPoint>::const_iterator it = v.begin(); it != v.end(); ++it) {
	TinyConnection connection = connectionMaps[fatSccI][*it];
	if(connection.first.first.configurationSCCI == (int)fatSccI) { // swap if necessary.
#ifdef _TRACE
	  std::cout << "(swapping)" << std::endl;
#endif
	  std::swap(connection.first, connection.second);
	}
#ifdef _TRACE
	std::cout << " TinyConnection: " << connection.first << ", " << connection.second << std::endl;
#endif
	unsigned int fatSccI2 = connection.second.first.configurationSCCI;
	ConnectionPoint &p2 = connection.second.second;

	if(unencoded.find(fatSccI2) != unencoded.end()) { // Connection to new scc found!
	  // Check if rotation necessary!
	  if(!fatSccs[fatSccI2].isRotationallyMinimal(p2)) {
	    rotateSCC(fatSccI, connectionPoints, connectionMaps);
	    p2 = ConnectionPoint(p2, fatSccs[fatSccI2].rotationBrickPosition);
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
    for(std::vector<TinyConnection>::iterator it = toEncode.begin(); it != toEncode.end(); ++it) {
      // Ensure above, then below
      TinyConnection &c = *it;
      if(!c.first.second.above)
	std::swap(c.first, c.second);
      // Blow up the TinyConnection... again:
      const IConnectionPoint &ip1 = c.first;
      const IConnectionPoint &ip2 = c.second;
      const BrickIdentifier &i1 =ip1.first;
      const BrickIdentifier &i2 =ip2.first;
      // TODO: permute indices!
      int aboveI = 6*perm[i1.configurationSCCI] + i1.sccBrickI;
      int belowI = 6*perm[i2.configurationSCCI] + i2.sccBrickI;
      const ConnectionPoint &cpAbove = ip1.second;
      const ConnectionPoint &cpBelow = ip2.second;
      // Perform encoding:
      encoded = (encoded << 3) + identifierToCompressed[aboveI];
      encoded = (encoded << 2) + (int)(cpAbove.type);
      encoded = (encoded << 3) + identifierToCompressed[belowI];
      encoded = (encoded << 2) + (int)(cpBelow.type);
    }
    
    std::cout << "..ConfigurationEncoder::Encoded " << toEncode.size() << " connections: " << encoded << std::endl;
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
  uint64_t encode(const ConnectionList &list) const {
#ifdef _TRACE
    std::cout << " INIT encode(" << list << ")" << std::endl;
#endif
    // Setup:
    std::vector<ConnectionPoint> connectionPoints[6];
    std::map<ConnectionPoint,TinyConnection> connectionMaps[6];

    // Assign connections to lists, then sort them:
    for(std::set<TinyConnection>::const_iterator it = list.begin(); it != list.end(); ++it) {
      // Blow up the TinyConnection:
      const TinyConnection &c = *it;
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
#ifdef _TRACE
    std::cout << " Vectors sorted" << std::endl;
#endif

    uint64_t minEncoded = 0;
    bool minSet = false;
    for(unsigned int i = 0; i < fatSccSize; ++i) {
      if(!(fatSccs[i] == fatSccs[0]))
	break;
      // Only run if connections are minimal:
      if(fatSccs[i].isRotationallyMinimal(connectionPoints[i])) {
	uint64_t encoded = encode(i, false, connectionPoints, connectionMaps);
	if(!minSet || encoded < minEncoded)
	  minEncoded = encoded;
	minSet = true;
      }
      // Rotate and run again if necessary:
      if(fatSccs[i].isRotationallyIdentical(connectionPoints[i])) {
	uint64_t encoded = encode(i, true, connectionPoints, connectionMaps);
	if(!minSet || encoded < minEncoded)
	  minEncoded = encoded;
	minSet = true;
      }
    }
    return minEncoded;
  }

  void decode(uint64_t encoded, ConnectionList &list) const {
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

      // Reconstruct TinyConnection:
      const BrickIdentifier &ib1 = compressedToIdentifier[cp1Index];
      RectilinearBrick rb1 = fatSccs[ib1.configurationSCCI][ib1.sccBrickI];
      ConnectionPoint cp1(cp1Type, rb1, true, ib1.sccBrickI);
      const BrickIdentifier &ib2 = compressedToIdentifier[cp2Index];
      RectilinearBrick rb2 = fatSccs[ib2.configurationSCCI][ib2.sccBrickI];
      ConnectionPoint cp2(cp2Type, rb2, false, ib2.sccBrickI);

      TinyConnection c(IConnectionPoint(ib1,cp1),IConnectionPoint(ib2,cp2));
      list.insert(c);
    }
    
    std::cout << "..ConfigurationEncoder::decode " << list.size() << " connections: " << list << std::endl;
  }

  void testCodec(const ConnectionList &list1) const {
#ifdef _TRACE
    std::cout << "INIT testCodec(" << list1 << ")" << std::endl;
#endif
    std::vector<Connection> cs;
    for(std::set<TinyConnection>::const_iterator it = list1.begin(); it != list1.end(); ++it)
      cs.push_back(Connection(*it));
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
    ConnectionList list2;
    decode(encoded, list2);
#ifdef _TRACE
    std::cout << " decoded: " << list2 << std::endl;
#endif
    
    cs.clear();
    for(std::set<TinyConnection>::const_iterator it = list2.begin(); it != list2.end(); ++it)
      cs.push_back(Connection(*it));
    Configuration c2(fatSccs, cs);
#ifdef _TRACE
    std::cout << " c2: " << c2 << std::endl;
#endif

    FatSCC min2 = c2.toMinSCC();
#ifdef _TRACE
    std::cout << " min2: " << min2 << std::endl;
#endif
    
    assert(min1 == min2);
  }
};

#endif // CONFIGURATION_HPP
