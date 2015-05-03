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

  Configuration(const FatSCC &scc) {
    bricksSize = 0;
    RectilinearBrick b;
    origBricks.insert(std::make_pair(0,Brick(b)));
    for(int i = 0; i < scc.size; b=scc.otherBricks[i++]) {
      Brick brick(b);
      IBrick ib(b, brick, BrickIdentifier(scc.index, i, 0));
      bricks[bricksSize++] = ib;
    }
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

  void toLDR(std::ofstream &os, int x, int y, int ldrColor) const {    
    int colors[6] = {LDR_COLOR_RED, LDR_COLOR_YELLOW, LDR_COLOR_BLUE, 3, 85, LDR_COLOR_BLACK};

    for(int i = 0; i < bricksSize; ++i) {
      bricks[i].b.toLDR(os, x, y, colors[bricks[i].bi.configurationSCCI]);
    }
  }
};

struct PossibleConnections {
  std::pair<int,int> possibleConnections[15]; // Brick index -> brick index.
  int rPossibleConnections[36]; // Reverse lookup of ^^: from * 6 + to -> index

  PossibleConnections() {
    int i = 0;
    for(int from = 0; from < 5; ++from) {
      for(int to = from+1; to <= 5; ++to) {
	rPossibleConnections[from*6 + to] = i;
	possibleConnections[i] = std::make_pair(from,to);
	++i;
      }
    }
  }
};
static PossibleConnections possibleConnections; // Only used in ConfigurationEncoder

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
  }

  uint64_t encode(unsigned int baseIndex, bool rotate, std::vector<ConnectionPoint> *connectionPoints, std::map<ConnectionPoint,TinyConnection> *connectionMaps) const {    
    // If rotate, use "rotated" connectionPoints list for baseIndex:
    std::vector<ConnectionPoint> rotatedList;
    if(rotate) {
      unsigned int size = connectionPoints[baseIndex].size();
      for(unsigned int i = 0; i < connectionPoints[baseIndex].size(); ++i) {
	rotatedList.push_back(connectionPoints[baseIndex][(i + size/2)%size]);
      }
    }

    // Compute connections to encode + encodedI on sccs:
    std::set<unsigned int> unencoded;
    for(unsigned int i = 0; i < fatSccSize; ++i) {
      if(i != baseIndex)
	unencoded.insert(i);
    }
    std::vector<TinyConnection> toEncode;
    std::queue<unsigned int> queue;
    queue.push(baseIndex);
    while(!unencoded.empty()) {
      unsigned int fatSccI = queue.front();
      queue.pop();
      std::vector<ConnectionPoint> &v = (fatSccI == baseIndex && rotate) ? rotatedList : connectionPoints[fatSccI];
      for(std::vector<ConnectionPoint>::const_iterator it = v.begin(); it != v.end(); ++it) {
	TinyConnection connection = connectionMaps[fatSccI][*it];
	unsigned int fatSccI2 = connection.first.first.configurationSCCI;
	if(fatSccI2 == fatSccI)
	  fatSccI2 = connection.second.first.configurationSCCI;
	if(unencoded.find(fatSccI2) != unencoded.end()) {
	  // Connection to new scc found!
	  toEncode.push_back(connection);
	  queue.push(fatSccI2);	  
	  unencoded.erase(fatSccI2);
	}
      }
    }      
    
    /*
     Encoding algorithm for a connection: 
     -- aboveBrickI(3 bit), 
     -- aboveConnectionPointType(2 bit) 
     -- -||- below
     */
    uint64_t encoded = (baseIndex << 1) + rotate;
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
      int aboveI = 6*i1.configurationSCCI + i1.sccBrickI;
      int belowI = 6*i2.configurationSCCI + i2.sccBrickI;
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

	/*
TODO: Implement if encode shows problems.
  void decode(uint64_t encoded, ConnectionList &list) const {
    unsigned int size = encoded & 0x0F;
    encoded >>= 4;

    for(unsigned int i = 0; i < size; ++i) {
      // decode parts:
      bool cp2Above = encoded & 0x01;
      encoded >>= 1;
      ConnectionPointType cp2Type = encoded & 0x03;
      encoded >>= 2;
      ConnectionPointType cp1Type = encoded & 0x03;
      encoded >>= 2;
      int connectionsIndex = encoded & 0x0F;
      encoded >>= 4;

      // Reconstruct TinyConnection:
      std::pair<int,int> connectionIndices = possibleConnections.possibleConnections(connectionsIndex);
      const IBrick &ib1 = c.bricks[connectionIndices.first];
      const ConnectionPoint cp1(cp1Type, b1.rb, !cp2Above, ib1.sccBrickI);
      const IBrick &ib2 = c.bricks[connectionIndices.second];
      const ConnectionPoint cp2(cp2Type, b2.rb, cp2Above, ib2.sccBrickI);

      TinyConnection c(cp1,cp2);
      list.add(c);
    }
    
    std::cout << "..ConfigurationEncoder::Encoded " << list.size() << " connections: " << encoded << std::endl;
    return encoded;
  }*/

};

#endif // CONFIGURATION_HPP
