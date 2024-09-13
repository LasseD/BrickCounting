// -*- mode: c++; tab-width: 2; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=2 sts=2 sw=2 noet :

#include "stdint.h"
#include <iostream>
#include <fstream>

#include "rectilinear.h"

namespace rectilinear {

Brick::Brick() : is_vertical(true), x(0), y(0) {
}
Brick::Brick(bool iv, int8_t x, int8_t y) : is_vertical(iv), x(x), y(y) {	
}
Brick::Brick(const Brick &b) : is_vertical(b.is_vertical), x(b.x), y(b.y) {
}
bool Brick::intersects(const Brick &b) const {
		if(is_vertical != b.is_vertical)
				return DIFF(b.x, x) < 3 && DIFF(b.y, y) < 3;
		if(is_vertical)
				return DIFF(b.x, x) < 2 && DIFF(b.y, y) < 4;
		else
				return DIFF(b.x, x) < 4 && DIFF(b.y, y) < 2;
}

bool Brick::operator ==(const Brick& b) const {
    return x == b.x && y == b.y && is_vertical == b.is_vertical;
}
bool Brick::operator !=(const Brick& b) const {
    return !(*this == b);
}
int Brick::cmp(const Brick& b) const {
	if(is_vertical != b.is_vertical)
		return - is_vertical + b.is_vertical;
  if(x != b.x)
		return x - b.x;
	return y - b.y;	
}
bool Brick::operator <(const Brick& b) const {
	return cmp(b) < 0;
}
uint64_t Brick::encode15() const {
		uint64_t X = x+30;
		uint64_t Y = y+30;
		return (((X << 7) | Y) << 1) | is_vertical;
}

std::ostream& operator <<(std::ostream &os,const Brick &b) {
    os << (b.is_vertical?"|":"=") << (int)b.x << "," << (int)b.y << (b.is_vertical?"|":"=");
    return os;
}

}
