#include "stdafx.h"
#include "RectilinearBrick.h"

bool RectilinearBrick::intersects(RectilinearBrick &b) {
	if(b.level != level)
		return false;
	if(horizontal && b.horizontal) {
		return inInterval(x-3, x+3, b.x) && inInterval(y-1, y+1, b.y);
	}
	if(!horizontal && !b.horizontal) {
		return inInterval(x-1, x+1, b.x) && inInterval(y-3, y+3, b.y);
	}
	if(horizontal && !b.horizontal) {
		return inInterval(x-1, x+3, b.x) && inInterval(y-3, y+1, b.y);
	}
	return inInterval(x-3, x+1, b.x) && inInterval(y-1, y+3, b.y);
}

/*void RectilinearBrick::getConnectionPointsAbove(ConnectionPoint *pts, int &sizePts) {
	getConnectionPointsNoLevel(pts, sizePts);
	for(unsigned int i = 0; i < 4; ++i) {
		pts[i].level = level+1;
	}
}
void RectilinearBrick::getConnectionPointsBelow(ConnectionPoint *pts, int &sizePts) {
	getConnectionPointsNoLevel(pts, sizePts);
	for(unsigned int i = 0; i < 4; ++i) {
		pts[i].level = level-1;
	}
}
void RectilinearBrick::getConnectionPointsNoLevel(ConnectionPoint *pts, int &sizePts) {
	sizePts = 4;
	setConnectionPointsDataNoLevel(pts[0], x, y, CONNECTION_DOWN_LEFT);
	if(horizontal) {
		setConnectionPointsDataNoLevel(pts[1], x+3, y, CONNECTION_DOWN_RIGHT);
		setConnectionPointsDataNoLevel(pts[2], x, y+1, CONNECTION_UP_LEFT);
		setConnectionPointsDataNoLevel(pts[3], x+3, y+1, CONNECTION_UP_RIGHT);
	}
	else {
		setConnectionPointsDataNoLevel(pts[1], x+1, y, CONNECTION_DOWN_RIGHT);
		setConnectionPointsDataNoLevel(pts[2], x, y+3, CONNECTION_UP_LEFT);
		setConnectionPointsDataNoLevel(pts[3], x+1, y+3, CONNECTION_UP_RIGHT);
	}
}*/

