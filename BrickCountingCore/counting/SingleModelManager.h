#ifndef COUNTING_SINGLE_MODEL_MANAGER_H
#define COUNTING_SINGLE_MODEL_MANAGER_H

#include <vector>
#include <set>
#include <map>

#include "../util/ProgressWriter.hpp"
#include "../modelling/Model.hpp"
#include "../modelling/ConnectionPoint.h"
#include "../modelling/Block.hpp"
#include "../modelling/ModelEncoder.h"
#include "AngleMapping.h"

using namespace modelling;

namespace counting {
	class SingleModelManager {
	public:
		std::set<uint64_t> nonCyclicModels;
		std::set<Encoding> cyclicModels;

#ifdef _COMPARE_ALGORITHMS
		std::map<FatBlock, uint64_t> foundBlocks; // For debugging only!
#endif

		std::vector<util::TinyVector<AngledConnection, 5> > manual;
	private:
		std::vector<Model> nrcToPrint; // Used when there are non-rectilinear models, but not multiple non-rectilinear models for a given connection set.
		std::vector<Model> modelsToPrint; // Used when there are multiple models for a given connection set then include all models in it - including the RC!
		std::set<uint64_t> investigatedConnectionPairListsEncoded;
		unsigned int combinationSize;
		FatBlock combination[6];
		ModelEncoder encoder;
		std::ofstream &os;
		util::ProgressWriter pw;

		// Used only for construction:
		std::set<ConnectionPoint> above[6], below[6];
		bool prevMustBeChosen[6], findExtremeAnglesOnly;

		void run(bool countForPw);
	public: // TODO: Only public for testing.
		void run(util::TinyVector<IConnectionPair, 5> &l, const std::vector<IConnectionPoint> &abovePool, const std::vector<IConnectionPoint> &belowPool, bool *remaining, int remainingSize, bool countForPw);

	public:
		SingleModelManager(const util::TinyVector<FatBlock, 6> &combination, std::ofstream &os, bool findExtremeAnglesOnly);
		SingleModelManager& operator=(const SingleModelManager &tmp) {
			assert(false); // Assignment operator should not be used.
			util::TinyVector<FatBlock, 6> c;
			SingleModelManager *ret = new SingleModelManager(c, tmp.os, false); // Not deleted - fails on invoce.
			return *ret;
		}

		void run();
		void printMPDFile() const;
		void printManualLDRFiles() const;
		void printManualLDRFile(const std::vector<std::pair<std::string, Model> > &v, const std::string &fileName) const;
		bool isRotationallyMinimal(const IConnectionPairSet &l) const;

		counter attempts, models, rectilinear;//, nonRectilinearModels, 
		counter angleMappingBoosts[BOOST_STAGES];
	};
}

#endif // COUNTING_SINGLE_MODEL_MANAGER_H
