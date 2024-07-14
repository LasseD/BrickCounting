// -*- mode: c++; tab-width: 2; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=2 sts=2 sw=2 noet :

#ifndef BRICKS_RECTILINEAR_H
#define BRICKS_RECTILINEAR_H

#define ABS(a) ((a) < 0 ? -(a) : (a))
#define DIFF(a,b) ((a) < (b) ? (b)-(a) : (a)-(b))

#include "stdint.h"
#include <stdarg.h>
#include <iostream>
#include <fstream>
#include <assert.h>
#include <sstream>
#include <chrono>
#include <set>
#include <algorithm>

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

		bool is_vertical:1;
		// Could use :6 when 6 bricks is maximum size, but it would not provide practical benefits.
		int8_t x:7, y:7;
};

const Brick FirstBrick = Brick();

/**
 * LayerBrick: A Brick at a given layer.
 */
struct LayerBrick {
		LayerBrick();
		LayerBrick(Brick b, int l);
		LayerBrick(const LayerBrick &b);	

		bool operator <(const LayerBrick& b) const;
		bool operator ==(const LayerBrick& b) const;
		friend std::ostream& operator <<(std::ostream &os,const LayerBrick &b);
		int cmp(const LayerBrick& b) const;

		Brick brick;
		uint8_t layer:3;
};

template <int Z> // Combination with Z bricks
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
				normalize();
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
						assert(bricks[i].layer == b.bricks[i].layer);
						int res = bricks[i].cmp(b.bricks[i]);
						if(res != 0)
								return res < 0;
				}		    
				return false;
		}
		bool operator ==(const Combination& b) const {
				for(int i = 0; i < Z; i++) {
						assert(bricks[i].layer == b.bricks[i].layer);
						int res = bricks[i].cmp(b.bricks[i]);
						if(res != 0)
								return false;
				}		    
				return true;
		}

		bool isValid(const std::string s) const {
				if(bricks[0].brick != FirstBrick) {
						std::cerr << "Error: First brick not |0,0|: " << *this << " at " << s << std::endl;
						return false;
				}
				if(bricks[0].layer != 0) {
						std::cerr << "Error: First brick not at layer 0: " << *this << " at " << s << std::endl;
						return false;
				}
				// Check ordering of bricks:
				for(int i = 1; i < Z; i++) {
						if(bricks[i] < bricks[i-1] || bricks[i-1] == bricks[i]) {
								std::cerr << "Error: Invalid < combination: " << *this << " at " << s << " for bricks " << (i-1) << " and " << i << std::endl;
								return false;
						}
				}
				// Check for overlaps:
				for(int i = 0; i < Z; i++) {
						const LayerBrick &bi = bricks[i];
						for(int j = i+1; j < Z; j++) {
								const LayerBrick &bj = bricks[j];
								if(bi.layer == bj.layer && bi.brick.intersects(bj.brick)) {
										std::cout << "Bricks " << i << " and " << j << " intersect in " << *this << std::endl;
										return false;
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

				for(int i = 0; i < Z; i++) {
						Brick &b = bricks[i].brick;
						if(b.is_vertical) {
								if(bricks[i].layer > 0) {
										break;
								}
								// Vertical bricks in layer 0 can be 'min':
								if(b.x < minx || (b.x == minx && b.y < miny)) {
										minx = b.x;
										miny = b.y;
								}
						}
				}
				// Move all to new min:
				for(int i = 0; i < Z; i++) {
						bricks[i].brick.x -= minx;
						bricks[i].brick.y -= miny;
				}
		}
		void rotate90() {
				for(int i = 0; i < Z; i++) {
						Brick &b = bricks[i].brick;
						int8_t tmp = b.x; // std::swap does not work on this type!
						b.x = b.y;
						b.y = -tmp;
						b.is_vertical = !b.is_vertical;
				}
				translateMinToOrigo();
				std::sort(bricks, &bricks[Z]);
				assert(isValid("rotate90"));
		}

		void rotate180() {
				int8_t minx = bricks[0].brick.x, miny = bricks[0].brick.y;

				for(int i = 1; i < Z; i++) { // 1 because the first brick stays.
						Brick &b = bricks[i].brick;
						if(b.is_vertical) {
								b.x = -b.x;			
								b.y = -b.y;
								if((int)bricks[i].layer == 0 && 
									 (b.x < minx || (b.x == minx && b.y < miny))) {
										minx = b.x;
										miny = b.y;
								}
						}
						else {
								b.x = -b.x;			
								b.y = -b.y;
						}
				}
				if(minx < 0 || (minx == 0 && miny < 0)) { // move all:
						for(int i = 0; i < Z; i++) {
								bricks[i].brick.x -= minx;
								bricks[i].brick.y -= miny;
						}
				}
				std::sort(bricks, &bricks[Z]);
				assert(isValid("rotate180"));
		}

		bool is180Symmetric() const {
				Combination<Z> c(*this);
				c.rotate180();
				return *this == c;
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

		/**
		 * Copy this combination and add b at layer.
		 */
		template <int W>
		bool addBrick(const Brick &b, const uint8_t layer, Combination<W> &out, int &rotated) const {
				assert(W == Z+1);
				for(int i = 0; i < Z; i++) {
						const LayerBrick &b2 = bricks[i];
						if(layer+1 < b2.layer)
								break; // no more to check.
						if(layer != b2.layer) 
								continue; // Skip to correct layer
						if(b2.brick.intersects(b))
								return false;
				}

				// Abb by finding first brick not smaller than bl (simple merge):
				int in = 0;
				const LayerBrick lb(b, layer);
				for(int i = 0; i < Z; i++) {
						if(in == 0) {
								const LayerBrick &lb2 = bricks[i];
								if(lb < lb2) {
										in = 1;
										out.bricks[i] = lb;
								}
						}
						out.bricks[i+in] = bricks[i];
				}
				if(in == 0)
						out.bricks[Z] = lb;

				out.normalize(rotated);
				return true;
		}

		template <int W>
		bool removeBrickAt(int idx, Combination<W> &out) const {
				assert(W == Z-1);
				// Remove brick at index idx:
				int outIdx = 0;
				for(int i = 0; i < Z; i++) {
						if(idx == i)
								continue;
						out.bricks[outIdx++] = bricks[i];
				}
				// Move layers down if lower single bricks was deleted:
				if(out.bricks[0].layer > 0) {
						for(int i = 0; i < W; i++) {
								out.bricks[i].layer--;
						}
				}

				// Check that combination is still connected:
				if(!out.isConnected()) {
						return false;
				}
		
				out.normalize();

				return true; // All OK.
		}
		
		void copy(const Combination<Z> &b) {
				for(int i = 0; i < Z; i++) {
						bricks[i] = b.bricks[i];
				}
		}
		void normalize(int &rotated) {
				// Ensure FirstBrick is first and all is sorted:
				bool hasVerticalLayer0Brick = false;
				for(int i = 0; i < Z; i++) {
						LayerBrick &lb = bricks[i];
						if(lb.layer > 0)
								break;
						if(lb.brick.is_vertical) {
								hasVerticalLayer0Brick = true;
								break;
						}
				}

				// Check if first brick is horizontal at 0,0:
				if(hasVerticalLayer0Brick) {
						translateMinToOrigo();
						std::sort(bricks, &bricks[Z]);
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
		bool isConnected() const {
				bool a[11];
				a[0] = true; // Start from first brick
				for(int i = 1; i < Z; i++) {
						a[i] = false;
				}

				// Greedily add to a:
				int cnt = 1; // Count of connected bricks
				bool anyAdded = true;
				while(anyAdded) {
						anyAdded = false;
						for(int i = 0; i < Z; i++) { // For all bricks:
								if(!a[i]) {
										continue; // Not yet connected.
								}
								const LayerBrick &bi = bricks[i];
								for(int j = 0; j < Z; j++) {
										if(a[j]) // j already included (includes i == j):
												continue;
										const LayerBrick &bj = bricks[j];
										if(ABS(bi.layer - bj.layer) == 1 && bi.brick.intersects(bj.brick)) {
												a[j] = true;
												anyAdded = true;
												cnt++;
										}
								}
						}
				}

				return cnt == Z;
		}

		void flip(int height) {
				for(int i = 0; i < Z; i++) {
						bricks[i].layer = height-1-bricks[i].layer;						
				}
				std::sort(bricks, &bricks[Z]);
				normalize();
		}

		uint8_t differentBrick(const Combination<Z> &c) const {
				uint8_t ret = 0;
				for(int i = 1; i < Z; i++) {
						if(bricks[i].brick != c.bricks[i].brick) {
								if(ret == 0) {
										ret = (uint8_t)i;										
								}
								else {
										return 0; // More than one bricks difference
								}
						}
				}
				return ret;
		}
};

template <int Z>
class CombinationReader {
private:
		std::ifstream *istream;
		int layerSizes[11], height;
		uint8_t bits, bitIdx;
		Combination<Z> lastCombination;
		bool reverse, done;
		uint64_t combinationCounter;
		std::string name;

public:
		CombinationReader(int layerSizes[]) {
				combinationCounter = 0;
				done = false;
				height = 0;
				bits = 0;
				bitIdx = 8; // Ensure a byte is read next time

				// TODO: Minimize layerSizes and flip if upside down!
				std::stringstream ss, ss2;
				ss << Z << "/";

				int layer_size = 0;
				int size_total = 0;
				for(int i = 0; size_total < Z; i++) {
						layer_size = layerSizes[i];
						this->layerSizes[i] = layer_size;
						ss2 << layer_size;
						size_total += layer_size;
						height++;
				}
				assert(Z == size_total);

				std::string layerString = ss2.str(), layerStringReverse = layerString;
				std::reverse(layerStringReverse.begin(), layerStringReverse.end());				
				reverse = layerString < layerStringReverse;

				name = (reverse ? layerStringReverse : layerString);
				ss << name;
				std::string file_name = ss.str();
	
				if(Z != 1) {
						istream = new std::ifstream(file_name.c_str(), std::ios::binary);
				}

				if(reverse) {
						std::reverse(this->layerSizes, &this->layerSizes[height]);
				}

				// Set lastCombination:
				lastCombination.bricks[0].brick = FirstBrick;
				int layer = 0;
				for(int i = 0, j = 0; i < Z; i++, j++) {
						if(j == this->layerSizes[layer]) {
								j = 0;
								layer++;
						}
						lastCombination.bricks[i].layer = layer;
				}
				std::cout << "  READER for " << layerString << ": " << file_name << ", reversed: " << reverse;
				#ifdef DEBUG
				std::cout << ", input layers: " << lastCombination << std::endl;
				#endif
		}

		~CombinationReader() {
				if(Z != 1) {
						istream->close();
						delete istream;
				}
		}

		bool readBit() {
				if(bitIdx == 8) {
						istream->read((char*)&bits, 1);
            #ifdef DEBUG
						std::cout << "   Reading byte " << (int)bits << std::endl;
            #endif
						bitIdx = 0;
				}
				bool bit = (bits >> (7-bitIdx)) & 1;
				bitIdx++;
        #ifdef DEBUG
				std::cout << "   readBit " << (int)bit << std::endl;
        #endif
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
        #ifdef DEBUG
				std::cout << "  readInt8 " << (int)ret << std::endl;
        #endif
				return ret;
		}

		uint8_t readUInt4() {
				uint8_t ret = 0;
				for(int i = 0; i < 4; i++) {
						ret = ret | ((int)readBit() << i);
				}
        #ifdef DEBUG
				std::cout << "  readUInt4 " << (int)ret << std::endl;
        #endif
				return ret;
		}
		
		void readBrick(Brick &b) {
				b.is_vertical = readBit();
				b.x = readInt8();
				b.y = readInt8();
        #ifdef DEBUG
				std::cout << "  readBrick " << b << std::endl;
        #endif
		}

		bool readCombination(Combination<Z> &c) {
				if(done) {
						return false;
				}
				if(Z == 1) {
						c.bricks[0].layer = 0;
						c.bricks[0].brick = FirstBrick;
						done = true;
						return true;
				}

				uint8_t brickI = readUInt4();
				if(brickI == 15) {
						done = true;
						return false;
				}

				if(brickI == 0) { // Read full combination:
						c.bricks[0] = LayerBrick(FirstBrick, 0);
						uint8_t layer = 0;
						int j = 1;
						for(int i = 1; i < Z; i++) { // Read size-1 bricks:
								if(j == layerSizes[(int)layer]) {
										j = 0;
										layer++;
								}
								c.bricks[i].layer = layer;
								readBrick(c.bricks[i].brick);
								j++;
						}
						lastCombination = c;
						assert(c.isValid("Full"));
				}
				else { // Read just one brick:
						c = lastCombination;
						readBrick(c.bricks[brickI].brick);
						assert(c.isValid("Compressed"));
				}

				if(reverse) {
						#ifdef DEBUG
						std::cout << "   Flipping " << c << " of height " << height << std::endl;
            #endif
						c.flip(height);
						assert(c.isValid("Flipped"));
				}

				#ifdef DEBUG
				std::cout << "  Reading combination " << combinationCounter++ << " of type " << name << ": " << c << std::endl;
				#endif
				return true;
		}
};

template <int Z>
class CombinationWriter {
		std::ofstream *ostream;
		int token; // eg. 211 for 4 bricks in config 2-1-1
		std::set<Combination<Z> > combinations;
		uint8_t bits, cntBits;
		Combination<Z> lastWritten;
		uint64_t writtenFull, writtenShort;

public:
		CombinationWriter(int token, bool saveOutput) : token(token) {
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
        #ifdef DEBUG
				//std::cout << "   WriteBit " << (int)bit << std::endl;
        #endif
				bits = (bits << 1) + (bit ? 1 : 0);
				cntBits++;
				if(cntBits == 8) {
            #ifdef DEBUG
				    std::cout << "   Writing byte '" << (int)bits << "'" << std::endl;
            #endif
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
        #ifdef DEBUG
				std::cout << "   WriteInt8 " << (int)toWrite << std::endl;
        #endif
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
				int i = toWrite;
				for(int j = 0; j < 4; j++) {
						writeBit(i & 1);
						i >>= 1;
				}
		}

		void writeBrick(const Brick &b) {
        #ifdef DEBUG
				std::cout << "   WriteBrick " << b << std::endl;
        #endif
				writeBit(b.is_vertical);
				writeInt8(b.x);
				writeInt8(b.y);
		}

		void writeCombination(const Combination<Z> &c) {
				if(ostream == NULL) {
						return;
				}
				uint8_t brickI = lastWritten.differentBrick(c);
				writeUInt4(brickI);
				if(brickI == 0) {
						for(int i = 1; i < Z; i++) { // Skip first brick
								writeBrick(c.bricks[i].brick);
						}
						lastWritten = c;
						writtenFull++;
				}
				else {
						writeBrick(c.bricks[brickI].brick);
						writtenShort++;
				}
		}

		template <int W>
		void add(Combination<W> &cOld, Brick &addedBrick, int layer, uint64_t &added, uint64_t &symmetries) {
				#ifdef DEBUG
				std::cout << " Try to add " << addedBrick << " at layer " << layer << std::endl;
        #endif
				assert(W == Z-1);
				Combination<Z> c;
				int rotated = 0;
				if(!cOld.addBrick(addedBrick, layer, c, rotated)) {
  				  #ifdef DEBUG
				    std::cout << " Does not fit!" << layer << std::endl;
            #endif
						return; // Does not fit.
				}

				// If cOld is symmetric, then check if we are first:
				if(cOld.is180Symmetric() && rotated > 90) {
  				  #ifdef DEBUG
				    std::cout << " Not first version of symmetric combination!" << std::endl;
            #endif
						return; // Not first version of this symmetric combination.
				}
				
				// Compute layer sizes: TODO: Move this to combination class and improve performance of various functions!
				int layer_sizes[11];
				for(int i = 0; i < Z; i++) {
						layer_sizes[i] = 0;
				}
				for(int i = 0; i < Z; i++) {
						layer_sizes[c.bricks[i].layer]++;
				}

				// Try to remove bricks b from c up to and including layer of addedBrick (unless layer only has 1 brick)
				// If b is lower layer than addedBrick and combination stays connected, then do not add!
				// If b same layer as addedBrick: Do not add if combination without b < combination without addedBrick
				for(int i = 0; i < Z; i++) {
						int siblingBrickLayer = c.bricks[i].layer;
						if(siblingBrickLayer < layer) {
								Combination<W> ignore;
								if(layer_sizes[siblingBrickLayer] > 1 && c.removeBrickAt(i, ignore)) {
										#ifdef DEBUG
										std::cout << "  Could be constructed from lower refinement " << ignore << std::endl;
										#endif
										return; // Sibling comes before cOld, so we do not add
								}
						}
						else if(siblingBrickLayer == layer) {
								#ifdef DEBUG
								std::cout << "   same layer!" << std::endl;
								#endif
								Combination<W> sibling;
								if(c.removeBrickAt(i, sibling)) { // Can remove siblingBrick:
										#ifdef DEBUG
										std::cout << "Comparing sibling " << sibling << " with old " << cOld << std::endl;
										#endif
										if(sibling < cOld) {
  	  									#ifdef DEBUG
												std::cout << "  Could be constructed from lesser sibling" << sibling << std::endl;
												#endif
												return; // Lesser sibling!
										}
								}
						}
						else { // siblingBrickLayer > layer:
								break; // Now above layer of brick to be removed.
						}
				}

				// All OK! c can be added and counted:
				#ifdef DEBUG
				std::cout << " ALL OK! Adding " << c << std::endl;
				#endif

				assert(combinations.insert(c).second);
				assert(c.isValid("Counting"));

				// Write to output file:
				writeCombination(c);

				// Update counters:
				added++;
				if(     (added-1)%1000000000 == 1000000000-1)
						std::cout << added << std::endl;
				else if((added-1)%10000000 == 10000000-1)
						std::cout << ":" << std::flush;
				else if((added-1)%1000000 == 1000000-1)
						std::cout << "." << std::flush;
				Combination<Z> rotated180(c);
				rotated180.rotate180();
				if(c == rotated180) {
						symmetries++;
				}
		}

		~CombinationWriter() {
				std::cout << "  Written combinations full: " << writtenFull << ", short: " << writtenShort << " Compression% " << (writtenShort)*100.0/(writtenFull+writtenShort) << std::endl;
				if(ostream != NULL) {
						writeUInt4(15); // Mark end of file.
						flushBits();
						ostream->flush();
						ostream->close();
						delete ostream;
				}
		}

		template <int W>
		void makeNewCombinations(Combination<W> &c, const int layer, uint64_t &added, uint64_t &symmetries, std::set<Combination<Z> > &_old, std::set<Combination<Z> > &_new) {
        #ifdef DEBUG
				std::cout << "-----------------------------" << std::endl;
				std::cout << "Building on " << c << std::endl;
				#endif
				assert(c.isValid("makeNewCombinations"));
				assert(W == Z-1);
				
        // Build new combinations by adding to all existing bricks:
				std::set<Brick> alreadyAdded;
				for(int i = 0; i < W; i++) {
						const LayerBrick &brick = c.bricks[i];
            #ifdef DEBUG
				    std::cout << " Try adding to brick " << brick << std::endl;
            #endif

						if(brick.layer > layer + 1) {
								break; // 2 or more above -> done.
						}
						if(!(brick.layer+1 == layer || brick.layer-1 == layer)) {
								continue; // Connect only to bricks above or below
						}
						bool isVertical = c.bricks[i].brick.is_vertical;

						// Add crossing bricks (one vertical, one horizontal):
						for(int x = -2; x < 3; x++) {
								for(int y = -2; y < 3; y++) {
										Brick b(!isVertical, brick.brick.x+x, brick.brick.y+y);
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
												add(c, b, layer, added, symmetries);
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
										Brick b(isVertical, brick.brick.x+x, brick.brick.y+y);
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
												add(c, b, layer, added, symmetries);
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
        #ifdef DEBUG
				std::cout << "DONE Building on " << c << std::endl;
				std::cout << "-----------------------------" << std::endl;
				#endif
		}

		void fillFromReaders(uint64_t &added, uint64_t &symmetries) {
 		    std::set<Combination<Z> > _old, _new;
        #ifdef DEBUG
				std::cout << "  Filling from readers. added=" << added << ", symmetries=" << symmetries << std::endl;
        #endif
				
				// Build layers:
				int layers = 0, tkn = token;
				int layer_sizes[11];
				for(int i = 0; i < 11; i++) {
						int size_add = tkn % 10;
						if(size_add == 0)
								break;
						layers++;
						layer_sizes[i] = size_add;
						tkn /= 10;
				}
				// Flip the layer sizes:
				for(int i = 0; i < layers/2; i++) {
						std::swap(layer_sizes[i], layer_sizes[layers-i-1]);
				}

				for(int i = 0; i < layers; i++) { // Reader from reader where brick on layer i was added:
						if(layer_sizes[i] == 1 && i != layers-1)
								continue;
						if(layers == 2 && layer_sizes[i-1] == Z-1 && Z != 2)
								continue;

						layer_sizes[i]--;
  	  			#ifdef DEBUG
						std::cout << "Creating reader for writer " << token << " with " << layers << " layers. Reducing layer " << i << std::endl;
						#endif
						CombinationReader<Z-1> reader(layer_sizes);	

						Combination<Z-1> c;
						while(reader.readCombination(c)) {
								makeNewCombinations(c, i, added, symmetries, _old, _new);
						}
  	  			#ifdef DEBUG
						std::cout << "Done handling reader for writer " << token << " with " << layers << " layers. Reducing layer " << i << std::endl;
						#endif
						layer_sizes[i]++;
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
void handleCombinationWriters(int token, int remaining, bool add_self, uint64_t &added, uint64_t &symmetries, bool saveOutput) {
		if(remaining == 0) {
				std::chrono::time_point<std::chrono::steady_clock> t_init = std::chrono::steady_clock::now();
				uint64_t before = added, before_s = symmetries;
				std::cout << " Handling combinations for token " << token << std::endl;

				CombinationWriter<Z> writer(token, saveOutput);
				#ifdef DEBUG
				std::cout << "  Writer constructed" << std::endl;
				#endif
				writer.fillFromReaders(added, symmetries);

				std::cout << "  Constructed " << (added-before) << " combinations, of which " << (symmetries-before_s) << " are symmetric in ";
				std::chrono::time_point<std::chrono::steady_clock> t_end = std::chrono::steady_clock::now();
				std::chrono::duration<double> t = t_end - t_init;
				std::cout << t.count() << "s." << std::endl;
				return;
		}

		//std::cout << " Building recurse writers for token " << token << ", remaining: " << remaining << ", add self?: " << add_self << std::endl;
		for(int i = remaining - (add_self ? 0 : 1); i > 0; i--) {
				int ntoken = 10*token + i;
				handleCombinationWriters<Z>(ntoken, remaining - i, true, added, symmetries, saveOutput);
		}
}

template <int Z>
void build_all_combinations(bool saveOutput) {
		std::chrono::time_point<std::chrono::steady_clock> t_init = std::chrono::steady_clock::now();
		uint64_t added = 0, symmetries = 0;
		if(Z == 1) {
				return; // Trivial case with 1 brick.
		}
	
		std::cout << "Building all combinations of size " << Z << std::endl;
		handleCombinationWriters<Z>(0, Z, false, added, symmetries, saveOutput);

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
