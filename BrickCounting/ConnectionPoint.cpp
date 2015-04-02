#include "ConnectionPoint.h"
#include "RectilinearBrick.h"

#include <iostream>
#include <stdint.h>

ConnectionPoint::ConnectionPoint(ConnectionPointType type, const RectilinearBrick &brick, bool above, int brickI) : type(type), brick(brick), above(above), angleLocked(false), brickI(brickI) {}

ConnectionPoint::ConnectionPoint() : brickI(-1) {}

ConnectionPoint::ConnectionPoint(const ConnectionPoint &p) : type(p.type), brick(p.brick), above(p.above), angleLocked(p.angleLocked), brickI(p.brickI) {}

ConnectionPoint::ConnectionPoint(const ConnectionPoint &p, std::pair<int,int> rotationPoint) : type((ConnectionPointType)((p.type+2)%4)), brick(p.brick), above(p.above), angleLocked(false), brickI(-1) {
  // Relocate position of brick:
  brick.y = rotationPoint.second - brick.y;
  brick.x = rotationPoint.first - brick.x;
}

// For specialized use: Pretend brick is at a 4x4 grid. Get x/y of connection point in this grid (+brick pos.).
uint8_t ConnectionPoint::x4x4() const {
  if(brick.horizontal()) {
    if(type == SW || type == SE)
      return brick.x;
    return brick.x+3;    
  }
  else {
    if(type == NW || type == SW)
      return brick.x+1;
    return brick.x+2;
  }
}

uint8_t ConnectionPoint::y4x4() const {
  if(brick.horizontal()) {
    if(type == SW || type == NW)
      return brick.y+2;
    return brick.y+1;    
  }
  else {
    if(type == SW || type == SE) 
      return brick.y;
    return brick.y+3;
  }
}

// For specialized use: Position of connection point 
double ConnectionPoint::x() const {
  if(type == NW || type == SW)
    return brick.x-HALF_STUD_DISTANCE;
  return brick.x+HALF_STUD_DISTANCE;
}

double ConnectionPoint::y() const {
  if(type == SW || type == SE) 
    return brick.y-STUD_AND_A_HALF_DISTANCE;
  return brick.y+STUD_AND_A_HALF_DISTANCE;
}

bool ConnectionPoint::operator < (const ConnectionPoint &p) const {
  if(brick != p.brick)
    return brick < p.brick;
  if(type != p.type)
    return type < p.type;
  return above < p.above;
}

bool ConnectionPoint::operator != (const ConnectionPoint &p) const {
  if(brick != p.brick)
    return true;
  if(type != p.type)
    return true;
  return above != p.above;
}

bool ConnectionPoint::operator == (const ConnectionPoint &p) const {
  if(brick != p.brick)
    return false;
  if(type != p.type)
    return false;
  return above == p.above;
}

std::ostream& operator<<(std::ostream &os, const ConnectionPoint& p) {
  if(p.above)
    os << "A";
  else
    os << "B";
  os << p.brick;
  switch(p.type) {
  case NW : os << "NW"; break;
  case NE : os << "NE"; break;
  case SW : os << "SW"; break;
  case SE : os << "SE"; break;
  }
  return os;
}

BrickIdentifier::BrickIdentifier() {}

BrickIdentifier::BrickIdentifier(const BrickIdentifier &bi) : sccI(bi.sccI), sccBrickI(bi.sccBrickI), configurationSCCI(bi.configurationSCCI) {}

BrickIdentifier::BrickIdentifier(unsigned long sccI, int sccBrickI, int configurationSCCI) : sccI(sccI), sccBrickI(sccBrickI), configurationSCCI(configurationSCCI) {}

bool BrickIdentifier::operator<(const BrickIdentifier &bi) const {
  if(sccI != bi.sccI)      
    return sccI < bi.sccI;
  if(sccBrickI != bi.sccBrickI)      
    return sccBrickI < bi.sccBrickI;
  return configurationSCCI < bi.configurationSCCI;
}

bool BrickIdentifier::operator!=(const BrickIdentifier &bi) const {
  return sccI != bi.sccI || sccBrickI != bi.sccBrickI || configurationSCCI != bi.configurationSCCI;
}

bool BrickIdentifier::operator==(const BrickIdentifier &bi) const {
  return sccI == bi.sccI && sccBrickI == bi.sccBrickI && configurationSCCI == bi.configurationSCCI;
}

std::ostream& operator<<(std::ostream &os, const BrickIdentifier& bi) {
  os << "BI[scc=" << bi.sccI << "," << bi.sccBrickI << ",confI=" << bi.configurationSCCI << "]";
  return os;
}

std::ostream& operator<<(std::ostream &os, const IConnectionPoint& p) {
  os << "ICP[" << p.first << "," << p.second << "]";
  return os;
}


