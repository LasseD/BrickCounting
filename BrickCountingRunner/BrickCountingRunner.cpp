#include "stdafx.h" // Windows only requirement.

#include <iostream>
#include <fstream>
#include <assert.h>
#include <algorithm>
#include <functional>

#include "../BrickCountingCore/modelling/RectilinearBrick.h"
#include "../BrickCountingCore/modelling/Block.hpp"
#include "../BrickCountingCore/counting/BlockManager.h"
#include "../BrickCountingCore/counting/ModelManager.h"

void ensureFoldersAreCreated() {
	std::cout << "Ensuring the necessary folders exist." << std::endl;
	std::cout << "Please ensure this program has permissions to create folders and write files here." << std::endl;

	CreateDirectory("blocks", NULL);
	CreateDirectory("blocks\\6", NULL);
	CreateDirectory("homotopies", NULL);
	CreateDirectory("homotopies\\4", NULL);
	CreateDirectory("homotopies\\5", NULL);
	CreateDirectory("homotopies\\6", NULL);
	CreateDirectory("manual", NULL);
	CreateDirectory("manual\\4", NULL);
	CreateDirectory("manual\\5", NULL);
	CreateDirectory("manual\\6", NULL);
	CreateDirectory("old_rc", NULL);

	std::cout << "Folder structure OK." << std::endl;
}

bool fileExist(char const * const fileName) {
	std::ifstream file(fileName);
	return file.good();
}

bool blockFilesExist(int maxBlockSize) {
	if (!fileExist("blocks\\1.dat"))
		return false;
	if (maxBlockSize <= 1)
		return true;
	if (!fileExist("blocks\\2.dat"))
		return false;
	if (maxBlockSize <= 2)
		return true;
	if (!fileExist("blocks\\3.dat"))
		return false;
	if (maxBlockSize <= 3)
		return true;
	if (!fileExist("blocks\\4.dat"))
		return false;
	if (maxBlockSize <= 4)
		return true;
	if (!fileExist("blocks\\5.dat"))
		return false;
	return true;
}

void printUsage() {
	std::cout << "Usage:" << std::endl;
	std::cout << " Computes all NR-models of a given type or size." << std::endl;
	std::cout << " Specify the model type using input paramenter." << std::endl;
	std::cout << " Examples:" << std::endl;
	std::cout << "  Model type 3/1: BrickCounting.exe 3 1" << std::endl;
	std::cout << "  Model type 5/1: BrickCounting.exe 5 1" << std::endl;
	std::cout << "  Model type 2/2/1: BrickCounting.exe 2 2 1" << std::endl;
	std::cout << " Alternatively. Specify the model size using a single input parameter. All model types with the given size will be computed." << std::endl;
	std::cout << " Examples:" << std::endl;
	std::cout << "  Model size 3: BrickCounting.exe 3" << std::endl;
	std::cout << "  Model size 4: BrickCounting.exe 4" << std::endl;
	std::cout << " Limitations:" << std::endl;
	std::cout << "  Max model size is 6." << std::endl;
	std::cout << "  Model types with 3 or more blocks might use too much memory (and time)." << std::endl;
	std::cout << " Special Use:" << std::endl;
	std::cout << "  Compute all Rectilinear models: BrickCounting.exe R" << std::endl;
	std::cout << "  Compute a lower bound for the number of models for a given size (such as size 4): BrickCounting.exe X 4" << std::endl;
}

/*
* MAIN ENTRY POINT!
*/
int main(int numArgs, char** argV) {
#ifdef _DEBUG
	std::cout << "DEBUG MODE" << std::endl;
#endif
	if (numArgs <= 1) {
		printUsage();
		//ModelManager mgr(3, false); mgr.test();

		return 0;
	}
	ensureFoldersAreCreated();

	if (numArgs == 2 || (numArgs == 3 && argV[1][0] == 'X')) {
		if (argV[1][0] == 'R') {
			counting::RectilinearModelManager blockMgr;
			blockMgr.createOld();
			return 0;
		}

		int blockSize = argV[numArgs - 1][0] - '0';

		if (!blockFilesExist(blockSize - 1)) { // Create Block data files:
			counting::RectilinearModelManager mgr;
			mgr.create(blockSize - 1);
		}

		if (blockSize < 3 || blockSize > 6) {
			printUsage();
			return 0;
		}

#ifdef _COMPARE_ALGORITHMS
		ModelManager mgr(blockSize, numArgs == 3);
#else
		counting::ModelManager mgr(blockSize - 1, numArgs == 3);
#endif
		mgr.runForSize(blockSize);
		return 0;
	}

	bool extreme = numArgs >= 2 && argV[1][0] == 'X';

	int combinedSize = 0;
	int maxBlockSize = 1;
	util::TinyVector<int, 6> combinationType;
	for (int i = 1 + (extreme ? 1 : 0); i < numArgs; ++i) {
		int blockSize = argV[i][0] - '0';
#ifdef _DEBUG
		std::cout << " Block SIZE: " << blockSize << std::endl;
#endif
		combinedSize += blockSize;
		combinationType.push_back(blockSize);
		if (blockSize > maxBlockSize)
			maxBlockSize = blockSize;
	}
	if (combinedSize < 4 || combinedSize > 6) {
		printUsage();
		return 0;
	}

	if (!blockFilesExist(maxBlockSize)) { // Create Block data files:
		counting::RectilinearModelManager mgr;
		mgr.create(maxBlockSize);
	}

	std::sort(combinationType.begin(), combinationType.end(), std::greater<int>());
#ifdef _COMPARE_ALGORITHMS
	ModelManager mgr(combinedSize, extreme);
#else
	counting::ModelManager mgr(maxBlockSize, extreme);
#endif
	mgr.runForCombinationType(combinationType, combinedSize);
	return 0;
}
