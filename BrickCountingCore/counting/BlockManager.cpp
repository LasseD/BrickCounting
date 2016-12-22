#include "BlockManager.h"

#include <sstream>
//#include <windows.h>

namespace counting {
	RectilinearModelManager::RectilinearModelManager() {
		lists = new void*[6]; // Deleted in ~RectilinearModelManager()
		lists[0] = &l1;
		lists[1] = &l2;
		lists[2] = &l3;
		lists[3] = &l4;
		lists[4] = &l5;
		lists[5] = &l6;
	}

	RectilinearModelManager::~RectilinearModelManager() {
		delete[] lists;
	}

	void RectilinearModelManager::create(int maxBlockSize) {
		// 1:
		RectilinearModel<1> baseModel;
		l1.s.insert(baseModel);
		std::cout << l1 << std::endl;
		writeToFile(0);

		l1.printMPDFile();
		if (maxBlockSize <= 1) return;

		// 2:
		l2.addAllFor(baseModel, false);
		//l1.s.clear();
		std::cout << l2 << std::endl;
		writeToFile(1);

		l2.printMPDFile();
		std::cout << "DONE STRONGLY CONNECTED CONFIGURATIONS OF SIZE 2" << std::endl << std::endl;
		if (maxBlockSize <= 2) return;

		// 3:
		l3.addAllFor(l2, false);
		l2.s.clear();
		std::cout << l3 << std::endl;
		writeToFile(2);

		l3.printMPDFile();
		std::cout << "DONE STRONGLY CONNECTED CONFIGURATIONS OF SIZE 3" << std::endl << std::endl;
		if (maxBlockSize <= 3) return;

		// 4:
		l4.addAllFor(l3, false);
		l3.s.clear();
		std::cout << l4 << std::endl;
		writeToFile(3);
		l4.printMPDFile(false, 2048);
		std::cout << "DONE STRONGLY CONNECTED CONFIGURATIONS OF SIZE 4" << std::endl << std::endl;
		if (maxBlockSize <= 4) return;

		// 5:
		l5.addAllFor(l4, false);
		l4.s.clear();
		std::cout << l5 << std::endl;
		writeToFile(4);
		l5.printMPDFile(false, 2048);
		std::cout << "DONE STRONGLY CONNECTED CONFIGURATIONS OF SIZE 5" << std::endl << std::endl;
		if (maxBlockSize <= 5) return;

		// 6:
		std::cout << "NOW STARTING TO COUNT ALL STRONGLY CONNECTED CONFIGURATIONS OF SIZE 6. RESTART TO SKIP!" << std::endl << std::endl;
		l6.countAllFor(l5, false);
		l5.s.clear();
		l6.printMPDFile(false, 2048);
		l6.s.clear();
	}

	void RectilinearModelManager::createOld() {
		// 1:
		RectilinearModel<1> baseModel;
		l1.s.insert(baseModel);
		std::cout << l1 << std::endl;
		writeToFile(0);

		l1.printMPDFile(true);

		// 2:
		l2.addAllFor(baseModel, true);
		l1.s.clear();
		std::cout << l2 << std::endl;
		writeToFile(1, true);

		l2.printMPDFile(true);
		std::cout << "DONE OLD OF SIZE 2" << std::endl << std::endl;

		// 3:
		l3.addAllFor(l2, true);
		std::cout << "Done add all for l3" << std::endl;
		l2.s.clear();
		std::cout << l3 << std::endl;
		writeToFile(2, true);

		l3.printMPDFile(true, 2048);
		std::cout << "DONE OLD OF SIZE 3" << std::endl << std::endl;

		// 4:
		l4.addAllFor(l3, true);
		l3.s.clear();
		std::cout << l4 << std::endl;
		writeToFile(3, true);
		l4.printMPDFile(true, 2048);
		std::cout << "DONE OLD OF SIZE 4" << std::endl << std::endl;

		// 5:
		l5.addAllFor(l4, true);
		l4.s.clear();
		std::cout << l5 << std::endl;
		writeToFile(4, true);
		l5.printMPDFile(true, 2048);
		std::cout << "DONE OLD OF SIZE 5" << std::endl << std::endl;

		// 6:
		std::cout << "NOW STARTING TO COUNT ALL RECTILINEAR CONFIGURATIONS OF SIZE 6. RESTART TO SKIP!" << std::endl << std::endl;
		l6.countAllFor(l5, true);
		l5.s.clear();
		l6.printMPDFile(true, 2048);
		l6.s.clear();
		std::cout << "DONE OLD OF SIZE 6" << std::endl << std::endl;
	}

	void RectilinearModelManager::writeToFile(int i, bool old) {
		std::ofstream os;
		std::stringstream ss;
		if (old)
			ss << "old_rc";
		else
			ss << "block";
		ss << "\\" << (i + 1) << ".dat";
		os.open(ss.str().c_str(), std::ios::binary | std::ios::out);
		std::cout << "Writing file with strongly connected models of size " << (i + 1) << " to " << ss.str() << std::endl;

		switch (i) {
		case 0:
			l1.serialize(os);
			break;
		case 1:
			l2.serialize(os);
			break;
		case 2:
			l3.serialize(os);
			break;
		case 3:
			l4.serialize(os);
			break;
		case 4:
			l5.serialize(os);
			break;
		case 5:
			l6.serialize(os);
			break;
		}
		os.close();
	}

	FatBlock* RectilinearModelManager::loadFromFile(std::string fileName, int i, unsigned long &size) const {
		std::ifstream is;
		is.open(fileName.c_str(), std::ios::binary | std::ios::in);
		FatBlock* ret;
		switch (i) {
		case 0:
			ret = l1.deserialize(is, size);
			break;
		case 1:
			ret = l2.deserialize(is, size);
			break;
		case 2:
			ret = l3.deserialize(is, size);
			break;
		case 3:
			ret = l4.deserialize(is, size);
			break;
		case 4:
			ret = l5.deserialize(is, size);
			break;
		case 5:
			ret = l6.deserialize(is, size);
			break;
		default:
			std::cout << "ERROR: RectilinearModelManager::loadFromFile() called on " << i << std::endl;
			return NULL;
		}
		is.close();

		std::cout << "Read " << size << " strongly connected models of size " << (i + 1) << " from " << fileName << std::endl;
		return ret;
	}

	void RectilinearModelManager::loadFromFile(std::set<FatBlock> &s, std::vector<int> combinationType) const {
		int combinedSize = 0;
		for (std::vector<int>::const_iterator it = combinationType.begin(); it != combinationType.end(); ++it)
			combinedSize += *it;

		std::stringstream ss;
		ss << "blocks\\" << combinedSize << "\\combination_type";
		for (std::vector<int>::const_iterator it = combinationType.begin(); it != combinationType.end(); ++it)
			ss << "_" << *it;
		ss << ".dat";
		std::cout << "Reading file with strongly connected models from " << ss.str() << std::endl;

		// Read:
		std::ifstream infile;
		infile.open(ss.str().c_str(), std::ios::binary | std::ios::in);

		while (true) {
			FatBlock model;
			model.size = combinedSize;
			model.index = NO_INDEX;
			model.deserialize(infile);

			if (model.otherBricks[0].isBase())
				break;

			s.insert(model.rotateToMin());
		}
		infile.close();
	}

	FatBlock* RectilinearModelManager::loadFromFile(int i, unsigned long &size, bool oldBlockFile) const {
		std::stringstream ss;
		if (oldBlockFile)
			ss << "old_rc\\";
		else
			ss << "blocks\\";
		ss << (i + 1) << ".dat";
		std::cout << "Reading file with strongly connected models of size " << (i + 1) << " from " << ss.str() << std::endl;
		return loadFromFile(ss.str(), i, size);
	}
}