#pragma once

enum ConnectionPointType {NW, NE, SW, SE};

struct ConnectionPoint {
	ConnectionPointType type;
	bool isAbove;
	ConnectionPoint(ConnectionPointType type, bool isAbove) : type(type), isAbove(isAbove) {}
};

class RectilinearBrick {
public:
	int x, y, level;
	bool horizontal;
	RectilinearBrick(int x, int y, int level, bool horizontal) : x(x), y(y), level(level), horizontal(horizontal) {}

	bool intersects(RectilinearBrick &b);

	void getStronglyConnected(RectilinearBrick *bricks, int &bricksSize);

	//void getConnectionPointsAbove(ConnectionPoint *pts, int &sizePts);
	//void getConnectionPointsBelow(ConnectionPoint *pts, int &sizePts);

private:
	/*void getConnectionPointsNoLevel(ConnectionPoint *pts, int &sizePts);
	void setConnectionPointsDataNoLevel(ConnectionPoint &pt, bool isAbove, ConnectionPointType type) {
		pt.isAbove = isAbove;
		pt.type = type;
	}*/
	bool inInterval(int min, int max, int a) {
	  return  min <= a && a <= max;
	}
	bool distLessThan2(int a, int b) {
		return a == b || a-1 == b || b-1 == a;
	}
};

