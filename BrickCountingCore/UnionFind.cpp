#include "UnionFind.h"

#include <iostream> // Used to printing information and error messages
#include <assert.h>
#include <time.h>
#include <stack>

namespace UnionFind {
	UnionFindStructure::UnionFindStructure(uint32_t numUnions) : numUnions(numUnions), flattened(false) {
		joins = new std::vector<uint32_t>[numUnions]; // Deleted in flatten()
	}
	UnionFindStructure::~UnionFindStructure() {
		assert(flattened);
		delete[] minInUnions;
	}

	void UnionFindStructure::join(uint32_t a, uint32_t b) {
		assert(!flattened);
		joins[a].push_back(b);
		joins[b].push_back(a);
	}

	void UnionFindStructure::join(const geometry::IntervalList &l1, const geometry::IntervalList &l2, uint32_t union1, uint32_t union2) {
		const geometry::Interval* it1 = l1.begin();
		const geometry::Interval* it2 = l2.begin();

		while (it1 != l1.end() && it2 != l2.end()) {
			if (it2->second < it1->first) {
				++it2;
				++union2;
				continue;
			}
			if (it1->second < it2->first) {
				++it1;
				++union1;
				continue;
			}
			join(union1, union2);
			if (it1->second <= it2->second) {
				++it1;
				++union1;
			}
			else {
				++it2;
				++union2;
			}
		}
	}

	uint32_t UnionFindStructure::getMinInUnion(uint32_t a) const {
		assert(flattened);
		assert(a < numUnions);
		return minInUnions[a];
	}

	void UnionFindStructure::flatten() {
		assert(!flattened);
		bool *handled = new bool[numUnions]; // Deleted further down
		for (uint32_t i = 0; i < numUnions; ++i)
			handled[i] = false;
		minInUnions = new uint32_t[numUnions]; // Deleted in ~UnionFindStructure()

		for (uint32_t i = 0; i < numUnions; ++i) {
			// Handle union with representative "i"
			if (handled[i])
				continue;
			minInUnions[i] = i;
			handled[i] = true;
			roots.push_back(i);

			std::stack<uint32_t> s;
			for (std::vector<uint32_t>::const_iterator it = joins[i].begin(); it != joins[i].end(); ++it) {
				s.push(*it);
				handled[*it] = true;
			}

			while (!s.empty()) {
				uint32_t top = s.top();
				assert(top < numUnions);
				s.pop();
				minInUnions[top] = i;

				for (std::vector<uint32_t>::const_iterator it = joins[top].begin(); it != joins[top].end(); ++it) {
					if (!handled[*it]) {
						s.push(*it);
						handled[*it] = true;
					}
				}
			}
		}

		delete[] handled;
		delete[] joins;
		flattened = true;
	}

	IntervalUnionFind::IntervalUnionFind() : M(*new geometry::IntervalListVector(0, 0)) { // New OK as this will fail
		assert(false); std::cerr << "DEFAULT CONSTRUCTOR FOR IntervalUnionFind SHOULD NEVER BE CALLED" << std::endl;
		int *die = NULL; die[0] = 42;
	}

	IntervalUnionFind& IntervalUnionFind::operator=(const IntervalUnionFind &) {
		return *(new IntervalUnionFind()); // New OK as this will fail
	}

	IntervalUnionFind::IntervalUnionFind(unsigned int numDimensions, unsigned short const * const dimensionSizes, const geometry::IntervalListVector &M) : numStepDimensions(numDimensions - 1), unionI(0), M(M) {
		time_t startTime, endTime;
		time(&startTime);

		// Initialize members:
		for (unsigned int i = 0; i < numDimensions; ++i) {
			this->dimensionSizes[i] = dimensionSizes[i];
		}

		// Initially fill basic vectors:
		buildIntervalIndicatorToUnion();

		// Perform joins:
		ufs = new UnionFindStructure(unionI); // Deleted in ~IntervalUnionFind()
		MixedPosition position;
		buildUnions(0, position);

		// Flatten:
		ufs->flatten();

		time(&endTime);
		double seconds = difftime(endTime, startTime);
		if (seconds > 2)
			std::cout << "Union find performed in " << seconds << " seconds." << std::endl;
	}

	IntervalUnionFind::~IntervalUnionFind() {
		delete ufs;
		delete[] intervalIndicatorToUnion;
		delete[] unionToInterval;
	}

	void IntervalUnionFind::buildIntervalIndicatorToUnion() {
		assert(unionI == 0);
		unionI = 1;
		intervalIndicatorToUnion = new uint32_t[M.sizeIndicator() + 1]; // Deletred in ~IntervalUnionFind()
		unionToInterval = new std::pair<uint32_t, unsigned short>[M.sizeNonEmptyIntervals() + 1]; // Deleted in ~IntervalUnionFind()

		for (uint32_t i = 0; i < M.sizeIndicator(); ++i) {
			unsigned short intervalSize = M.intervalSizeForIndicator(i);
			if (intervalSize == 0) {
				intervalIndicatorToUnion[i] = 0;
				continue;
			}
			intervalIndicatorToUnion[i] = unionI;
			assert(unionI + intervalSize <= M.sizeNonEmptyIntervals() + 1);

			for (unsigned short j = 0; j < intervalSize; ++j) {
				unionToInterval[unionI + j].first = i;
				unionToInterval[unionI + j].second = j;
			}
			unionI += intervalSize;
		}
	}

	void IntervalUnionFind::buildUnions(unsigned int positionI, MixedPosition &position) {
		if (positionI < numStepDimensions) {
			for (unsigned short i = 0; i < dimensionSizes[positionI]; ++i) {
				position.p[positionI] = i;
				buildUnions(positionI + 1, position);
			}
			return;
		}

		uint32_t positionIndex = indicatorIndexOf(position);
		geometry::IntervalList l1;
		M.get(positionIndex, l1);
		if (l1.empty())
			return;
		const uint32_t unionStart1 = intervalIndicatorToUnion[positionIndex];

		// Run and join for each lower dimension:
		for (unsigned int i = 0; i < numStepDimensions; ++i) {
			if (position.p[i] == 0)
				continue; // Can't further decrease in this dimension.
			--position.p[i];
			uint32_t neighbourPositionIndex = indicatorIndexOf(position);
			++position.p[i];

			// Join all in dimension:
			geometry::IntervalList l2;
			M.get(neighbourPositionIndex, l2);
			if (l2.empty())
				continue;
			const uint32_t unionStart2 = intervalIndicatorToUnion[neighbourPositionIndex];

			ufs->join(l1, l2, unionStart1, unionStart2);
		}
	}

	uint32_t IntervalUnionFind::indicatorIndexOf(const MixedPosition &position) const {
		uint32_t index = numStepDimensions == 0 ? 0 : position.p[0];
		for (unsigned int i = 1; i < numStepDimensions; ++i)
			index = (index * dimensionSizes[i]) + position.p[i];

#ifdef _DEBUG
		if (intervalIndicatorToUnion[index] == 0) return index;
		MixedPosition rep;
		getRepresentativeOfUnion(intervalIndicatorToUnion[index], rep);
		for (unsigned int i = 0; i < numStepDimensions; ++i) {
			if (rep.p[i] != position.p[i]) {
				std::cout << "Error in representation juggling! Number stepping angles: " << numStepDimensions << std::endl;
				std::cout << " Failing index: " << i << std::endl;
				std::cout << " Index of indicator: " << index << std::endl;
				std::cout << " Index of union: " << intervalIndicatorToUnion[index] << std::endl;
				std::cout << " Indicator info: Index: " << unionToInterval[intervalIndicatorToUnion[index]].first << ", size: " << unionToInterval[intervalIndicatorToUnion[index]].second << std::endl;
				std::cout << " Number of steps: " << std::endl;
				for (unsigned int j = 0; j < numStepDimensions; ++j)
					std::cout << "  Step angle " << j << ": " << dimensionSizes[j] << std::endl;
				std::cout << " Positions: " << std::endl;
				for (unsigned int j = 0; j < numStepDimensions; ++j)
					std::cout << "  Step angle " << j << ". Input: " << position.p[j] << ", check: " << rep.p[i] << std::endl;
				assert(false); std::cerr << "DIE X018" << std::endl;
				int *die = NULL; die[0] = 42;
			}
			assert(rep.p[i] == position.p[i]);
		}
#endif

		return index;
	}

	uint32_t IntervalUnionFind::getRootForPosition(const MixedPosition rep) const {
		uint32_t indicatorIndex = indicatorIndexOf(rep);
		assert(indicatorIndex < M.sizeIndicator());
		uint32_t firstUnion = intervalIndicatorToUnion[indicatorIndex];

		geometry::IntervalList l;
		M.get(indicatorIndex, l);
		uint32_t index = 0;
		for (const geometry::Interval* it = l.begin(); it != l.end(); ++it, ++index) {
			if (it->first <= rep.lastAngle && rep.lastAngle <= it->second) {
				return ufs->getMinInUnion(firstUnion + index);
			}
		}

		assert(false); std::cerr << "DIE X007" << std::endl;
		int *die = NULL; die[0] = 42;
		return 0;
	}

	void IntervalUnionFind::getRepresentativeOfUnion(unsigned int _unionI, MixedPosition &rep) const {
		assert(_unionI != 0);
		assert(_unionI < M.sizeNonEmptyIntervals() + 1);
		std::pair<uint32_t, unsigned short> intervalInfo = unionToInterval[_unionI];

		// Construct position from interval index:
		uint32_t encodedPosition = intervalInfo.first;
		for (unsigned int i = 0; i < numStepDimensions; ++i) {
			int revDim = numStepDimensions - i - 1;
			rep.p[revDim] = encodedPosition % dimensionSizes[revDim];
			encodedPosition /= dimensionSizes[revDim];
		}

		// Get angle from M:
		geometry::Interval interval = M.get(intervalInfo.first, intervalInfo.second);
		rep.lastAngle = (interval.first + interval.second) / 2;
	}

	std::vector<uint32_t>::const_iterator IntervalUnionFind::rootsBegin() const {
		std::vector<uint32_t>::const_iterator ret = ufs->roots.begin();
		++ret;
		return ret;
	}

	std::vector<uint32_t>::const_iterator IntervalUnionFind::rootsEnd() const {
		return ufs->roots.end();
	}
}
