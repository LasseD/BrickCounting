#ifndef COUNTING_BLOCK_MANAGER_H
#define COUNTING_BLOCK_MANAGER_H

#include <set>
#include <iostream>
#include <fstream>

#include "../modelling/Block.hpp"
#include "BlockList.hpp"

using namespace modelling;

namespace counting {
	class RectilinearModelManager {
	public:
		RectilinearModelManager();
		~RectilinearModelManager();
		void create(int maxSize);
		void createOld();
		void loadAllFromDisk();

		void **lists;

		FatBlock* loadFromFile(int i, unsigned long &size, bool oldBlockFile) const;
		//FatBlock* loadFromFile(std::vector<int> combinationType, unsigned long &size) const;
		void loadFromFile(std::set<FatBlock> &s, std::vector<int> combinationType) const;

	private:
		FatBlock* loadFromFile(std::string fileName, int i, unsigned long &size) const;

		RectilinearModelList<1> l1;
		RectilinearModelList<2> l2;
		RectilinearModelList<3> l3;
		RectilinearModelList<4> l4;
		RectilinearModelList<5> l5;
		RectilinearModelList<6> l6;

		void writeToFile(int i, bool old = false);
	};
}

#endif // COUNTING_BLOCK_MANAGER_H
