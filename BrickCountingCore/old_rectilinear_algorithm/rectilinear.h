// -*- mode: c++; tab-width: 2; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=2 sts=2 sw=2 noet :

#ifndef BRICKS_RECTILINEAR_H
#define BRICKS_RECTILINEAR_H

#include "stdint.h"
#include <set>
#include <stdarg.h>
#include <iostream>
#include <fstream>
#include <assert.h>
#include <sstream>
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace boost::posix_time;

namespace rectilinear {

struct Brick {
	Brick();
	Brick(bool iv, int8_t x, int8_t y);
	Brick(const Brick &b);

  bool operator <(const Brick& b) const;
  bool operator ==(const Brick& b) const;
  bool operator !=(const Brick& b) const;
  friend std::ostream& operator <<(std::ostream &os,const Brick &b);
  int cmp(const Brick& b) const;

  bool is_vertical:1;
  int8_t x:6, y:6; // vs uint16_t x:5
};

const Brick FirstBrick = Brick();

struct LayerBrick {
	LayerBrick();
	LayerBrick(Brick b, uint8_t l);
	LayerBrick(const LayerBrick &b);	

    bool operator <(const LayerBrick& b) const;
    friend std::ostream& operator <<(std::ostream &os,const LayerBrick &b);
    int cmp(const LayerBrick& b) const;

    Brick brick;
    uint8_t layer:3; // :3
};

/*
  Height: always 6.
*/
template <int Z>
class Combination {
public:
    LayerBrick bricks[Z];

	/*
	  Init with one brick on layer 0 at 0,0.
	*/
	Combination() {
		bricks[0] = LayerBrick(FirstBrick, 0);
	}

	Combination(LayerBrick b[]) {
		for(int i = 0; i < Z; i++) {
			bricks[i] = b[i];
		}
	}

	Combination(const Combination<Z> &b) {
		for(int i = 0; i < Z; i++) {
			bricks[i] = b.bricks[i];
		}
	}

	inline friend std::ostream& operator << (std::ostream &os, const Combination<Z> &b) {
		for(int i = 0; i < Z; i++)
			os << b.bricks[i] << " ";
		return os;
	}

	bool operator <(const Combination<Z>& b) const {
		for(int i = 0; i < Z; i++) {
			int res = bricks[i].cmp(b.bricks[i]);
			if(res != 0)
				return res < 0;
		}		    
		return false;
	}
	bool operator ==(const Combination& b) const {
		for(int i = 0; i < Z; i++) {
			int res = bricks[i].cmp(b.bricks[i]);
			if(res != 0)
				return false;
		}		    
		return true;
	}

	bool is_valid() const {
		for(int i = 1; i < Z; i++) {
			if(bricks[i] < bricks[i-1] || !(bricks[i-1] < bricks[i])) {
				std::cerr << "Error: Invalid combination: " << *this << std::endl;
				return false;
			}
		}
		return true;
	}

	void rotate90() {
		int8_t minx = 16, miny = 16;

		for(int i = 0; i < Z; i++) {
			int8_t tmp = bricks[i].brick.x;
			bricks[i].brick.x = bricks[i].brick.y;
			bricks[i].brick.y = tmp;
//		std::swap(bricks[i].brick.x, bricks[i].brick.y);
			if(bricks[i].brick.is_vertical) {
				bricks[i].brick.y = -bricks[i].brick.y-2;
			}
			else {
				bricks[i].brick.y = -bricks[i].brick.y-4;
				if((int)bricks[i].layer == 0 && 
				   (bricks[i].brick.x < minx || 
					(bricks[i].brick.x == minx && bricks[i].brick.y < miny))) {
					minx = bricks[i].brick.x;
					miny = bricks[i].brick.y;
				}
			}
			bricks[i].brick.is_vertical = !bricks[i].brick.is_vertical;
		}
		for(int i = 0; i < Z; i++) {
			bricks[i].brick.x -= minx;
			bricks[i].brick.y -= miny;
		}
		std::sort(bricks,&bricks[Z]);
		assert(is_valid());
	}

	void rotate180() {
		//std::cout << " Rotating180: " << *this << std::endl;
		int8_t minx = bricks[0].brick.x, miny = bricks[0].brick.y;

		for(int i = 1; i < Z; i++) { // The first brick stays.
			if(bricks[i].brick.is_vertical) {
				//std::cout << "  Move: " << bricks[i].brick << std::endl;
				bricks[i].brick.x = -bricks[i].brick.x;			
				bricks[i].brick.y = -bricks[i].brick.y;
				if((int)bricks[i].layer == 0 && 
				   (bricks[i].brick.x < minx || 
					(bricks[i].brick.x == minx && bricks[i].brick.y < miny))) {
					minx = bricks[i].brick.x;
					miny = bricks[i].brick.y;
				}
				//std::cout << "  To: " << bricks[i].brick << std::endl;
			}
			else {
				bricks[i].brick.x = -bricks[i].brick.x - 2;			
				bricks[i].brick.y = -bricks[i].brick.y + 2;
			}
		}
		//std::cout << " Before add: " << *this << ", minx: " << minx << ", miny: " << miny << std::endl;
		if(minx < 0 || (minx == 0 && miny < 0)) { // move all:
			for(int i = 0; i < Z; i++) {
				bricks[i].brick.x -= minx;
				bricks[i].brick.y -= miny;
			}
		}
		//std::cout << " Before sort: " << *this << std::endl;
		std::sort(bricks,&bricks[Z]);
		assert(is_valid());
	}

	bool can_rotate90() const {
		if(Z < 3)
			return false;
		for(int i = 0; i < Z && (int)bricks[i].layer == 0; i++) {
			if(!bricks[i].brick.is_vertical)
				return true;
		}
		return false;
	}

	int diff(int8_t const a, int8_t const b) const {
		if(a < b)
			return b-a;
		return a-b;
	}

	template <int W>
	bool add(Brick b, uint8_t layer, Combination<W> &out) const {
		assert(W == Z+1);
		for(int i = 0; i < Z; i++) {
			LayerBrick b2 = bricks[i];
			if(layer+1 < b2.layer)
				break; // no more to check.
			if(layer != b2.layer) 
				continue;
			if(b.is_vertical) {
				if(b2.brick.is_vertical) {
					if(diff(b.x, b2.brick.x) < 2 && 
					   diff(b.y, b2.brick.y) < 4)
						return false;
				}
				else {
					if(!((int)b2.brick.x >= (int)b.x + 2 ||
						 (int)b2.brick.y >= (int)b.y + 4 ||
						 (int)b2.brick.x + 4 <= (int)b.x ||
						 (int)b2.brick.y + 2 <= (int)b.y))
						return false;				
				}
			}
			else {
				if(b2.brick.is_vertical) {
					if(!((int)b.x >= (int)b2.brick.x + 2 ||
						 (int)b.y >= (int)b2.brick.y + 4 ||
						 (int)b.x + 4 <= (int)b2.brick.x ||
						 (int)b.y + 2 <= (int)b2.brick.y))
						return false;				
				}
				else {
					if(diff(b.x, b2.brick.x) < 4 && 
					   diff(b.y, b2.brick.y) < 2)
						return false;
				}
			}
		}

		if((int)layer == 0 && b.is_vertical && (b.x < 0 || (b.x == 0 && b.y < 0))) { // move all:
			//std::cout << "Moving " << b << " at " << (int)layer << " to " << *this << std::endl;
			for(int i = 1; i < W; i++) {
				out.bricks[i] = bricks[i-1];
				out.bricks[i].brick.x -= b.x;
				out.bricks[i].brick.y -= b.y;
			}
			out.bricks[0] = LayerBrick(FirstBrick, 0); // b moved
			return true;	   
		}

		// Put in:
		//std::cout << "Adding " << b << " at " << (int)layer << " to " << *this << std::endl;
		bool in = false;
		for(int i = 0; i < Z; i++) {
			if(!in) {
				out.bricks[i] = bricks[i];
				if(i+1 >= Z || bricks[i+1].layer > layer || (bricks[i+1].layer == layer && b < bricks[i+1].brick)) {
//				std::cout << "Cont bricks[i].layer: " << (int)(bricks[i].layer) << " vs " << (int)layer << ", " << bricks[i].brick << " vs " << b << std::endl;
					in = true;
					out.bricks[i+1] = LayerBrick(b, layer);
				}
			}
			else
				out.bricks[i+1] = bricks[i];
		}	
		if(!in)
			out.bricks[Z] = LayerBrick(b, layer);

		//std::cout << "To " << out << std::endl;
		return true;
	}

};

template <int Z>
struct TinyComp {
    LayerBrick bricks[Z];	

	TinyComp(Combination<Z+1> const &c) {	
		//assert(c.bricks[0].brick == Brick.FirstBrick && (int)(c.bricks[0].layer) == 0);
		for(int i = 1; i < Z+1; i++) {
			bricks[i-1] = c.bricks[i];
		}
	}
	
	Combination<Z+1> get() {
		Combination<Z+1> out;
		for(int i = 1; i < Z+1; i++) {
			out.bricks[i] = bricks[i-1];
		}
		return out;
	}

    bool operator <(const TinyComp<Z>& b) const {
		for(int i = 0; i < Z; i++) {
			if(bricks[i] < b.bricks[i])
				return true;
			if(b.bricks[i] < bricks[i])
				return false;
		}		    
		return false;		
	}
};

template <int Z>
class CombinationReader {
    std::ifstream *istream;
	Brick nxt;
	int layer_sizes[6];

public:
	CombinationReader(int ls[]) {
		std::cout << "  Combination reader for " << Z << "/";
		std::stringstream ss;
		ss << Z << "/";
	
		int layer_size = 0;
		int size_total = 0;
		for(int i = 0; size_total < Z; i++) {
			layer_size = ls[i];
			layer_sizes[i] = layer_size;
			std::cout << layer_size;
			ss << layer_size;
			size_total += layer_size;
		}
		assert(Z == size_total);

		std::string file_name = ss.str();
	
		istream = new std::ifstream(file_name.c_str(), std::ios::binary);
		// Read one (nxt): 
		if(Z == 1)
			nxt = Brick(true,0,0);
		else
			istream->read((char*)&nxt, sizeof(Brick));
		std::cout << " created with init " << nxt << std::endl;
	}

	~CombinationReader() {
		delete istream;
	}

	Combination<Z> next() {
//	std::cout << "  Reading size " << Z << " combination: " << Brick.FirstBrick;
		assert(has_next());
		LayerBrick bricks[6];
		bricks[0] = LayerBrick(FirstBrick,0);
		if(Z == 1) {
//		std::cout << " single " << std::endl;
			istream->read((char*)&nxt, sizeof(Brick));
			assert(Combination<Z>(bricks).is_valid());
			return Combination<Z>(bricks);
		}

		uint8_t layer = 0;
		int j = 1;
		for(int i = 1; i < Z; i++) { // Read size-1 bricks:
			assert(nxt.x != 20);
			if(j == layer_sizes[(int)layer]) {
				j = 0;
				layer++;
			}
			bricks[i] = LayerBrick(nxt,layer);
//		std::cout << bricks[i];
			istream->read((char*)&nxt, sizeof(Brick));
			j++;
		}
//	std::cout << std::endl;

		assert(Combination<Z>(bricks).is_valid());
		return Combination<Z>(bricks);
	}

	bool has_next() {
		return nxt.x != 20;
	}
};

template <int Z>
class CombinationWriter {
    std::set<TinyComp<Z-1> > combinations;
    std::ofstream *ostream;
	int token; // eg. 211 for 4 bricks in config 2-1-1	

public:
	CombinationWriter(int token) : token(token) {
		// inverse token:
		int it = 0;
		while(token > 0) {
			it *= 10;
			int size_add = token % 10;
			it += size_add;
			token /= 10;
		}

		std::stringstream ss;
		ss << Z << "/";
	
		while(it > 0) {
			ss << (it % 10);
			it /= 10;
		}

		std::string file_name = ss.str();
		//std::cout << " Combination writer to " << file_name << std::endl;
	
		ostream = new std::ofstream(file_name.c_str(), std::ios::binary);
	}

	int tokenize(Combination<Z> c) {
		int out = 0;
		int on_layer = 0;
		for(int i = 0; i < Z; i++) {
			if(i == 0 || c.bricks[i-1].layer == c.bricks[i].layer)
				on_layer++;
			else {
				out *= 10;
				out += on_layer;
				on_layer = 1;
			}	   	
		}
		out *= 10;
		out += on_layer;
		return out;
	}

	bool add(Combination<Z> c, int &added, int &symmetries) {
		assert(c.is_valid());
/*		if(token != tokenize(c)) {
		std::cout << "Adding: " << c << std::endl;
		std::cout << "token: " << token << ", nize: " << tokenize(c) << std::endl;
		}*/
		assert(token == tokenize(c));
/*	bool inserted = combinations.insert(TinyComp(c)).second;
	if(!inserted) {
//		std::cout << " already present: skip" << std::endl;
return false;
}*/
		Combination<Z> min(c);
		Combination<Z> rotated(c);
		symmetries++;
		if(c.can_rotate90()) {
			symmetries+=2;
			for(int i = 0; i < 3; i++) {			
				rotated.rotate90();
				if(rotated < min)
					min = rotated;
			}
//		std::cout << " added (" << added << "). Adding 3 symmetries" << std::endl;
		}
		else {
			rotated.rotate180();
			if(rotated < min)
				min = rotated;
		}
		//std::cout << "  Writing " << c << ": ";
		bool inserted = combinations.insert(TinyComp<Z-1>(min)).second;
		if(!inserted) {
			return false;
		}
		
		for(int i = 1; i < Z; i++) {
			ostream->write((char*)&(min.bricks[i].brick), sizeof(Brick));
			//std::cout << min.bricks[i];
		}
		//std::cout << std::endl;
		added++;
		return true;
	}

	~CombinationWriter() {
		Brick LastBrick(true, 20, 20); // hexdump: x (7f), is_vertical (01), 0, y (7f)
		ostream->write((char*)&(LastBrick), sizeof(Brick));
		ostream->flush();
		ostream->close();
		delete(ostream);
	}

	template <int W>
	void make_new_combinations(Combination<W> &c, int layer, int &added, int &symmetries) {
//			std::cout << "  Building on " << c << std::endl;
		assert(c.is_valid());
		assert(W == Z-1);
		// Build new combinations:
		for(int i = 0; i < W; i++) {
			LayerBrick brick = c.bricks[i];
			int bl = (int)c.bricks[i].layer;
			if(!(bl+1 == layer || bl-1 == layer)) {
				continue;
			}
			if(bl > layer +1) {
				break;
			}
			int w = 4, h = 2;
			if(c.bricks[i].brick.is_vertical) {
				w = 2;
				h = 4;
			}
		
			// horizontal:
			int xAdd = brick.brick.x-3;
			int yAdd = brick.brick.y-1;
			Combination<Z> c2;
			for(int x = 0; x < w+3; x++) {
				for(int y = 0; y < h+1; y++) {
					if(c.add(Brick(false, xAdd+x, yAdd+y), (uint8_t)layer, c2)) {
//								std::cout << "   Valid with " << LayerBrick(Brick(false, xAdd+x, yAdd+y),l) << ": " << c2;
//								std::cout << std::endl;
						add(c2, added, symmetries);
					}							
				}
			}
		
			// vertical:
			xAdd = brick.brick.x-1;
			yAdd = brick.brick.y-3;
			for(int x = 0; x < w+1; x++) {
				for(int y = 0; y < h+3; y++) {
					if(c.add(Brick(true, xAdd+x, yAdd+y), (uint8_t)layer, c2)) {
//								std::cout << "   Valid with " << LayerBrick(Brick(true, xAdd+x, yAdd+y),l) << ": " << c2;
//								std::cout << std::endl;
						add(c2, added, symmetries);
					}							
				}
			}					
		}
	}

	//void fill_from_readers(int &added, int &symmetries);
	void fill_from_readers(int &added, int &symmetries) {	
		// Build layers:
		int layers = 0, tkn = token;
		int layer_sizes[6];
		for(int i = 0; i < 6; i++) {
			int size_add = tkn % 10;
			if(size_add == 0)
				break;
			layers++;
			layer_sizes[i] = size_add;
			tkn /= 10;
		}
		for(int i = 0; i < layers/2; i++)
			std::swap(layer_sizes[i],layer_sizes[layers-i-1]);

		for(int i = 0; i < layers; i++) { // Reader from reader where brick on layer i was added:
			if(layer_sizes[i] == 1 && i != layers-1)
				continue;
			if(layers == 2 && layer_sizes[i-1] == Z-1 && Z != 2)
				continue;

			layer_sizes[i]--;
			//std::cout << "Creating reader for writer " << token << " with " << layers << " layers " << " shifting layer " << i << std::endl;
			CombinationReader<Z-1> reader(layer_sizes);	

			while(reader.has_next()) {
				Combination<Z-1> c = reader.next();
				make_new_combinations(c, i, added, symmetries);
			}
			layer_sizes[i]++;
		}
	}

};

/*
  Construct the writers.
  If remaining is 0, the writer of name token is constructed and added to writers.
  Otherwise, combinations of all remaining are constructed recursively.
*/
template <int Z>
void handle_combination_writers(int token, int remaining, bool add_self, int &added, int &symmetries) {
	if(remaining == 0) {		
		ptime t_init=microsec_clock::local_time();
		int before = added;
		std::cout << " Handling combinations for token " << token << std::endl;

		CombinationWriter<Z>(token).fill_from_readers(added, symmetries);

		std::cout << "  Constructed " << added << " combinations in ";
		double millis = (microsec_clock::local_time()-t_init).total_milliseconds();
		std::cout << millis << "ms." << std::endl;
	}
	else {
		//std::cout << " Building recurse writers for token " << token << ", remaining: " << remaining << ", add self?: " << add_self << std::endl;
		for(int i = remaining - (add_self ? 0 : 1); i > 0; i--) {
			int ntoken = 10*token + i;
			handle_combination_writers<Z>(ntoken, remaining - i, true, added, symmetries);
		}
	}
}

template <int Z>
void build_all_combinations() {
	ptime t_init=microsec_clock::local_time();
	std::cout << "Building all combinations of size " << Z << std::endl;
	int added = 0, symmetries = 0;
	if(Z == 1) {
		CombinationWriter<1> cw(1);
		bool res = cw.add(Combination<1>(), added, symmetries);
		assert(res);
		std::cout << "Constructed 1 combination of size 1 ignoring 1 symmetry in ";
		double millis = (microsec_clock::local_time()-t_init).total_milliseconds();
		std::cout << millis << "ms." << std::endl;
		return;
	}
	
    handle_combination_writers<Z>(0, Z, false, added, symmetries);

	std::cout << "Constructed " << added << " combinations of size " << Z << " ignoring " << symmetries << " symmetries in ";
	double millis = (microsec_clock::local_time()-t_init).total_milliseconds();
	std::cout << millis << "ms." << std::endl;
	if(Z == 2)
		assert(added == 24);
	if(Z == 3)
		assert(added == 1560);
	if(Z == 4)
		assert(added == 119580);
	if(Z == 5)
		assert(added == 10166403);
	if(Z == 6)
		assert(added == 915103765);
}

}

#endif
