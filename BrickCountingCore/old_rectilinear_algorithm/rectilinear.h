// -*- mode: c++; tab-width: 2; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=2 sts=2 sw=2 noet :

#ifndef BRICKS_RECTILINEAR_H
#define BRICKS_RECTILINEAR_H

#define ABS(a) ((a) < 0 ? -(a) : (a))
#define MAX(a,b) ((a) < (b) ? (b) : (a))
#define DIFF(a,b) ((a) < (b) ? (b)-(a) : (a)-(b))
#define MAX_BRICKS 11
#define MAX_HEIGHT 6
#define MAX_LAYER_SIZE 9

#include "stdint.h"
#include <stdarg.h>
#include <iostream>
#include <fstream>
#include <assert.h>
#include <sstream>
#include <chrono>
#include <set>
#include <map>
#include <algorithm>

extern uint64_t added, symmetries, earlyExits;

namespace rectilinear {

/**
 * A Brick represents a 2x4 LEGO brick with an orientation and x,y-position.
 * x,y is the position of the middle of the brick (like in LDRAW).
 */
struct Brick {
		Brick();
		Brick(bool iv, int8_t x, int8_t y);
		Brick(const Brick &b);

		bool operator <(const Brick& b) const;
		bool operator ==(const Brick& b) const;
		bool operator !=(const Brick& b) const;
		friend std::ostream& operator <<(std::ostream &os,const Brick &b);
		int cmp(const Brick& b) const;
		bool intersects(const Brick &b) const;
		uint64_t encode15() const;

		bool is_vertical:1;
		// Could use :6 when 6 bricks is maximum size, but it would not provide practical benefits.
		int8_t x:7, y:7;
};

const Brick FirstBrick = Brick();

template <int Z> // Combination with Z bricks
class Combination {
private:
    // State to check connectivity:
		bool connected[MAX_HEIGHT][MAX_LAYER_SIZE];
public:
		int layerSizes[MAX_HEIGHT], height;
		Brick bricks[MAX_HEIGHT][MAX_LAYER_SIZE]; // Layer, idx

		/*
			Init with one brick on layer 0 at 0,0.
		*/
		Combination() {
				bricks[0][0] = FirstBrick;
				layerSizes[0] = 1;
				height = 1;
		}

		void copy(const Combination<Z> &b) {
				assert(b.height > 0);
				height = b.height;
				#ifdef DEBUG
				int check = 0;
				#endif
				for(int i = 0; i < height; i++) {
						int s = b.layerSizes[i];
						layerSizes[i] = s;
						for(int j = 0; j < s; j++) {
								bricks[i][j] = b.bricks[i][j];
						}
  		 	 	  #ifdef DEBUG
				    check += s;
				    #endif
				}
				#ifdef DEBUG
				if(check != Z) {
						std::cerr << "Failed check in copy " << b << std::endl;
						assert(false);
				}
				#endif
		}

		Combination(const Combination<Z> &b) {
				copy(b);
		}

		inline friend std::ostream& operator << (std::ostream &os, const Combination<Z> &b) {
				os << "Combination |" << Z << "|, height " << b.height << ", layers ";
				for(int i = 0; i < b.height; i++) {
						os << b.layerSizes[i];
				}
				os << ", bricks";
				for(int i = 0; i < b.height; i++) {
						for(int j = 0; j < b.layerSizes[i]; j++) {
								os << " " << b.bricks[i][j] << " ";
						}
				}
				return os;
		}

		bool operator <(const Combination<Z>& b) const {
				assert(height == b.height);
				for(int i = 0; i < height; i++) {
						assert(layerSizes[i] == b.layerSizes[i]);
						int s = layerSizes[i];
						for(int j = 0; j < s; j++) {
								int res = bricks[i][j].cmp(b.bricks[i][j]);
								if(res != 0) {
										return res < 0;
								}
						}   
				}
				return false;
		}
		bool operator ==(const Combination& b) const {
				assert(height == b.height);
				for(int i = 0; i < height; i++) {
						assert(layerSizes[i] == b.layerSizes[i]);
						int s = layerSizes[i];
						for(int j = 0; j < s; j++) {
								int res = bricks[i][j].cmp(b.bricks[i][j]);
								if(res != 0) {
										return false;
								}
						}   
				}
				return true;
		}

		bool isValid(const std::string s) {
				if(bricks[0][0] != FirstBrick) {
						std::cerr << "Error: First brick not |0,0|: " << *this << " at " << s << std::endl;
						return false;
				}
				// Check ordering of bricks:
				for(int layer = 0; layer < height; layer++) {
						int s = layerSizes[layer];
						for(int i = 0; i+1 < s; i++) {
								if(!(bricks[layer][i] < bricks[layer][i+1])) {
										std::cerr << "Error: Invalid " << s << " combination: " << *this << " at layer " << layer << " for bricks " << i << " and " << (i+1) << std::endl;
										return false;
								}
						}
				}
				// Check for overlaps:
				for(int layer = 0; layer < height; layer++) {
						int s = layerSizes[layer];
						for(int i = 0; i+1 < s; i++) {
								const Brick &bi = bricks[layer][i];
								for(int j = i+1; j < s; j++) {
										const Brick &bj = bricks[layer][j];
										if(bi.intersects(bj)) {
												std::cout << "Bricks " << i << " and " << j << " of layer " << layer << " intersect in " << *this << std::endl;
												return false;
										}
								}
						}
				}
				// Check connectivity:
				if(!isConnected()) {
						std::cerr << "Not connected! " << *this << std::endl;
						return false;
				}
				return true;
		}

		void translateMinToOrigo() {
				int8_t minx = 127, miny = 127;

				for(int i = 0; i < layerSizes[0]; i++) {
						Brick &b = bricks[0][i];
						if(b.is_vertical) {
								// Vertical bricks in layer 0 can be 'min':
								if(b.x < minx || (b.x == minx && b.y < miny)) {
										minx = b.x;
										miny = b.y;
								}
						}
				}
				// Move all to new min:
				for(int i = 0; i < height; i++) {
						for(int j = 0; j < layerSizes[i]; j++) {
								bricks[i][j].x -= minx;
								bricks[i][j].y -= miny;
						}
				}
		}

		void sortBricks() {
				for(int layer = 0; layer < height; layer++) {
						int layerSize = layerSizes[layer];
						if(layerSize > 1) {
								std::sort(bricks[layer], &bricks[layer][layerSize]);
						}
				}
		}

		void rotate90() {
				for(int i = 0; i < height; i++) {
						for(int j = 0; j < layerSizes[i]; j++) {
								Brick &b = bricks[i][j];
								int8_t tmp = b.x; // std::swap does not work on int8_t!
								b.x = b.y;
								b.y = -tmp;
								b.is_vertical = !b.is_vertical;
						}
				}
				translateMinToOrigo();
				sortBricks();
				//assert(isValid("rotate90"));
		}

		void rotate180() {
				// Perform rotation:
				for(int i = 0; i < height; i++) {
						for(int j = 0; j < layerSizes[i]; j++) {
								Brick &b = bricks[i][j];
								b.x = -b.x;
								b.y = -b.y;
						}
				}
				translateMinToOrigo();
				sortBricks(); // TODO: Is std::reverse fast enough?
				//assert(isValid("rotate180"));
		}

		// TODO: Optimize by:
		// 1) Finding center of first layer.
		// 2) For each layer:
		//  - Check same center as first. If not, return false.
		//  - If a brick is at center, disregard it.
		//  - If odd number of remaining bricks: return false.
		//  - All bricks must have a corresponding brick across the center.
		// 3) return true
		bool is180Symmetric() const {
				Combination<Z> c(*this);
				c.rotate180();
				return *this == c;
		}

		bool isSymmetricAboveFirstLayer() const {
				Combination<Z-1> c;
				c.height = height-1;
				for(int i = 1; i < height; i++) {
						c.layerSizes[i-1] = layerSizes[i];
						for(int j = 0; j < layerSizes[i]; j++) {
								c.bricks[i-1][j] = bricks[i][j];
						}
				}
				c.normalize();
				Combination<Z-1> c2(c);
				c2.rotate180();
				return c == c2;
		}

		bool isConnectedAboveFirstLayer() const {
				Combination<Z-1> c;
				c.height = height-1;
				for(int i = 1; i < height; i++) {
						c.layerSizes[i-1] = layerSizes[i];
						for(int j = 0; j < layerSizes[i]; j++) {
								c.bricks[i-1][j] = bricks[i][j];
						}
				}
				//c.normalize(); // TODO: Needed?
				return c.isConnected();
		}

		bool can_rotate90() const {
				if(Z <= 2)
						return false; // Not possible to rotate 2 brick models 90 degrees.
				for(int i = 0; i < layerSizes[0]; i++) {
						if(!bricks[0][i].is_vertical) {
								return true;
						}
				}
				return false;
		}

		/**
		 * Copy this combination and add b at layer.
		 * Assume the brick is connected to one of the existing bricks.
		 */
		template <int W>
		bool addBrick(const Brick &b, const uint8_t layer, Combination<W> &out, int &rotated) const {
				assert(W == Z+1);
				int layerSize = layer == height ? 0 : layerSizes[layer];
				for(int i = 0; i < layerSize; i++) {
						const Brick &b2 = bricks[layer][i];
						if(b2.intersects(b)) {
								return false; // Intersects!
						}
				}

				// Set up out-configuration:
				out.height = height;
				for(int i = 0; i < height; i++) {
						int s = layerSizes[i];
						if(i == layer) {
								out.layerSizes[i] = s+1;
								continue; // Handle this layer separately
						}
						out.layerSizes[i] = s;
						for(int j = 0; j < s; j++) {
								out.bricks[i][j] = bricks[i][j];
						}
				}
				if(height == layer) {
						out.height = height + 1;
						out.layerSizes[height] = 1;
				}
				
				// For layer, add by finding first brick not smaller than b (simple merge):
				int in = 0;
				for(int i = 0; i < layerSize; i++) {
						if(in == 0) {
								const Brick &b2 = bricks[layer][i];
								if(b < b2) { // Merge in:
										in = 1;
										out.bricks[layer][i] = b;
								}
						}
						out.bricks[layer][i+in] = bricks[layer][i];
				}
				// Finish up:
				if(in == 0) {
						out.bricks[layer][layerSize] = b; // Add to the end
				}

				out.normalize(rotated);
				return true;
		}

		template <int W>
		void removeSingleLowerBrick(Combination<W> &out) const {
				assert(W == Z-1);
				out.height = height-1;
				for(int layer = 0; layer+1 < height; layer++) {
						int s = layerSizes[layer+1];
						out.layerSizes[layer] = s;
						for(int i = 0; i < s; i++) {
								out.bricks[layer][i] = bricks[layer+1][i];
						}
				}
		}
				
		template <int W>
		void removeSingleTopBrick(Combination<W> &out) const {
				assert(W == Z-1);
				out.height = height-1;
				for(int layer = 0; layer < height-1; layer++) {
						int s = layerSizes[layer];
						out.layerSizes[layer] = s;
						for(int i = 0; i < s; i++) {
								out.bricks[layer][i] = bricks[layer][i];
						}
				}
		}
				
		template <int W>
		bool removeBrickAt(int layer, int idx, Combination<W> &out) const {
				assert(W == Z-1);
				int layerSize = layerSizes[layer];
				if(layerSize == 1) {
						if(layer == 0) {
								// Special case: Single lower brick removal
								removeSingleLowerBrick(out);
						}
						else if(layer == height-1) {
								// Special case: Top brick
								removeSingleTopBrick(out);
						}
						else {
								return false; // Single last brick in non-extreme layer!
						}
				}

				out.height = height;
				for(int i = 0; i < height; i++) {
						int s = layerSizes[i];
						if(i == layer) {
								out.layerSizes[i] = s-1;
								continue;
						}
						out.layerSizes[i] = s;
						for(int j = 0; j < s; j++) {
								out.bricks[i][j] = bricks[i][j];
						}
				}
				
				// Remove brick at index idx:
				for(int i = 0, outIdx = 0; i < Z; i++) {
						if(idx == i) {
								continue;
						}
						out.bricks[layer][outIdx++] = bricks[layer][i];
				}

				// Check that combination is still connected:
				if(!out.isConnected()) {
						return false;
				}
		
				out.normalize();

				return true; // All OK.
		}
		
		void normalize(int &rotated) {
				// Ensure FirstBrick is first and all is sorted:
				bool hasVerticalLayer0Brick = false;
				for(int i = 0; i < layerSizes[0]; i++) {
						Brick &b = bricks[0][i];
						if(b.is_vertical) {
								hasVerticalLayer0Brick = true;
								break;
						}
				}

				// Check if first brick is horizontal at 0,0:
				if(hasVerticalLayer0Brick) {
						translateMinToOrigo();
						sortBricks();
				}
				else {
						rotate90();
						rotated = 90;
				}
				
				Combination c(*this);
				if(can_rotate90()) {
						for(int i = 0; i < 3; i++) {
								c.rotate90();
								if(c < *this) {
										copy(c);
										rotated = 90*(i+1);
								}
						}
				}
				else {
						c.rotate180();
						if(c < *this) {
								copy(c);
								rotated = 180;
						}
				}
		}
		void normalize() {
				int ignore;
				normalize(ignore);
		}
 
		int countConnected(int layer, int idx) {
				connected[layer][idx] = true;
				const Brick &b = bricks[layer][idx];
				int ret = 1;
				// Add for layer below:
				if(layer > 0) {
						int s = layerSizes[layer-1];
						for(int i = 0; i < s; i++) {
								if(connected[layer-1][i]) {
										continue;
								}
								const Brick &b2 = bricks[layer-1][i];
								if(b.intersects(b2)) {
										ret += countConnected(layer-1, i);
								}
						}
				}
				// Add for layer above:
				if(layer < height-1) {
						int s = layerSizes[layer+1];
						for(int i = 0; i < s; i++) {
								if(connected[layer+1][i]) {
										continue;
								}
								const Brick &b2 = bricks[layer+1][i];
								if(b.intersects(b2)) {
										ret += countConnected(layer+1, i);
								}
						}
				}
				return ret;
		}

		bool isConnected() {
				// Reset state:
				for(int i = 0; i < height; i++) {
						int s = layerSizes[i];
						for(int j = 0; j < s; j++) {
								connected[i][j] = false;
						}
				}
				// Run DFS:
				int cnt = countConnected(0, 0);

				return cnt == Z;
		}

		void flip() {
				Brick tmp[MAX_BRICKS-1];
				for(int layer1 = 0, layer2 = height-1; layer1 < layer2; layer1++, layer2--) {
						int s1 = layerSizes[layer1];
						int s2 = layerSizes[layer2];
						// Save layer 1 in tmp:
						for(int i = 0; i < s1; i++)
								tmp[i] = bricks[layer1][i];
						// Move layer2 to layer1:
						for(int i = 0; i < s2; i++)
								bricks[layer1][i] = bricks[layer2][i];
						// Move tmp to layer 2:
						for(int i = 0; i < s1; i++)
								bricks[layer2][i] = tmp[i];
				}
				std::reverse(layerSizes, &layerSizes[height]);
				normalize();
		}

		uint64_t encodeFor12(bool &topSymmetric, bool &topConnected) const {
				assert(layerSizes[0] == 1);
				assert(layerSizes[1] == 2);

				uint64_t ret = 0;

				// Encode bricks:
				for(int i = 0; i < 2; i++) {
						for(int j = 0; j < layerSizes[i]; j++) {
								ret = (ret << 15) | bricks[i][j].encode15();
						}
				}
				
				// Additional bits:
				topSymmetric = isSymmetricAboveFirstLayer();
				topConnected = isConnectedAboveFirstLayer();
				
				return (ret << 2) | (topSymmetric << 1) | topConnected;
		}

		uint64_t countSymmetricLayer0Siblings() const {
				assert(height >= 2);
				assert(layerSizes[0] > 1);

				uint64_t ret = 0;
				int X = bricks[1][0].x + bricks[1][1].x;
				int Y = bricks[1][0].y + bricks[1][1].y;
				std::set<std::pair<int,int> > v, h, *seen;
				for(int i = 0; i < layerSizes[0]; i++) {
						const Brick &b = bricks[0][i];
						int x = b.x;
						int y = b.y;
						std::pair<int,int> p2(X-x, Y-y);
						seen = b.is_vertical ? &v : &h;
						if(seen->find(p2) == seen->end()) {
								std::pair<int,int> p1(x,y);
								seen->insert(p1);
						}
						else {
								ret++;
						}
				}
				return ret;
		}

		uint64_t countSymmetricLayer0SiblingsThatConnectAbove() const {
				assert(layerSizes[0] > 1);
				assert(layerSizes[1] == 2);

				uint64_t ret = 0;
				int X = bricks[1][0].x + bricks[1][1].x;
				int Y = bricks[1][0].y + bricks[1][1].y;
				std::set<std::pair<int,int> > v, h, *seen;
				for(int i = 0; i < layerSizes[0]; i++) {
						const Brick &b = bricks[0][i];
						if(!b.intersects(bricks[1][0]) || !b.intersects(bricks[1][1])) {
								continue; // Not connecting above
						}
						
						int x = b.x;
						int y = b.y;
						std::pair<int,int> p2(X-x, Y-y);
						seen = b.is_vertical ? &v : &h;
						if(seen->find(p2) == seen->end()) {
								std::pair<int,int> p1(x,y);
								seen->insert(p1);
						}
						else {
								ret++;
						}
				}
				return ret;
		}
};

template <int Z>
class CombinationReader {
private:
		std::ifstream *istream; // Reads reversed combinations
		int layerSizes[MAX_HEIGHT], // Reversed
				height, combinationsLeft;
		uint8_t bits, bitIdx, brickLayer;
		Combination<Z-1> baseCombination; // Reversed!
		bool reverse, done;
		uint64_t combinationCounter;
		std::string name;

public:
		CombinationReader(const int layerSizes[]) {
				combinationCounter = 0;
				done = false;
				height = 0;
				bits = 0;
				bitIdx = 8; // Ensure a byte is read next time

				std::stringstream ss, ss2;
				ss << Z << "/";

				int layerSize = 0;
				int size_total = 0;
				for(int i = 0; size_total < Z; i++) {
						layerSize = layerSizes[i];
						this->layerSizes[i] = layerSize;
						ss2 << layerSize;
						size_total += layerSize;
						height++;
				}
				assert(Z == size_total);

				std::string layerString = ss2.str(), layerStringReverse = layerString;
				std::reverse(layerStringReverse.begin(), layerStringReverse.end());				
				reverse = layerString < layerStringReverse; // Reverse => Read upside-down.

				name = layerString; // Do not reverse
				ss << (reverse ? layerStringReverse : layerString);
				std::string file_name = ss.str();

				bool invalid = height == 2 && size_total == 8 && (layerSizes[0] == 1 || layerSizes[1] == 1);
				if(Z > 1 && !invalid) {
						istream = new std::ifstream(file_name.c_str(), std::ios::binary);
				}
				else {
						istream = NULL;
				}
				if(invalid) {
						done = true;
				}

				if(reverse) {
						std::reverse(this->layerSizes, &this->layerSizes[height]);
				}

				baseCombination.bricks[0][0] = FirstBrick;
				combinationsLeft = 0;

				std::cout << "   READER for " << layerString << ": " << file_name << ", reversed: " << reverse << ", invalid: " << invalid << std::endl;
		}

		~CombinationReader() {
				if(istream != NULL) {
						std::cout << "  Closing combination reader " << name << ". Combinations read: " << combinationCounter << std::endl;
				
						istream->close();
						delete istream;
				}
		}

		bool readBit() {
				if(bitIdx == 8) {
						istream->read((char*)&bits, 1);
						bitIdx = 0;
				}
				bool bit = (bits >> (7-bitIdx)) & 1;
				bitIdx++;
				return bit;
		}

		int8_t readInt8() {
				int8_t ret = 0;
				bool negate = readBit();
				for(int i = 0; i < 6; i++) {
						ret = ret | ((int)readBit() << i);
				}
				if(negate)
						ret = -ret;
				return ret;
		}

		uint8_t readUInt4() {
				uint8_t ret = 0;
				for(int i = 0; i < 4; i++) {
						ret = ret | ((int)readBit() << i);
				}
				return ret;
		}
		
		uint32_t readUInt32() {
				uint32_t ret = 0;
				for(int block = 0; block < 4; block++) {
						for(int i = 0; i < 8; i++) {
								ret = ret | ((int)readBit() << i);
						}
				}
				return ret;
		}
		
		void readBrick(Brick &b) {
				b.is_vertical = readBit();
				b.x = readInt8();
				b.y = readInt8();
		}

		// Assume c already has height and layerSizes set.
		bool readCombination(Combination<Z> &c) {
				if(done) {
						return false;
				}
				if(Z == 1) {
						c.layerSizes[0] = 1;
						c.bricks[0][0] = FirstBrick;
						c.height = 1;
						done = true;
						return true;
				}

				if(combinationsLeft == 0) { // Read new base combination:
						brickLayer = readUInt4(); // Brick to be added to base combination.
						if(brickLayer == 15) {
								done = true;
								return false;
						}

						baseCombination.height = height - (brickLayer == height-1 && layerSizes[brickLayer] == 1);

						for(int i = 0; i < baseCombination.height; i++) {
								int s = layerSizes[i] - (brickLayer == i);
								baseCombination.layerSizes[i] = s;

								for(int j = (i == 0 ? 1 : 0); j < s; j++) {
										readBrick(baseCombination.bricks[i][j]);
								}
						}
						
						combinationsLeft = readUInt32();
						assert(baseCombination.isValid("Base combination"));
				}

				// Copy base combination and add one brick:
				c.height = height;
				for(int i = 0; i < height; i++) {
						c.layerSizes[i] = layerSizes[i];
						int in = 0;
						if(i == brickLayer) {
								readBrick(c.bricks[i][0]);
								in = 1;
						}
						int s = (i == baseCombination.height) ? 0 : baseCombination.layerSizes[i];
						for(int j = 0; j < s; j++) {
								c.bricks[i][j+in] = baseCombination.bricks[i][j];
						}
				}
				combinationsLeft--;

				if(reverse) {
						c.flip();
				}
				else {
						c.normalize(); // Because brickLayer might not be sorted
				}
				assert(c.isValid("Read"));

				#ifdef TRACE
				std::cout << "  Reading combination " << combinationCounter << " of type " << name << ": " << c << std::endl;
				#endif

				combinationCounter++;

				return true;
		}
};

template <int Z>
class CombinationWriter {
		std::ofstream *ostream;
		int token, height; // eg. 211 for 4 bricks in config 2-1-1
		std::set<Combination<Z> > combinations; // Only used by assertion!
		uint8_t bits, cntBits;
		uint64_t writtenFull, writtenShort;

public:
		CombinationWriter(int token, bool saveOutput) : token(token) {
				height = 0;

				#ifdef DEBUG
				std::cout << " Create Combination writer for token " << token << ", output?: " << saveOutput << std::endl;
				#endif
				// inverse token:
				int it = 0;
				while(token > 0) {
						it *= 10;
						int size_add = token % 10;
						it += size_add;
						token /= 10;
						height++;
				}

				std::stringstream ss;
				ss << Z << "/";
	
				while(it > 0) {
						ss << (it % 10);
						it /= 10;
				}

				std::string file_name = ss.str();

				if(saveOutput) {
						ostream = new std::ofstream(file_name.c_str(), std::ios::binary);
						bits = cntBits = 0;
				}
				else
						ostream = NULL;
				writtenFull = 0;
				writtenShort = 0;
		}

		void writeBit(bool bit) {
				bits = (bits << 1) + (bit ? 1 : 0);
				cntBits++;
				if(cntBits == 8) {
						ostream->write((char*)&bits, 1);
						cntBits = 0;
				}
		}

		void flushBits() {
				while(cntBits > 0) {
						writeBit(0);
				}
		}

		// Max difference from first brick is for 11 bricks: 10*3 = 30 studs, so 6 bits suffice:
		void writeInt8(const int8_t toWrite) {
				int i = toWrite;
				if(i < 0) {
						writeBit(true); // minus
						i = -i;
				}
				else {
						writeBit(false); // plus
				}
				for(int j = 0; j < 6; j++) {
						writeBit(i & 1);
						i >>= 1;
				}
				assert(i == 0); // At most 6 bits used.
		}

		void writeUInt4(const uint8_t toWrite) {
				uint8_t i = toWrite;
				for(int j = 0; j < 4; j++) {
						writeBit(i & 1);
						i >>= 1;
				}
		}

		void writeUInt32(uint32_t toWrite) {
				for(int j = 0; j < 32; j++) {
						writeBit(toWrite & 1);
						toWrite >>= 1;
				}
		}

		void writeBrick(const Brick &b) {
				writeBit(b.is_vertical);
				writeInt8(b.x);
				writeInt8(b.y);
		}

		template <int W>
		void writeCombinations(const Combination<W> &baseCombination, uint8_t brickLayer, std::vector<Brick> &v) {
				assert(W == Z-1);
				if(ostream == NULL || v.empty()) {
						return;
				}

				writeUInt4(brickLayer);
				for(int i = 0; i < baseCombination.height; i++) {
						int s = baseCombination.layerSizes[i];
						for(int j = (i == 0 ? 1 : 0); j < s; j++) {
								writeBrick(baseCombination.bricks[i][j]);
						}
				}
				writeUInt32(v.size());

				for(int i = 0; i < v.size(); i++) {
						writeBrick(v[i]);
				}

				writtenFull++;
				writtenShort+=v.size();
		}

		template <int W>
		bool add(Combination<W> &cOld, const Brick &addedBrick, const uint8_t layer) {
				#ifdef DEBUG
				std::cout << "   TRY NEW " << addedBrick << " at layer " << (int)layer << std::endl;
        #endif
				assert(W == Z-1);
				Combination<Z> c; // Build completely in addBrick()
				int rotated = 0;
				if(!cOld.addBrick(addedBrick, layer, c, rotated)) {
  				  #ifdef DEBUG
				    std::cout << " Does not fit!" << std::endl;
            #endif
						return false; // Does not fit.
				}
				#ifdef DEBUG
				std::cout << "   FIT OK" << std::endl;
        #endif

				// If cOld is symmetric, then check if we are first:
				if(cOld.is180Symmetric() && rotated > 90) {
  				  #ifdef DEBUG
				    std::cout << " Not first version of symmetric combination!" << std::endl;
            #endif
						return false; // Not first version of this symmetric combination.
				}

				// Try to remove bricks b from c up to and including layer of addedBrick (unless layer only has 1 brick)
				// If b is lower layer than addedBrick and combination stays connected, then do not add!
				// If b same layer as addedBrick: Do not add if combination without b < combination without addedBrick
				Combination<W> ignore, sibling;
				for(int i = MAX(0, layer-1); i < layer; i++) {
						for(int j = 0; j < c.layerSizes[i]; j++) {
								if(c.layerSizes[i] > 1 && c.removeBrickAt(i, j, ignore)) {
										#ifdef DEBUG
										std::cout << "  Could be constructed from lower refinement " << ignore << std::endl;
										#endif
										return false; // Sibling comes before cOld, so we do not add
								}
						}
				}

				int s = c.layerSizes[layer];
				if(s >= 2) { // If there is at least one other brick at the layer:
						for(int j = 0; j < s; j++) {
                #ifdef DEBUG
								std::cout << "   Check same layer!" << std::endl;
						    #endif
								if(c.removeBrickAt(layer, j, sibling)) { // Can remove siblingBrick:
                    #ifdef DEBUG
										std::cout << "Comparing sibling " << sibling << " with old " << cOld << std::endl;
								    #endif
										if(sibling < cOld) {
                        #ifdef DEBUG
												std::cout << "  Could be constructed from lesser sibling" << sibling << std::endl;
										    #endif
												return false; // Lesser sibling!
										}
								}
						}
				}

				// All OK! c can be added and counted:
				#ifdef DEBUG
				std::cout << " ALL OK! Adding " << c << std::endl;
				#endif

				assert(combinations.insert(c).second);
				assert(c.isValid("Counting"));

				// Update counters:
				added++;
				if(     (added-1)%1000000000 == 1000000000-1)
						std::cout << " " << added/1000000000 << " billion counted" << std::endl;
				else if((added-1)%100000000 == 100000000-1)
						std::cout << "#" << std::flush;
				else if((added-1)%10000000 == 10000000-1)
						std::cout << ":" << std::flush;
				else if((added-1)%1000000 == 1000000-1)
						std::cout << "." << std::flush;
				Combination<Z> rotated180(c);
				rotated180.rotate180();
				if(c == rotated180) {
						symmetries++;
				}
				// TODO: What about 90 degree symmetries?
				return true;
		}

		~CombinationWriter() {
				#ifdef DEBUG
				std::cout << "  Written combinations full: " << writtenFull << ", short: " << writtenShort << " Compression% " << (writtenShort)*100.0/(writtenFull+writtenShort) << std::endl;
				#endif
				if(ostream != NULL) {
						std::cout << "   Closing write stream for " << token << std::endl;
						writeUInt4(15); // Mark end of file.
						flushBits();
						ostream->flush();
						ostream->close();
						delete ostream;
				}
		}

		template <int W>
		void makeNewCombinations(Combination<W> &c, const int layer, std::set<Combination<Z> > &_old, std::set<Combination<Z> > &_new) {
        #ifdef TRACE
				std::cout << "-----------------------------" << std::endl;
				std::cout << "Building on " << c << std::endl;
				#endif
				assert(c.isValid("makeNewCombinations"));
				assert(W == Z-1);

				// Stop early if c is stable with brick below layer-1 removed!
				Combination<W-1> evenSmaller;
				for(int i = 0; i < layer-1; i++) {
						int s = c.layerSizes[i];
						if(s == 1) {
								continue; // Do not remove brick if it is the only one in the layer!
						}
						for(int j = 0; j < s; j++) {
								if(c.removeBrickAt(i, j, evenSmaller)) {
										earlyExits++;
										#ifdef DEBUG
										std::cout << "EARLY EXIT due to stable base on layer " << i << ": " << c << " -> " << evenSmaller << std::endl;
										#endif
										if((earlyExits-1)%1000000 == 1000000-1)
												std::cout << "X" << std::flush;
										return;
								}
						}
				}
				
        // Build new combinations by adding to all existing bricks:
				std::set<Brick> alreadyAdded, neighbours;
				std::vector<Brick> v;

				// Find all potential neighbours above and below:
				if(layer > 0) {
						int s = c.layerSizes[layer-1];
						for(int j = 0; j < s; j++) {
								Brick &b = c.bricks[layer-1][j];
								neighbours.insert(b);
						}
				}
				if(layer+1 < c.height) {
						int s = c.layerSizes[layer+1];
						for(int j = 0; j < s; j++) {
								Brick &b = c.bricks[layer+1][j];
								neighbours.insert(b);
						}
				}
				
				for(std::set<Brick>::const_iterator it = neighbours.begin(); it != neighbours.end(); it++) {
						const Brick &brick = *it;
            #ifdef DEBUG
				    std::cout << " Try adding to brick " << brick << std::endl;
            #endif

						bool isVertical = brick.is_vertical;

						// Add crossing bricks (one vertical, one horizontal):
						for(int x = -2; x < 3; x++) {
								for(int y = -2; y < 3; y++) {
										Brick b(!isVertical, brick.x+x, brick.y+y);
										if(alreadyAdded.insert(b).second) { // Brick not already added:
												#ifdef DEBUG
												std::cout << "  TRY ADDING CROSSING " << b << " on layer " << layer << std::endl;
												int ignore;
												Combination<Z> c2;
												if(c.addBrick(b, layer, c2, ignore)) {
														std::cout << "   OLD OK" << std::endl;
														_old.insert(c2);
												}
												uint64_t before = added;
												#endif
												if(add(c, b, layer)) {
														v.push_back(b);
												}
												#ifdef DEBUG
												if(before == added-1) {
														std::cout << "   NEW OK" << std::endl;
														_new.insert(c2);
												}
												#endif
										}
								}
						}

						// Add parallel bricks:
						int w = 4, h = 2;
						if(isVertical) {
								w = 2;
								h = 4;
						}		
						for(int y = -h+1; y < h; y++) {
								for(int x = -w+1; x < w; x++) {
										Brick b(isVertical, brick.x+x, brick.y+y);
										if(alreadyAdded.insert(b).second) {
                        #ifdef DEBUG
												std::cout << "  TRY ADDING PARALLEL " << b << " on layer " << layer << std::endl;
												int ignore;
												Combination<Z> c2;
												if(c.addBrick(b, layer, c2, ignore)) {
														std::cout << "   OLD OK" << std::endl;
														_old.insert(c2);
												}
												uint64_t before = added;
												#endif
												if(add(c, b, layer)) {
														v.push_back(b);
												}
												#ifdef DEBUG
												if(before == added-1) {
														std::cout << "   NEW OK" << std::endl;
														_new.insert(c2);
												}
												#endif
										}
								}
						}
            #ifdef DEBUG
				    std::cout << " Done adding to brick " << brick << std::endl;
            #endif
				}

				// Write:
				writeCombinations<W>(c, layer, v);
				
        #ifdef TRACE
				std::cout << "DONE Building on " << c << std::endl;
				std::cout << "-----------------------------" << std::endl;
				#endif
		}

		void fillFromReaders() {
 		    std::set<Combination<Z> > _old, _new;
        #ifdef DEBUG
				std::cout << "  Filling from readers. added=" << added << ", symmetries=" << symmetries << std::endl;
        #endif
				
				// Build layers:
				int layers = 0, tkn = token;
				int layerSizes[MAX_HEIGHT];
				while(true) {
						int size_add = tkn % 10;
						if(size_add == 0)
								break;
						layerSizes[layers++] = size_add;
						tkn /= 10;
				}
				// Flip the layer sizes:
				for(int i = 0; i < layers/2; i++) {
						std::swap(layerSizes[i], layerSizes[layers-i-1]);
				}

				// Combination to read:
				Combination<Z-1> c;
				c.height = height;
				for(int i = 0; i < height; i++) {
						c.layerSizes[i] = layerSizes[i];
				}
				
				for(int i = 0; i < layers; i++) { // Reader from reader where brick on layer i was added:
						if(layerSizes[i] == 1 && i != layers-1)
								continue;
						if(layers == 2 && i > 0 && layerSizes[i-1] == Z-1 && Z != 2)
								continue;

						layerSizes[i]--;
						CombinationReader<Z-1> reader(layerSizes);

						while(reader.readCombination(c)) {
								makeNewCombinations(c, i, _old, _new);
						}
						layerSizes[i]++;
				}

  	  	#ifdef DEBUG
				std::cout << "Checking old vs new algorithm" << std::endl;
				#endif
				// Find incorrectly counted:
				if(_old.size() > 0) {
						std::cout << "Old algorithm: " << _old.size() << std::endl;
						typename std::set<Combination<Z> >::iterator it;
						for(it = _old.begin(); it != _old.end(); it++) {
								if(_new.find(*it) == _new.end()) {
										std::cout << "Missing combination: " << *it << std::endl;
								}
						}
				}
		}

};

/*
  Construct the writers.
  If remaining is 0, the writer of name token is constructed and added to writers.
  Otherwise, combinations of all remaining are constructed recursively.
*/
template <int Z>
void handleCombinationWriters(int token, int remaining, bool add_self, bool saveOutput) {
		if(remaining == 0) {
				std::chrono::time_point<std::chrono::steady_clock> t_init = std::chrono::steady_clock::now();
				uint64_t before = added, before_s = symmetries;
				std::cout << " Handling combinations for token " << token << std::endl;

				CombinationWriter<Z> writer(token, saveOutput);
				#ifdef TRACE
				std::cout << "  Writer constructed" << std::endl;
				#endif
				writer.fillFromReaders();

				std::cout << "  Constructed " << (added-before) << " combinations, of which " << (symmetries-before_s) << " are symmetric in ";
				std::chrono::time_point<std::chrono::steady_clock> t_end = std::chrono::steady_clock::now();
				std::chrono::duration<double> t = t_end - t_init;
				std::cout << t.count() << "s. Early exits: " << earlyExits << std::endl;
				return;
		}

		#ifdef TRACE
		std::cout << " Building recurse writers for token " << token << ", remaining: " << remaining << ", add self?: " << add_self << std::endl;
		#endif
		for(int i = remaining - (add_self ? 0 : 1); i > 0; i--) {
				int ntoken = 10*token + i;
				handleCombinationWriters<Z>(ntoken, remaining - i, true, saveOutput);
		}
}







template <int Z, int Y, int layer0Size>
void countLayer0P(Combination<Z> &symmetryChecker, int idx, std::set<Brick>::const_iterator itBegin, std::set<Brick>::const_iterator itEnd, uint64_t &cnt, uint64_t &cntSymmetric, const uint64_t divisor, bool topSymmetric, bool topConnected, std::set<Combination<Z> > &seen) {
		if(idx == layer0Size-1) {
				Combination<Z> c(symmetryChecker);
				c.normalize();

				if(seen.find(c) != seen.end()) {
						return; // Already seen!
				}
				seen.insert(c);

				assert(c.height >= 2);
				assert(c.layerSizes[1] == Y);
				assert(c.layerSizes[0] == layer0Size);
				bool isSymmetric = c.is180Symmetric();
				uint64_t timesCounted;

				if(!topConnected) {
						uint64_t connectsAbove = 0;
						for(int i = 0; i < layer0Size; i++) {
								Brick &b = c.bricks[0][i];
								if(b.intersects(c.bricks[1][0]) && b.intersects(c.bricks[1][1])) {
										connectsAbove++;
								}
						}
						
						if(topSymmetric) {
								uint64_t s = c.countSymmetricLayer0SiblingsThatConnectAbove();
								timesCounted = connectsAbove - s;
						}
						else {
								// Top is not connected and not symmetric.
								// 
								timesCounted = connectsAbove;
						}
				}
				else { // topConnected:
						// Top is connected, so each brick in layer 0 causes a count... unless
						// top is symmetric and bricks below have symmetric sibling
						if(topSymmetric) {
								uint64_t s = c.countSymmetricLayer0Siblings();
								timesCounted = layer0Size - s;
						}
						else {
								timesCounted = layer0Size; // Once for each brick
						}
				}

				cnt += divisor / timesCounted;
				if(topSymmetric && isSymmetric) {
						cntSymmetric += divisor / timesCounted;
				}

				#ifdef TRACE
				//if(topConnected && topSymmetric && timesCounted == 2) {
				if(!topConnected && timesCounted == 3) {
						std::cout << std::endl;
						std::cout << c << std::endl;
						std::cout << "topConnected " << topConnected << std::endl;
						std::cout << "isSymmetric " << isSymmetric << std::endl;
						std::cout << "topSymmetric " << topSymmetric << std::endl;
						std::cout << "timesCounted " << timesCounted << std::endl;
						std::cout << "countSymmetricLayer0SiblingsThatConnectAbove " << c.countSymmetricLayer0SiblingsThatConnectAbove() << std::endl;
						std::cout << "countSymmetricLayer0Siblings " << c.countSymmetricLayer0Siblings() << std::endl;
						int *a = NULL; a[2] = 4;
				}
				#endif

				return;
		}
		
		for(std::set<Brick>::const_iterator it = itBegin; it != itEnd; it++) {
				const Brick b = *it;
				// Check that b does not collide:
				bool ok = true;
				for(int i = Z-1; i < idx+1; i++) {
						if(symmetryChecker.bricks[0][i].intersects(b)) {
								ok = false;
								break;
						}
				}
				if(!ok) {
						continue; // Intersection with previously placed brick!
				}
				
				symmetryChecker.bricks[0][idx+1] = b;
				std::set<Brick>::const_iterator nxt = it;
				nxt++;
				countLayer0P<Z,Y,layer0Size>(symmetryChecker, idx+1, nxt, itEnd, cnt, cntSymmetric, divisor, topSymmetric, topConnected, seen);
		}
}

/*
	Place "toPlace" bricks on layer 0 in all ways possible.
 */
template <int Z, int Y, int layer0Size>
void countLayer0Placements(Combination<Z-layer0Size+Y-1> &c, uint64_t &cnt, uint64_t &cntSymmetric, const uint64_t divisor, bool topSymmetric, bool topConnected, std::set<Combination<Z> > &seen) {
		assert(c.height >= 2);
		assert(c.layerSizes[0] == Y-1);
		assert(c.layerSizes[1] == Y);
		Brick *blockers = c.bricks[0];

		// Find legal placements v:
		std::set<Brick> s;
		for(int i = 0; i < 2; i++) {
				Brick &b = c.bricks[1][i];
				bool isVertical = b.is_vertical;

				// Add crossing bricks (one vertical, one horizontal):
				for(int x = -2; x < 3; x++) {
						for(int y = -2; y < 3; y++) {
								Brick b2(!isVertical, b.x+x, b.y+y);
								bool ok = true;
								for(int i = 0; i < Y-1; i++) {
										if(blockers[i].intersects(b2)) {
												ok = false;
												break;
										}
								}
								if(ok) {
										s.insert(b2);
								}
						}
				}
				// Add parallel bricks:
				int w = 4, h = 2;
				if(isVertical) {
						w = 2;
						h = 4;
				}		
				for(int y = -h+1; y < h; y++) {
						for(int x = -w+1; x < w; x++) {
								Brick b2(isVertical, b.x+x, b.y+y);
								bool ok = true;
								for(int i = 0; i < Y-1; i++) {
										if(blockers[i].intersects(b2)) {
												ok = false;
												break;
										}
								}
								if(ok) {
										s.insert(b2);
								}
						}
				}
		}

		// Construct symmetry checker c2:
		Combination<Z> c2;
		c2.height = c.height;
		c2.layerSizes[0] = layer0Size;
		for(int i = 0; i < c.height; i++) {
				c2.layerSizes[i] = c.layerSizes[i];
				for(int j = 0; j < c.layerSizes[i]; j++) {
						c2.bricks[i][j] = c.bricks[i][j];
				}
		}

		countLayer0P<Z,Y,layer0Size>(c2, 0, s.begin(), s.end(), cnt, cntSymmetric, divisor, topSymmetric, topConnected, seen);
}

/*
	Special case: First layer has at least 2 bricks, while second layer has 2.
	Call it <X2...>
  Iterate over <12...>
	Add remaining bricks to first layer and count how many ways that can be done.
 */
template <int Z, int layer0Size>
void countX2(char* input) {
		int smallerToken = 1; // Just have 1 in first layer of smaller
		int smallerLayerSizes[MAX_HEIGHT];
		smallerLayerSizes[0] = 1;
		char c;
		for(int i = 1; (c = input[i]); i++) {
				smallerToken = smallerToken * 10 + (c-'0');
				smallerLayerSizes[i] = (c-'0');
		}
		std::cout << "Constructing to " << layer0Size << " bricks for first layer of <" << smallerToken << ">" << std::endl;

		// Go through all smaller combinations:
		uint64_t cnt = 0, cntSymmetric = 0, divisor = 5*6*7*8*9, countSmaller = 0, cntSkip = 0, preCount = 0;

		Combination<Z-layer0Size+1> smaller;
		{
				CombinationReader<Z-layer0Size+1> preReader(smallerLayerSizes);
				while(preReader.readCombination(smaller)) {
						preCount++;
						if((preCount-1)%1000000000 == 1000000000-1)
								std::cout << " " << preCount/1000000000 << " billion" << std::endl;
						else if((preCount-1)%100000000 == 100000000-1)
								std::cout << ":" << std::flush;	
						else if((preCount-1)%10000000 == 10000000-1)
								std::cout << "." << std::flush;	
				}
				// In own space to ensure proper closing of file before next read.
		}
		std::cout << "Number of smaller configuration to build on: " << preCount << std::endl;
		
		std::map<uint64_t,uint64_t> x2Cnt, x2Sym; // Memory of results
		
		CombinationReader<Z-layer0Size+1> reader(smallerLayerSizes);
		while(reader.readCombination(smaller)) {
				countSmaller++;
				
				// Find all locations to place a brick on first layer:
				bool topSymmetric, topConnected;
				uint64_t encoded = smaller.encodeFor12(topSymmetric, topConnected);

				if(x2Cnt.find(encoded) != x2Cnt.end()) {
						cnt += x2Cnt[encoded];
						cntSymmetric += x2Sym[encoded];
						cntSkip++;
						if((cntSkip-1)%100000000 == 100000000-1)
								std::cout << " " << (cntSkip/1000000) << " million. " << (countSmaller*100.0/preCount) << "% read." << std::endl;
						else if((cntSkip-1)%10000000 == 10000000-1)
								std::cout << ":" << std::flush;
						else if((cntSkip-1)%1000000 == 1000000-1)
								std::cout << "." << std::flush;				
				}
				else {
						std::set<Combination<Z> > seen;
						uint64_t _cnt = 0, _cntSymmetric = 0;
						countLayer0Placements<Z,2,layer0Size>(smaller, _cnt, _cntSymmetric, divisor, topSymmetric, topConnected, seen);

						x2Cnt[encoded] = _cnt;
						x2Sym[encoded] = _cntSymmetric;

						cnt += _cnt;
						cntSymmetric += _cntSymmetric;

						if(x2Cnt.size()%1000 == 0)
								std::cout << "| " << x2Cnt.size() << " |" << std::flush;

				    #ifdef TRACE
						std::cout << seen.size() << " models created on " << smaller << std::endl;
  	  			#endif
				}
		}
		std::cout << " Read " << countSmaller << " smaller combinations." << std::endl;
		std::cout << " Counted " << cnt << std::endl;
		std::cout << " Skipped " << cntSkip << " (" << (cntSkip*100.0)/countSmaller << "%)" << std::endl;
		std::cout << " Skip map size " << x2Cnt.size() << std::endl;		

		std::cout << cnt / divisor << " (" << cntSymmetric / divisor << ")" << std::endl;
		assert(cnt % divisor == 0);
		assert(cntSymmetric % divisor == 0);
}

/*
	Special case: First layer has at least Y bricks, while second layer has exactly Y.
	Iterate over smaller Z-layer0Size+Y-1 size models and add all remaining

	Use connection colouring of bricks in second layer
 */
template <int Z, int Y, int layer0Size>
void countXY(char* input) {
		std::cout << "THIS CODE IS NOT DONE YET!" << std::endl;
		int smallerToken = Y-1; // Y-1 bricks required to ensure all combinations in second layer are represented.
		int smallerLayerSizes[MAX_HEIGHT];
		smallerLayerSizes[0] = Y-1;
		char c;
		for(int i = 1; (c = input[i]); i++) {
				smallerToken = smallerToken * 10 + (c-'0');
				smallerLayerSizes[i] = (c-'0');
		}
		std::cout << "Constructing to " << layer0Size << " bricks for first layer of <" << smallerToken << ">" << std::endl;

		// Go through all smaller combinations:
		uint64_t cnt = 0, cntSymmetric = 0, divisor = 5*6*7*8*9, countSmaller = 0, cntSkip = 0, preCount = 0;

		Combination<Z-layer0Size+Y-1> smaller;
		{
				CombinationReader<Z-layer0Size+Y-1> preReader(smallerLayerSizes);
				while(preReader.readCombination(smaller)) {
						preCount++;
						if((preCount-1)%1000000000 == 1000000000-1)
								std::cout << " " << preCount/1000000000 << " billion" << std::endl;
						else if((preCount-1)%100000000 == 100000000-1)
								std::cout << ":" << std::flush;	
						else if((preCount-1)%10000000 == 10000000-1)
								std::cout << "." << std::flush;	
				}
				// In own space to ensure proper closing of file before next read.
		}
		std::cout << "Number of smaller configuration to build on: " << preCount << std::endl;
		
		std::map<Combination<Z>,uint64_t> xyCnt, xySym; // Memory of results // TODO: Should be CutCombination
		
		CombinationReader<Z-layer0Size+Y-1> reader(smallerLayerSizes);
		while(reader.readCombination(smaller)) {
				countSmaller++;
				
				// Find all locations to place a brick on first layer:
				bool topSymmetric, topConnected;
				uint64_t encoded = smaller.encodeForYY(topSymmetric, topConnected);

				if(xyCnt.find(encoded) != xyCnt.end()) {
						cnt += xyCnt[encoded];
						cntSymmetric += xySym[encoded];
						cntSkip++;
						if((cntSkip-1)%100000000 == 100000000-1)
								std::cout << " " << (cntSkip/1000000) << " million. " << (countSmaller*100.0/preCount) << "% read." << std::endl;
						else if((cntSkip-1)%10000000 == 10000000-1)
								std::cout << ":" << std::flush;
						else if((cntSkip-1)%1000000 == 1000000-1)
								std::cout << "." << std::flush;				
				}
				else {
						std::set<Combination<Z> > seen;
						uint64_t _cnt = 0, _cntSymmetric = 0;
						countLayer0Placements<Z,Y,layer0Size>(smaller, _cnt, _cntSymmetric, divisor, topSymmetric, topConnected, seen);

						xyCnt[encoded] = _cnt;
						xySym[encoded] = _cntSymmetric;

						cnt += _cnt;
						cntSymmetric += _cntSymmetric;

						if(xyCnt.size()%1000 == 0)
								std::cout << "| " << xyCnt.size() << " |" << std::flush;

				    #ifdef TRACE
						std::cout << seen.size() << " models created on " << smaller << std::endl;
  	  			#endif
				}
		}
		std::cout << " Read " << countSmaller << " smaller combinations." << std::endl;
		std::cout << " Counted " << cnt << std::endl;
		std::cout << " Skipped " << cntSkip << " (" << (cntSkip*100.0)/countSmaller << "%)" << std::endl;
		std::cout << " Skip map size " << xyCnt.size() << std::endl;		

		std::cout << cnt / divisor << " (" << cntSymmetric / divisor << ")" << std::endl;
		assert(cnt % divisor == 0);
		assert(cntSymmetric % divisor == 0);
}

template <int Z>
void countRefinements(int token, int height, char* input, bool saveOutput) {
		// Special case handling:
		if(height >= 2 && input[0] > '1' && input[1] == '2') {
				std::cout << "Special case <X2...>" << std::endl;
				const int layer0Size = input[0]-'0';
				switch(layer0Size) {
				case 2:
						countX2<Z,2>(input);
						break;
				case 3:
						countX2<Z,3>(input);
						break;
				case 4:
						countX2<Z,4>(input);
						break;
				case 5:
						countX2<Z,5>(input);
						break;
				case 6:
						countX2<Z,6>(input);
						break;
				case 7:
						countX2<Z,7>(input);
						break;
				case 8:
						countX2<Z,8>(input);
						break;
				case 9:
						countX2<Z,9>(input);
						break;
				default:
						std::cerr << "Unexpected size of first layer: " << layer0Size << std::endl;
						return;
				}
		}
		else { // Normal case:
				int reverseToken = 0;
				char c;
				for(int i = height-1; i >= 0; i--) {
						c = input[i];
						reverseToken = reverseToken * 10 + (c-'0');
				}

				if(token < reverseToken) {
						token = reverseToken;
				}
				handleCombinationWriters<Z>(token, 0, false, saveOutput);				
		}
}

template <int Z>
void build_all_combinations(bool saveOutput) {
		std::chrono::time_point<std::chrono::steady_clock> t_init = std::chrono::steady_clock::now();
		added = 0; symmetries = 0; earlyExits = 0;
		if(Z == 1) {
				return; // Trivial case with 1 brick.
		}
	
		std::cout << "Building all combinations of size " << Z << std::endl;
		handleCombinationWriters<Z>(0, Z, false, saveOutput);

		std::cout << "Constructed " << added << " combinations of size " << Z << ", " << symmetries << " of those being symmetric in ";
		std::chrono::time_point<std::chrono::steady_clock> t_end = std::chrono::steady_clock::now();
		std::chrono::duration<double> t = t_end - t_init;
		std::cout << t.count() << "s." << std::endl;
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
