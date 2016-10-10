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

LayerBrick::LayerBrick() : brick(), layer((uint8_t)0) {
}
LayerBrick::LayerBrick(Brick b, uint8_t l) : brick(b), layer(l) {
}
LayerBrick::LayerBrick(const LayerBrick &b) : brick(b.brick), layer(b.layer) {
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

int LayerBrick::cmp(const LayerBrick& b) const {
    if(layer != b.layer)
		return layer - b.layer;
	return brick.cmp(b.brick);
}
bool LayerBrick::operator <(const LayerBrick& b) const {
	return cmp(b) < 0;
}

std::ostream& operator <<(std::ostream &os,const Brick &b) {
    os << (b.is_vertical?"|":"=") << (int)b.x << "," << (int)b.y << (b.is_vertical?"|":"=");
    return os;
}

std::ostream& operator <<(std::ostream &os,const LayerBrick &b) {
    os << (b.brick.is_vertical?"|":"=") << (int)b.brick.x << "," << (int)b.brick.y << "," << (int)b.layer << (b.brick.is_vertical?"|":"=");
	return os;
}

}
