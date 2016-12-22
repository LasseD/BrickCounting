#include "ModelManager.h"

#include <iostream>
#include <algorithm>
#include <time.h>

#include "BlockManager.h"
#include "SingleModelManager.h"

namespace counting {
	void ModelManager::runForCombination(const util::TinyVector<FatBlock, 6> &combination, const util::TinyVector<int, 6> &combinationType, int prevBlockIndex, std::ofstream &os) {
		if (combination.size() == combinationType.size()) {

			pw.reportProgress();
			SingleModelManager mgr(combination, os, findExtremeAnglesOnly);
			mgr.run();

			attempts += mgr.attempts;
			rectilinear += mgr.rectilinear;
			models += mgr.models;
			problematic += mgr.manual.size();
			for (int i = 0; i < BOOST_STAGES; ++i) {
				angleMappingBoosts[i] += mgr.angleMappingBoosts[i];
			}

#ifdef _DEBUG
			//    mgr.printMPDFile();
#endif

#ifdef _COMPARE_ALGORITHMS
			if (correct.empty()) {
				return;
			}
			std::cout << "Removing " << mgr.foundBlocks.size() << " from " << correct.size() << std::endl;
			for (std::map<FatBlock, uint64_t>::const_iterator it = mgr.foundBlocks.begin(); it != mgr.foundBlocks.end(); ++it) {
				const FatBlock &block = it->first;
				if (correct.find(block) == correct.end()) {
					std::cout << "Incorrectly found: " << block << std::endl;
					MPDPrinter h, d, d2;
					h.add("IncorrectlyFound", new Model(block)); // new OK as we are done.
					h.print("IncorrectlyFound");

					for (std::map<FatBlock, uint64_t>::const_iterator it2 = mgr.foundBlocks.begin(); it2 != mgr.foundBlocks.end(); ++it2) {
						const FatBlock &block2 = it2->first;
						std::stringstream ss;
						ss << "all" << it2->second;
						d.add(ss.str(), new Model(block2)); // new OK as we are done.
					}
					d.print("AllInIncorrectBatch");

					int i = 0;
					for (std::set<FatBlock>::const_iterator it2 = correct.begin(); it2 != correct.end() && i < 9; ++it2, ++i) {
						const FatBlock &block2 = *it2;
						std::stringstream ss;
						ss << "correct_" << i;
						d2.add(ss.str(), new Model(block2)); // new OK as we are done.
					}
					d2.print("FirstCorrect");

					assert(false); std::cerr << "DIE X429" << std::endl;
					int *die = NULL; die[0] = 42;
				}
				correct.erase(block);
			}
#endif

			return;
		}

		int blockSize = combinationType[combination.size()];
		int prevBlockSize = combinationType[combination.size() - 1];
		unsigned int i = prevBlockSize == blockSize ? prevBlockIndex : 0;
		for (; i < blocksSize[blockSize - 1]; ++i) {
			util::TinyVector<FatBlock, 6> v(combination);
			FatBlock &fatBlock = blocks[blockSize - 1][i];
			v.push_back(fatBlock);
			runForCombination(v, combinationType, i, os);
		}
	}

	void ModelManager::runForCombinationType(const util::TinyVector<int, 6> &combinationType, int combinedSize) {
#ifdef _COMPARE_ALGORITHMS
		RectilinearModelManager blockMgr;
		blockMgr.loadFromFile(correct, combinationType);
		std::cout << "RCs to be found: " << correct.size() << std::endl;
#endif

		if (findExtremeAnglesOnly && combinationType.size() <= 2)
			return; // Don't run extreme angles for combinatoins that can be handled better using the normal algorithm.
		time_t startTime, endTime;
		time(&startTime);

		std::ofstream os;
		std::stringstream ss, ss2;

		ss << "manual\\manual_size_" << combinedSize << "_blocksizes";
		ss2 << " Combination type ";

		for (const int* it = combinationType.begin(); it != combinationType.end(); ++it) {
			if (it != combinationType.begin()) {
				ss2 << "/";
			}
			ss2 << *it;
			ss << "_" << *it;
		}
		std::cout << "Computing all models for" << ss2.str();
		ss << ".txt";
		os.open(ss.str().c_str(), std::ios::out);
		if (findExtremeAnglesOnly)
			std::cout << " (extreme angles only)";
		std::cout << "." << std::endl;
		pw.initReportName(ss2.str());

		unsigned long steps = 1;
		for (const int* it = combinationType.begin(); it != combinationType.end(); ++it) {
			steps *= blocksSize[*it - 1];
		}
		pw.initSteps(steps);
		pw.initTime();

		// Store counters:
		counter storeAttempts = attempts;
		counter storeRectilinear = rectilinear;
		counter storeModels = models;
		counter storeProblematic = problematic;
		attempts = rectilinear = models = problematic = 0;

		int firstBlockSize = combinationType[0];
		for (unsigned int i = 0; i < blocksSize[firstBlockSize - 1]; ++i) {
			util::TinyVector<FatBlock, 6> v;
			v.push_back(blocks[firstBlockSize - 1][i]);
			runForCombination(v, combinationType, i, os);
		}

		os.flush();
		os.close();

		time(&endTime);
		double seconds = difftime(endTime, startTime);

		printResults(combinationType, seconds);

		std::cout << std::endl;
		std::cout << "Results for combination type ";
		for (const int* it = combinationType.begin(); it != combinationType.end(); ++it) {
			if (it != combinationType.begin())
				std::cout << "/";
			std::cout << *it;
		}
		std::cout << " (Accumulated totals):" << std::endl;
		std::cout << " Attempts:                                 " << attempts << std::endl;
		std::cout << " Precision boost multiplier:               " << PRECISION_BOOST_MULTIPLIER << std::endl;
		std::cout << " Rectilinear corner connected Blocks:        " << rectilinear << std::endl;
		if (findExtremeAnglesOnly && combinationType.size() > 2) {
			std::cout << " NRCs:                                     " << models << std::endl;
		}
		else {
			std::cout << " Models:                                   " << models << std::endl;
			std::cout << " Models requiring manual confirmation:     " << problematic << std::endl;
		}
		std::cout << " Program execution time (seconds):         " << seconds << std::endl;
#ifdef _COMPARE_ALGORITHMS
		std::cout << " Boosts performed in AngleMappings:" << std::endl;
		for (int i = 0; i < BOOST_STAGES; ++i) {
			std::cout << "  BOOST LEVEL " << (i + 1) << ": " << angleMappingBoosts[i] << std::endl;
		}
		std::cout << " Remaining Rectilinear Blocks to find:       " << correct.size() << std::endl;
		std::cout << std::endl;
		// Print unseen:
		if (!correct.empty()) {
			MPDPrinter d;
			int j = 0;
			for (std::set<FatBlock>::const_iterator it = correct.begin(); it != correct.end(); ++it) {
				std::stringstream ss;
				ss << "missing_" << j++;
				d.add(ss.str(), new Model(*it)); // 'new' is OK as we are done... and don't give a damn anymore.
			}
			d.print("missing");
		}
#endif

		// Restore counters
		attempts += storeAttempts;
		rectilinear += storeRectilinear;
		models += storeModels;
		problematic += storeProblematic;
	}

	void ModelManager::runForCombinationType(const util::TinyVector<int, 6> &combinationType, int remaining, int prevSize, int combinedSize) {
		if (remaining == 0) {
			runForCombinationType(combinationType, combinedSize);
			return;
		}

		for (int i = MIN(prevSize, remaining); i > 0; --i) {
			util::TinyVector<int, 6> v(combinationType);
			v.push_back(i);
			runForCombinationType(v, remaining - i, i, combinedSize);
		}
	}

	void ModelManager::runForSize(int size) {
		time_t startTime, endTime;
		time(&startTime);

		// For base being 1..size-1:
		for (int base = size - 1; base > 0; --base) {
			util::TinyVector<int, 6> combination;
			combination.push_back(base);
			runForCombinationType(combination, size - base, base, size);
		}

		// Output results:
		time(&endTime);
		double seconds = difftime(endTime, startTime);

		util::TinyVector<int, 6> fakeCombination;
		fakeCombination.push_back(size);
		printResults(fakeCombination, seconds);

		std::cout << "Results for size " << size << ":" << std::endl;
		std::cout << " Attempts:                                 " << attempts << std::endl;
		std::cout << " Precision boost multiplier:               " << PRECISION_BOOST_MULTIPLIER << std::endl;
		std::cout << " Rectilinear corner connected Blocks:        " << rectilinear << std::endl;
		if (findExtremeAnglesOnly) {
			std::cout << " NRCs/models:                              " << models << std::endl;
		}
		else {
			std::cout << " Models:                                   " << models << std::endl;
			std::cout << " Models requiring manual confirmation:     " << problematic << std::endl;
		}
		std::cout << " Program execution time (seconds):         " << seconds << std::endl;
#ifdef _DEBUG
		std::cout << " Boosts performed in AngleMappings:" << std::endl;
		for (int i = 0; i < BOOST_STAGES; ++i) {
			std::cout << "  BOOST LEVEL " << (i + 1) << ": " << angleMappingBoosts[i] << std::endl;
		}
#endif
#ifdef _COMPARE_ALGORITHMS
		std::cout << " Remaining Rectilinear Blocks to find:       " << correct.size() << std::endl;
		// Print unseen:
		if (!correct.empty()) {
			MPDPrinter d;
			int j = 0;
			for (std::set<FatBlock>::const_iterator it = correct.begin(); it != correct.end(); ++it) {
				std::stringstream ss;
				ss << "missing_" << j++;
				d.add(ss.str(), new Model(*it)); // 'new' is OK as we are done... and don't give a damn anymore.
			}
			d.print("missing");
		}
#endif
		std::cout << std::endl;
	}

	ModelManager::ModelManager(int maxBlockSize, bool findExtremeAnglesOnly) : attempts(0), rectilinear(0), models(0), problematic(0), findExtremeAnglesOnly(findExtremeAnglesOnly) {
		for (int i = 0; i < BOOST_STAGES; ++i) {
			angleMappingBoosts[i] = 0;
		}
		RectilinearModelManager blockMgr;
		for (int i = 0; i < MIN(maxBlockSize, 5); ++i) {
			blocks[i] = blockMgr.loadFromFile(i, blocksSize[i], false);
		}
	}

	void ModelManager::test() {
		util::TinyVector<int, 6> v;
		v.push_back(2);
		v.push_back(2);
		v.push_back(1);
		//runForCombinationType(v, 6);

		util::TinyVector<FatBlock, 6> v2;
		v2.push_back(blocks[1][0]);
		v2.push_back(blocks[1][0]);
		v2.push_back(blocks[1][2]);

		std::ofstream os;
		os.open("temp.txt", std::ios::out);//

		//runForCombination(v2, v, -1, os);
		// encoding (17307906, 0) std::pair<unsigned __int64,unsigned __int64>

		uint64_t encoded = 17307906;
		IConnectionPairSet list;
		util::TinyVector<IConnectionPair, 5> listx;
		ModelEncoder encoder(v2);
		encoder.decode(encoded, list);

		std::vector<AngledConnection> cs;
		for (std::set<IConnectionPair>::const_iterator it2 = list.begin(); it2 != list.end(); ++it2) {
			cs.push_back(AngledConnection(*it2, StepAngle()));
			listx.push_back(*it2);
			std::cout << *it2 << std::endl;
		}

		std::vector<IConnectionPoint> pool;
		SingleModelManager singleMgr(v2, os, false);
		singleMgr.run(listx, pool, pool, NULL, 0, false);
		singleMgr.printManualLDRFiles();
		singleMgr.printMPDFile();

		/*
		util::TinyVector<IConnectionPair, 5> pairs;
		RectilinearBrick b0;
		RectilinearBrick &b1 = blocks[2][125].otherBricks[0];
		RectilinearBrick &b2 = blocks[2][934].otherBricks[1];

		IConnectionPoint icp1(BrickIdentifier(125,1,0),ConnectionPoint(NE,b1,true,1));
		IConnectionPoint icp2(BrickIdentifier(934,2,1),ConnectionPoint(NW,b2,false,2));

		pairs.push_back(IConnectionPair(icp1,icp2));

		SingleModelManager sm(v2, os, false);
		std::vector<IConnectionPoint> pools;
		sm.run(pairs, pools, pools, NULL, 0);
		sm.printMPDFile();
		sm.printManualLDRFiles();
		*/
	}

	void ModelManager::printResults(const util::TinyVector<int, 6> &combinationType, double seconds) const {
		std::stringstream ssFileName;
		ssFileName << "results";
		for (const int* it = combinationType.begin(); it != combinationType.end(); ++it) {
			ssFileName << "_" << *it;
		}
		ssFileName << ".txt";
		std::ofstream ss;
		ss.open(ssFileName.str().c_str(), std::ios::out);

		ss << "Results for combination type ";
		for (const int* it = combinationType.begin(); it != combinationType.end(); ++it) {
			if (it != combinationType.begin())
				ss << "/";
			ss << *it;
		}
		ss << ":" << std::endl;
		ss << " Attempts:                                 " << attempts << std::endl;
		ss << " Precision boost multiplier:               " << PRECISION_BOOST_MULTIPLIER << std::endl;
		ss << " Rectilinear corner connected Blocks:        " << rectilinear << std::endl;
		if (findExtremeAnglesOnly && combinationType.size() > 2) {
			ss << " NRCs:                                     " << models << std::endl;
		}
		else {
			ss << " Models:                                   " << models << std::endl;
			ss << " Models requiring manual confirmation:     " << problematic << std::endl;
		}
		ss << " Program execution time (seconds):         " << seconds << std::endl;
		ss.flush();
		ss.close();
	}
}