#ifndef MODELLING_BLOCK_HPP
#define MODELLING_BLOCK_HPP

#include <iostream>
#include <fstream>
#include <algorithm>
#include <assert.h> 
#include <set> 
#include <limits.h>
#include <stack>
#include <functional>

#include "../util/LDRPrintable.h"
#include "RectilinearBrick.h"
#include "ConnectionPoint.h"
#include "Brick.h"

#define NO_INDEX ULONG_MAX

namespace modelling {
	template <unsigned int SIZE>
	class RectilinearModel : public util::LDRPrintable {
	public:
		RectilinearBrick otherBricks[SIZE == 1 ? 1 : SIZE - 1]; // first brick 0,0, vertical at lv. 0. Bricks sorted.

		bool verify() const {
			const RectilinearBrick origin;
			for (int i = 0; i < SIZE - 1; ++i) {
				if (otherBricks[i] < origin ||
					otherBricks[i].x < -15 || otherBricks[i].x > 15 || otherBricks[i].y < -15 || otherBricks[i].y > 15) {
					std::cout << "Verification failed for " << *this << ": Following brick is illegal: " << otherBricks[i] << std::endl;
					return false;
				}
				if (origin.intersects(otherBricks[i])) {
					std::cout << "Verification failed for " << *this << ": Origin intersected!" << std::endl;
					return false;
				}
			}
			return true;
		}

		/*
		Return true if changed.
		*/
		bool ensureOriginIsSmallest() {
			int minI = -1;
			RectilinearBrick min; // origin.
			for (int i = 0; i < SIZE - 1; ++i) {
				if (otherBricks[i] < min) {
					minI = i;
					min = otherBricks[i];
				}
			}

			if (minI == -1)
				return false;

			// Move all according to the new origin:
			for (int i = 0; i < SIZE - 1; ++i) {
				otherBricks[i].x -= min.x;
				otherBricks[i].y -= min.y;
			}

			// Re-introduce old origin at the brick now representing the new origin:
			otherBricks[minI].x -= min.x;
			otherBricks[minI].y -= min.y;
			std::sort(otherBricks, &otherBricks[SIZE - 1]);
			return true;
		}

		RectilinearModel() {}

		RectilinearModel(const RectilinearModel<SIZE>& c) {
			for (int i = 0; i < SIZE - 1; ++i) {
				otherBricks[i] = c.otherBricks[i];
			}
		}

		/*
		Constructor for constructing a block from a smaller block c and a brick b.
		Assumptions about the input: b does not intersect c. b and c are strongly connected.
		*/
		RectilinearModel(const RectilinearModel<SIZE - 1> &c, const RectilinearBrick &b) {
			if (SIZE == 2) {
				otherBricks[0] = b;
				return;
			}

			int ithis = 0;
			for (int i = 0; i < SIZE - 2; ++i) {
				if (i == ithis && b < c.otherBricks[i])
					otherBricks[ithis++] = b;
				otherBricks[ithis++] = c.otherBricks[i];
			}
			if (ithis == SIZE - 2)
				otherBricks[SIZE - 2] = b;

			ensureOriginIsSmallest(); // Sorting is ensured above, hence no additional sorting required if origin doesn't change.
			assert(verify());
		}

		/*
		Turns this block 90 degrees
		Might cause reordering.
		Causes new brick to be "origin".
		It is assumed that it is possible to turn 90 degrees.
		1,2 -> 2,-1 -> -1,-2
		*/
		void turn90() {
			// First turn 90:
			for (int i = 0; i < SIZE - 1; ++i) {
				int8_t oldX = otherBricks[i].x;
				otherBricks[i].x = otherBricks[i].y;
				otherBricks[i].y = -oldX;
				otherBricks[i].flipHorizontal();
			}

			// Find the new origin:
			int minI = 0;
			for (int i = 1; i < SIZE - 1; ++i) {
				if (otherBricks[i] < otherBricks[minI])
					minI = i;
			}

			// Move all according to the new origin:
			int8_t moveX = otherBricks[minI].x;
			int8_t moveY = otherBricks[minI].y;
			for (int i = 0; i < SIZE - 1; ++i) {
				otherBricks[i].x -= moveX;
				otherBricks[i].y -= moveY;
			}

			// Re-introduce the turned old origin:
			otherBricks[minI].setHorizontalTrue();
			otherBricks[minI].x -= moveX;
			otherBricks[minI].y -= moveY;

			// Sort bricks:
			std::sort(otherBricks, &otherBricks[SIZE - 1]);
			assert(verify());
		}

		void turn180() {
			// First turn all bricks 180:
			for (int i = 0; i < SIZE - 1; ++i) {
				otherBricks[i].x = -otherBricks[i].x;
				otherBricks[i].y = -otherBricks[i].y;
			}
			if (SIZE == 2)
				return;

			// Find the new origin if necessary. If no change: sort.:
			if (!ensureOriginIsSmallest())
				std::sort(otherBricks, &(otherBricks[SIZE - 1]));

			assert(verify());
		}

		bool canTurn90() const {
			if (SIZE <= 2) {
				return false;
			}

			for (int i = 0; i < SIZE - 1; ++i) {
				if (otherBricks[i].level() > 0)
					return false;
				if (otherBricks[i].horizontal())
					return true;
			}
			return false;
		}

		std::pair<int, int> rotationBrickPosition() const {
			if (SIZE == 1)
				return std::make_pair(0, 0);
			RectilinearBrick furthest;
			for (int i = 0; i < SIZE - 1; ++i) {
				if (otherBricks[i].level() > 0)
					break;
				if (otherBricks[i].horizontal())
					continue; // horizontal first - ignore these!
				furthest = otherBricks[i];
			}
			return std::make_pair(furthest.x, furthest.y);
		}

		bool operator<(const RectilinearModel<SIZE> &c) const {
			for (int i = 0; i < SIZE - 1; ++i) {
				if (otherBricks[i] != c.otherBricks[i])
					return otherBricks[i] < c.otherBricks[i];
			}
			return false;
		}

		bool operator==(const RectilinearModel<SIZE> &c) const {
			for (int i = 0; i < SIZE - 1; ++i) {
				if (otherBricks[i] != c.otherBricks[i])
					return false;
			}
			return true;
		}

		void serialize(std::ofstream &os) const {
			for (int i = 0; i < SIZE - 1; ++i) {
				otherBricks[i].serialize(os);
			}
		}

		void deserialize(std::ifstream &is) {
			for (int i = 0; i < SIZE - 1; ++i)
				otherBricks[i].deserialize(is);
		}

		// O(SIZE) algorithm.
		bool intersects(const RectilinearBrick &brick) const {
			RectilinearBrick b;
			for (int i = 0; i < SIZE; b = otherBricks[i++]) {
				if (b.intersects(brick))
					return true;
			}
			return false;
		}

		void toLDR(std::ofstream &os, int ldrColor) const {
			RectilinearBrick b;
			for (int i = 0; i < SIZE; b = otherBricks[i++]) {
				b.toLDR(os, ldrColor);
			}
		}

		unsigned long layerHash() const {
			// Initialize layers:
			unsigned int hLayerSizes[SIZE] = {};
			unsigned int vLayerSizes[SIZE] = {};

			// count:
			RectilinearBrick b;
			for (int i = 0; i < SIZE; b = otherBricks[i++]) {
				if (b.horizontal())
					++hLayerSizes[b.level()];
				else
					++vLayerSizes[b.level()];
			}

			// encode:
			unsigned long res = 0;
			for (int i = 0; i < SIZE; ++i) {
				res *= 10;
				if (vLayerSizes[i] == hLayerSizes[i]) {
					res += hLayerSizes[i] + vLayerSizes[i] + 5;
				}
				else {
					res += hLayerSizes[i] + vLayerSizes[i];
				}
			}
			return res;
		}

		void getCombinationType(util::TinyVector<int, 6> &l) const {
			assert(l.empty());
			// Create initial blocks:
			bool linked[36];
			for (int i = 0; i < 36; ++i)
				linked[i] = false;

			// Join blocks based on strong connections:
			RectilinearBrick b;
			for (int i = 0; i < SIZE; b = otherBricks[i++]) {
				for (int j = i + 1; j < SIZE; ++j) {
					if (b.isStronglyConnectedWith(otherBricks[j - 1]))
						linked[6 * i + j] = linked[6 * j + i] = true;
				}
			}

			// Traverse blocks:
			bool handled[6] = { false,false,false,false,false,false };
			std::stack<int> stack;
			for (int i = 0; i < SIZE; ++i) {
				if (handled[i])
					continue;
				handled[i] = true;
				int blockSize = 1;
				stack.push(i);
				while (!stack.empty()) {
					int fromStack = stack.top();
					stack.pop();
					for (int j = i + 1; j < SIZE; ++j) {
						if (!handled[j] && linked[6 * fromStack + j]) {
							stack.push(j);
							handled[j] = true;
							++blockSize;
						}
					}
				}
				l.push_back(blockSize);
			}
			std::sort(l.begin(), l.end(), std::greater<int>());
		}
	};

	/*
	template <>
	class RectilinearModel<1> : public LDRPrintable {
	public:
	void serialize(std::ofstream &os) const;
	void deserialize(std::ifstream &is);
	void toLDR(std::ofstream &os, int ldrColor) const;
	};*/

	template <unsigned int SIZE>
	std::ostream& operator<<(std::ostream& os, const RectilinearModel<SIZE>& c)
	{
		for (int i = 0; i < SIZE - 1; ++i)
			os << c.otherBricks[i];
		return os;
	}

	class FatBlock {
	public:
		int size;
		unsigned long index;
		RectilinearBrick otherBricks[5];
		bool isRotationallySymmetric;
		std::pair<int, int> rotationBrickPosition; // if isRotationallySymmetric

		FatBlock() : size(1), index(0), isRotationallySymmetric(true), rotationBrickPosition(std::pair<int, int>(0, 0)) {}
		FatBlock(const FatBlock &f) : size(f.size), index(f.index), isRotationallySymmetric(f.isRotationallySymmetric), rotationBrickPosition(f.rotationBrickPosition) {
			for (int i = 0; i < size - 1; ++i) {
				otherBricks[i] = f.otherBricks[i];
			}
		}

		template <unsigned int SIZE>
		FatBlock(const RectilinearModel<SIZE>& block, unsigned long index) : size(SIZE), index(index) {
			for (int i = 0; i < SIZE - 1; ++i) {
				otherBricks[i] = block.otherBricks[i];
			}
			RectilinearModel<SIZE> turned(block);
			turned.turn180();
			isRotationallySymmetric = (turned == block);
			rotationBrickPosition = block.rotationBrickPosition();
		}

		FatBlock(util::TinyVector<Brick, 6> v) : size((int)v.size()), index(NO_INDEX), isRotationallySymmetric(false), rotationBrickPosition(std::make_pair(0, 0)) {
			// Ensure level starts at 0:
			int8_t minLv = 99;
			for (const Brick* it = v.begin(); it != v.end(); ++it) {
				if (it->level < minLv)
					minLv = it->level;
			}
			for (Brick* it = v.begin(); it != v.end(); ++it) {
				it->level -= minLv; // OK because v is not used by reference.
			}

			// Turn into RectilinearBricks and find min:
			util::TinyVector<RectilinearBrick, 6> rbricks;
			RectilinearBrick min = v.begin()->toRectilinearBrick();
			for (const Brick* it = v.begin(); it != v.end(); ++it) {
				RectilinearBrick rb = it->toRectilinearBrick();
				if (rb < min)
					min = rb;
				rbricks.push_back(rb);
			}

			// Turn 90 degrees if min is horixzontal:
			if (min.horizontal()) {
				for (RectilinearBrick* it = rbricks.begin(); it != rbricks.end(); ++it) {
					int8_t oldX = it->x;
					it->x = it->y;
					it->y = -oldX;
					it->flipHorizontal();
				}

				// Find the origin:
				min = *rbricks.begin();
				for (const RectilinearBrick* it = rbricks.begin(); it != rbricks.end(); ++it) {
					if (*it < min) {
						min = *it;
					}
				}
			}

			// Move all according to the origin:
			int8_t moveX = min.x;
			int8_t moveY = min.y;
			for (RectilinearBrick* it = rbricks.begin(); it != rbricks.end(); ++it) {
				it->x -= moveX;
				it->y -= moveY;
			}

			// Sort and save bricks:
			std::sort(rbricks.begin(), rbricks.end());
			int i = -1;
			for (const RectilinearBrick* it = rbricks.begin(); it != rbricks.end(); ++it, ++i) {
				if (i == -1)
					continue; // don't save origin.
				otherBricks[i] = *it;
			}
		}

		template <unsigned int SIZE>
		RectilinearModel<SIZE> toBlock() const {
			RectilinearModel<SIZE> ret;
			for (int i = 0; i < SIZE - 1; ++i) {
				ret.otherBricks[i] = otherBricks[i];
			}
			return ret;
		}

		bool angleLocked(const ConnectionPoint &p) const {
			RectilinearBrick b;
			for (int i = 0; i < size; b = otherBricks[i++]) {
				if (b.angleLocks(p))
					return true;
			}
			return false;
		}
		bool blocked(const ConnectionPoint &p) const {
			RectilinearBrick b;
			for (int i = 0; i < size; b = otherBricks[i++]) {
				if (b.blocks(p))
					return true;
			}
			return false;
		}

		void getConnectionPoints(std::set<ConnectionPoint> &above, std::set<ConnectionPoint> &below) const {
			// Find bricks that might block every level:
			std::set<RectilinearBrick> blockers[6];
			{ // using block so that b can be re-used below.
				RectilinearBrick b;
				for (int i = 0; i < size; b = otherBricks[i++]) {
					blockers[b.level()].insert(b);
				}
			}

			// Get all possible connection points and add them to sets (unless blocked).
			ConnectionPoint tmp[4];
			RectilinearBrick b;
			for (int i = 0; i < size; b = otherBricks[i++]) {
				b.getConnectionPointsAbove(tmp, i);
				for (int j = 0; j < 4; ++j) {
					if (!blocked(tmp[j])) {
						above.insert(tmp[j]);
					}
				}
				b.getConnectionPointsBelow(tmp, i);
				for (int j = 0; j < 4; ++j) {
					if (!blocked(tmp[j])) {
						below.insert(tmp[j]);
					}
				}
			}
		}

		bool isRotationallyMinimal(const util::TinyVector<ConnectionPoint, 5> &pointsForBlocks) const {
			if (!isRotationallySymmetric)
				return true; // not applicaple.
			util::TinyVector<ConnectionPoint, 5> rotatedPoints;
			for (const ConnectionPoint* it = pointsForBlocks.begin(); it != pointsForBlocks.end(); ++it) {
				ConnectionPoint rotatedPoint(*it, rotationBrickPosition);
				rotatedPoints.push_back(rotatedPoint);
			}
			std::sort(rotatedPoints.begin(), rotatedPoints.end());

			for (const ConnectionPoint *it1 = pointsForBlocks.begin(), *it2 = rotatedPoints.begin(); it1 != pointsForBlocks.end(); ++it1, ++it2) {
				if (*it2 < *it1) {
					return false;
				}
				if (*it1 < *it2) {
					return true;
				}
			}
			return true;
		}

		bool isRotationallyMinimal(const ConnectionPoint &p) const {
			if (!isRotationallySymmetric)
				return true; // not applicaple.
			ConnectionPoint rotatedPoint(p, rotationBrickPosition);
			return p < rotatedPoint;
		}

		RectilinearBrick operator[](int i) const {
			assert(i < size);
			assert(i >= 0);
			if (i == 0) {
				return RectilinearBrick();
			}
			return otherBricks[i - 1];
		}
		bool operator<(const FatBlock &c) const {
			assert((index == NO_INDEX) == (c.index == NO_INDEX));
			if (index == NO_INDEX) { // Proper comparison:
				assert(size == c.size);
				for (int i = 0; i < size - 1; ++i) {
					if (otherBricks[i] != c.otherBricks[i])
						return otherBricks[i] < c.otherBricks[i];
				}
				return false;
			}

			if (size != c.size)
				return size > c.size; // Big first.
			return index < c.index;
		}
		bool operator==(const FatBlock &c) const {
			return size == c.size && index == c.index;
		}

		bool ensureOriginIsSmallest() {
			int minI = -1;
			RectilinearBrick min; // origin.
			for (int i = 0; i < size - 1; ++i) {
				if (otherBricks[i] < min) {
					minI = i;
					min = otherBricks[i];
				}
			}

			if (minI == -1)
				return false;

			// Move all according to the new origin:
			for (int i = 0; i < size - 1; ++i) {
				otherBricks[i].x -= min.x;
				otherBricks[i].y -= min.y;
			}

			// Re-introduce old origin at the brick now representing the new origin (this was centered at 0,0, so can be used for this):
			otherBricks[minI].x = -min.x;
			otherBricks[minI].y = -min.y;
			std::sort(otherBricks, otherBricks + size - 1);
			return true;
		}

		/*
		Turns this block 90 degrees
		Might cause reordering.
		Causes new brick to be "origin".
		It is assumed that it is possible to turn 90 degrees.
		1,2 -> 2,-1 -> -1,-2
		*/
		void turn90() {
			// First turn 90:
			for (int i = 0; i < size - 1; ++i) {
				int8_t oldX = otherBricks[i].x;
				otherBricks[i].x = otherBricks[i].y;
				otherBricks[i].y = -oldX;
				otherBricks[i].flipHorizontal();
			}

			// Find the new origin:
			int minI = 0;
			for (int i = 1; i < size - 1; ++i) {
				if (otherBricks[i] < otherBricks[minI])
					minI = i;
			}

			// Move all according to the new origin:
			int8_t moveX = otherBricks[minI].x;
			int8_t moveY = otherBricks[minI].y;
			for (int i = 0; i < size - 1; ++i) {
				otherBricks[i].x -= moveX;
				otherBricks[i].y -= moveY;
			}

			// Re-introduce the turned old origin:
			otherBricks[minI].setHorizontalTrue();
			otherBricks[minI].x -= moveX;
			otherBricks[minI].y -= moveY;

			// Sort bricks:
			std::sort(otherBricks, otherBricks + size - 1);
		}

		void turn180() {
			// First turn all bricks 180:
			for (int i = 0; i < size - 1; ++i) {
				otherBricks[i].x = -otherBricks[i].x;
				otherBricks[i].y = -otherBricks[i].y;
			}
			if (size <= 2)
				return;

			// Find the new origin if necessary. If no change: sort.:
			if (!ensureOriginIsSmallest())
				std::sort(otherBricks, otherBricks + size - 1);
		}

		bool canTurn90() const {
			if (size <= 2) {
				return false;
			}

			for (int i = 0; i < size - 1; ++i) {
				if (otherBricks[i].level() > 0)
					return false;
				if (otherBricks[i].horizontal())
					return true;
			}
			return false;
		}

		FatBlock rotateToMin() const {
			FatBlock min(*this);
			FatBlock candidate(*this);
			if (canTurn90()) {
				for (int i = 0; i < 3; ++i) {
					candidate.turn90();
					if (candidate < min) {
						min = candidate;
					}
				}
			}
			else {
				candidate.turn180();
				if (candidate < min) {
					min = candidate;
				}
			}
			min.index = NO_INDEX;
			return min;
		}

		int getBrickIndex(const RectilinearBrick &b) const {
			if (b.isBase())
				return 0;
			for (int i = 0; i < size - 1; ++i)
				if (b == otherBricks[i])
					return i + 1;
			assert(false); std::cerr << "DIE X002" << std::endl;
			int *die = NULL; die[0] = 42;
			return -1;
		}

		std::pair<int, int> getRotationBrickPosition() const {
			if (size == 1)
				return std::make_pair(0, 0);
			RectilinearBrick furthest;
			for (int i = 0; i < size - 1; ++i) {
				if (otherBricks[i].level() > 0)
					break;
				if (otherBricks[i].horizontal())
					continue; // horizontal first - ignore these!
				furthest = otherBricks[i];
			}
			return std::make_pair(furthest.x, furthest.y);
		}

		void deserialize(std::ifstream &is) {
			for (int i = 0; i < size - 1; ++i)
				otherBricks[i].deserialize(is);

			FatBlock turned(*this);
			turned.turn180();
			isRotationallySymmetric = (turned == *this);
			rotationBrickPosition = getRotationBrickPosition();
		}
	};

	inline std::ostream& operator<<(std::ostream& os, const FatBlock& c) {
		os << "FatBlock[size=" << c.size;
		if (c.index != NO_INDEX)
			os << ",index=" << c.index;
		if (c.isRotationallySymmetric) {
			os << ",symmetric@" << c.rotationBrickPosition.X << "," << c.rotationBrickPosition.Y;
		}
		for (int i = 0; i < c.size - 1; ++i)
			os << c.otherBricks[i];
		os << "]";
		return os;
	}
}

#endif // MODELLING_BLOCK_HPP
