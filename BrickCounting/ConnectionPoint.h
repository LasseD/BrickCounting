#pragma once

enum ConnectionPointType {CONNECTION_UP_RIGHT, CONNECTION_UP_LEFT, CONNECTION_DOWN_RIGHT, CONNECTION_DOWN_LEFT};

struct ConnectionPoint {
	ConnectionPointType type;
	int x, y, level;
};
