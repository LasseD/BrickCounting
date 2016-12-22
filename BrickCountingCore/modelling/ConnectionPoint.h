#ifndef MODELLING_CONNECTION_POINT_H
#define MODELLING_CONNECTION_POINT_H

#include <iostream>
#include <stdint.h>

#include "RectilinearBrick.h"

namespace modelling {
	enum ConnectionPointType { NW = 0, NE = 1, SE = 2, SW = 3 };

	struct ConnectionPoint {
		ConnectionPointType type;
		RectilinearBrick brick;
		bool above, angleLocked;
		int brickI;

		ConnectionPoint(ConnectionPointType type, const RectilinearBrick &brick, bool above, int brickI);
		ConnectionPoint();
		ConnectionPoint(const ConnectionPoint &p);
		ConnectionPoint(const ConnectionPoint &p, std::pair<int8_t, int8_t> rotationPoint);

		// For specialized use: Pretend brick is at a 4x4 grid. Get x/y of connection point in this grid (+brick pos.).
		int8_t x4x4() const;
		int8_t y4x4() const;
		int8_t level() const;
		bool angleLocks(const ConnectionPoint &p) const;

		// For specialized use: Position of connection point 
		double x() const;
		double y() const;

		bool operator < (const ConnectionPoint &p) const;
		bool operator != (const ConnectionPoint &p) const;
		bool operator == (const ConnectionPoint &p) const;
	};

	std::ostream& operator<<(std::ostream &os, const ConnectionPoint& p);

	struct BrickIdentifier {
		unsigned long blockInFile; // In Block list imported from file.
		int brickIndexInBlock; // In Block.
		int modelBlockI;
		BrickIdentifier();
		BrickIdentifier(const BrickIdentifier &bi);
		BrickIdentifier(unsigned long blockInFile, int brickIndexInBlock, int modelBlockI);

		bool operator<(const BrickIdentifier &bi) const;
		bool operator!=(const BrickIdentifier &bi) const;
		bool operator==(const BrickIdentifier &bi) const;
	};

	std::ostream& operator<<(std::ostream &os, const BrickIdentifier& bi);

	typedef std::pair<BrickIdentifier, ConnectionPoint> IConnectionPoint;

	std::ostream& operator<<(std::ostream &os, const IConnectionPoint& p);
}

#endif // MODELLING_CONNECTION_POINT_H
