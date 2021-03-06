#include "ConnectionPoint.h"

#include <iostream>
#include <stdint.h>

#include "RectilinearBrick.h"

namespace modelling {
	ConnectionPoint::ConnectionPoint(ConnectionPointType type, const RectilinearBrick &brick, bool above, int brickI) : type(type), brick(brick), above(above), angleLocked(false), brickI(brickI) {}

	ConnectionPoint::ConnectionPoint() : brickI(-1) {}

	ConnectionPoint::ConnectionPoint(const ConnectionPoint &p) : type(p.type), brick(p.brick), above(p.above), angleLocked(p.angleLocked), brickI(p.brickI) {}

	ConnectionPoint::ConnectionPoint(const ConnectionPoint &p, std::pair<int8_t, int8_t> rotationPoint) : type((ConnectionPointType)((p.type + 2) % 4)), brick(p.brick), above(p.above), angleLocked(false), brickI(-1) {
		// Relocate position of brick:
		brick.y = rotationPoint.second - brick.y;
		brick.x = rotationPoint.first - brick.x;
	}

	// For specialized use: Pretend brick is at a 4x4 grid. Get x/y of connection point in this grid (+brick pos.).
	int8_t ConnectionPoint::x4x4() const {
		if (brick.horizontal()) {
			if (type == SW || type == SE)
				return brick.x;
			return brick.x + 3;
		}
		else {
			if (type == NW || type == SW)
				return brick.x + 1;
			return brick.x + 2;
		}
	}

	int8_t ConnectionPoint::y4x4() const {
		if (brick.horizontal()) {
			if (type == SW || type == NW)
				return brick.y + 2;
			return brick.y + 1;
		}
		else {
			if (type == SW || type == SE)
				return brick.y;
			return brick.y + 3;
		}
	}

	int8_t ConnectionPoint::level() const {
		if (above)
			return brick.level() + 1;
		else
			return brick.level() - 1;
	}

	bool ConnectionPoint::angleLocks(const ConnectionPoint &p) const {
		if (level() != p.level()) {
			return false; // Can't angle lock at different level.
		}
		return ((x4x4() == p.x4x4() + 1 || x4x4() + 1 == p.x4x4()) && y4x4() == p.y4x4()) ||
			((y4x4() == p.y4x4() + 1 || y4x4() + 1 == p.y4x4()) && x4x4() == p.x4x4());
	}

	// For specialized use: Position of connection point 
	double ConnectionPoint::x() const {
		if (brick.horizontal()) {
			if (type == SW || type == SE)
				return brick.x - STUD_AND_A_HALF_DISTANCE;
			return brick.x + STUD_AND_A_HALF_DISTANCE;
		}
		else {
			if (type == NW || type == SW)
				return brick.x - HALF_STUD_DISTANCE;
			return brick.x + HALF_STUD_DISTANCE;
		}
	}

	double ConnectionPoint::y() const {
		if (brick.horizontal()) {
			if (type == SW || type == NW)
				return brick.y + HALF_STUD_DISTANCE;
			return brick.y - HALF_STUD_DISTANCE;
		}
		else {
			if (type == SW || type == SE)
				return brick.y - STUD_AND_A_HALF_DISTANCE;
			return brick.y + STUD_AND_A_HALF_DISTANCE;
		}
	}

	bool ConnectionPoint::operator < (const ConnectionPoint &p) const {
		if (brick != p.brick)
			return brick < p.brick;
		if (type != p.type)
			return type < p.type;
		return above < p.above;
	}

	bool ConnectionPoint::operator != (const ConnectionPoint &p) const {
		if (brick != p.brick)
			return true;
		if (type != p.type)
			return true;
		return above != p.above;
	}

	bool ConnectionPoint::operator == (const ConnectionPoint &p) const {
		if (brick != p.brick)
			return false;
		if (type != p.type)
			return false;
		return above == p.above;
	}

	std::ostream& operator<<(std::ostream &os, const ConnectionPoint& p) {
		if (p.above)
			os << "A";
		else
			os << "B";
		//os << p.brick;
		//os << p.brickI;
		switch (p.type) {
		case NW: os << "NW"; break;
		case NE: os << "NE"; break;
		case SW: os << "SW"; break;
		case SE: os << "SE"; break;
		}
		return os;
	}

	BrickIdentifier::BrickIdentifier() {}

	BrickIdentifier::BrickIdentifier(const BrickIdentifier &bi) : blockInFile(bi.blockInFile), brickIndexInBlock(bi.brickIndexInBlock), modelBlockI(bi.modelBlockI) {}

	BrickIdentifier::BrickIdentifier(unsigned long blockInFile, int brickIndexInBlock, int modelBlockI) : blockInFile(blockInFile), brickIndexInBlock(brickIndexInBlock), modelBlockI(modelBlockI) {}

	bool BrickIdentifier::operator<(const BrickIdentifier &bi) const {
		if (blockInFile != bi.blockInFile)
			return blockInFile < bi.blockInFile;
		if (brickIndexInBlock != bi.brickIndexInBlock)
			return brickIndexInBlock < bi.brickIndexInBlock;
		return modelBlockI < bi.modelBlockI;
	}

	bool BrickIdentifier::operator!=(const BrickIdentifier &bi) const {
		return blockInFile != bi.blockInFile || brickIndexInBlock != bi.brickIndexInBlock || modelBlockI != bi.modelBlockI;
	}

	bool BrickIdentifier::operator==(const BrickIdentifier &bi) const {
		return blockInFile == bi.blockInFile && brickIndexInBlock == bi.brickIndexInBlock && modelBlockI == bi.modelBlockI;
	}

	std::ostream& operator<<(std::ostream &os, const BrickIdentifier& bi) {
		bool first = true;
		if (bi.blockInFile != 0) {
			os << "fileI=" << bi.blockInFile;
			first = false;
		}
		if (bi.brickIndexInBlock != 0) {
			if (!first)
				os << ",";
			os << "brickI=" << bi.brickIndexInBlock;
			first = false;
		}
		if (bi.modelBlockI != 0) {
			if (!first)
				os << ",";
			os << "blockI=" << bi.modelBlockI;
		}
		return os;
	}

	std::ostream& operator<<(std::ostream &os, const IConnectionPoint& p) {
		os << "(" << p.first << "," << p.second << ")";
		return os;
	}
}
