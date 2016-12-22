#ifndef COUNTING_MODEL_MANAGER_H
#define COUNTING_MODEL_MANAGER_H

#include "../modelling/Block.hpp"
#include "SingleModelManager.h"

using namespace modelling;

namespace counting {
	class ModelManager {
	private:
		FatBlock* blocks[5];
		unsigned long blocksSize[5];

		counter attempts, rectilinear, models, problematic;
		counter angleMappingBoosts[BOOST_STAGES];
		bool findExtremeAnglesOnly;
		util::ProgressWriter pw;

		void runForCombination(const util::TinyVector<FatBlock, 6> &combination, const util::TinyVector<int, 6> &combinationType, int prevBlockIndex, std::ofstream &os);

		void runForCombinationType(const util::TinyVector<int, 6> &combinationType, int remaining, int prevSize, int combinedSize);
	public:
		void runForCombinationType(const util::TinyVector<int, 6> &combinationType, int combinedSize);
#ifdef _COMPARE_ALGORITHMS
		std::set<FatBlock> correct; // For debugging only!
#endif

		ModelManager(int maxBlockSize, bool findExtremeAnglesOnly);
		void runForSize(int size);

		void test();
		void printResults(const util::TinyVector<int, 6> &combinationType, double seconds) const;
	};
}

#endif // COUNTING_MODEL_MANAGER_H
