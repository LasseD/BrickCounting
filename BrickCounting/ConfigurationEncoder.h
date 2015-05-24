#ifndef CONFIGURATION_ENCODER_HPP
#define CONFIGURATION_ENCODER_HPP

#include "RectilinearBrick.h"
#include "ConnectionPoint.h"
#include "StronglyConnectedConfiguration.hpp"
#include "Configuration.hpp"

/*
Used for uniquely encoding:
- A configuration, not necessarily valid, without given angles, into a uint64_t.
- A configuration with angles into a EncodedConfiguration.
Performs necessary rotations and permutations on FatSCCs of configuration in order to minimize encoded value.
*/
class ConfigurationEncoder {
  FatSCC fatSccs[6];
  unsigned int fatSccSize;
  // Compressed index for bricks enables brick ID saving using 3 bit:
  BrickIdentifier compressedToIdentifier[6];
  int identifierToCompressed[36]; // 6*configurationI + brickI in scc.
  // Duplicate mapping assists in quick lookup for running indices of duplicate SCCs:
  unsigned int duplicateMapping[6];

public:
  ConfigurationEncoder(const std::vector<FatSCC> &combination);

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
  uint64_t encode(const IConnectionPairList &list) const;

  void decode(uint64_t encoded, IConnectionPairList &list) const;

  void testCodec(const IConnectionPairList &list1) const;

private:
  void rotateSCC(int i, std::vector<ConnectionPoint> *connectionPoints, std::map<ConnectionPoint,IConnectionPair> *connectionMaps) const;

  /*
  When encoding:
  - perform permutation so that SCC of same size,diskIndex are ordered by visiting order.
  - Rotate rotationally symmetric SCCs so that they are initially visited at the minimally rotated position.
  */
  uint64_t encode(unsigned int baseIndex, bool rotate, std::vector<ConnectionPoint> *connectionPoints, std::map<ConnectionPoint,IConnectionPair> *connectionMaps) const;
};

#endif // CONFIGURATION_ENCODER_HPP
