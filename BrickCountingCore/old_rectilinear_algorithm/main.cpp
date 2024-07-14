// -*- mode: c++; tab-width: 2; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=2 sts=2 sw=2 noet :

#include <iostream>
#include <stdlib.h>
#include "rectilinear.h"

void countRefinements(char* input) {
		int size = 0, token = 0;
		char c;
		for(int i = 0; (c = input[i]); i++) {
				token = token * 10 + (c-'0');
				size += (c-'0');
		}
		uint64_t added = 0, symmetries = 0;
		bool saveOutput = true;
		switch(size) {
		case 2:
				rectilinear::handleCombinationWriters<2>(token, 0, false, added, symmetries, saveOutput);
				break;
		case 3:
				rectilinear::handleCombinationWriters<3>(token, 0, false, added, symmetries, saveOutput);
				break;
		case 4:
				rectilinear::handleCombinationWriters<4>(token, 0, false, added, symmetries, saveOutput);
				break;
		case 5:
				rectilinear::handleCombinationWriters<5>(token, 0, false, added, symmetries, saveOutput);
				break;
		case 6:
				rectilinear::handleCombinationWriters<6>(token, 0, false, added, symmetries, saveOutput);
				break;
		case 7:
				rectilinear::handleCombinationWriters<7>(token, 0, false, added, symmetries, saveOutput);
				break;
		case 8:
				rectilinear::handleCombinationWriters<8>(token, 0, false, added, symmetries, saveOutput);
				break;
		case 9:
				rectilinear::handleCombinationWriters<9>(token, 0, false, added, symmetries, saveOutput);
				break;
		case 10:
				rectilinear::handleCombinationWriters<10>(token, 0, false, added, symmetries, saveOutput);
				break;
		case 11:
				rectilinear::handleCombinationWriters<11>(token, 0, false, added, symmetries, saveOutput);
				break;
		default:
				std::cout << "Model size not supported: " << size << std::endl;
				break;
		}
}

/*
- file format:
-- Rectilinear combination bit usage (11 bit total):
 First bit: 0 for vertical, 1 for horizontal
 x index: 0 to 31: -15 to 15 (5 bit)
 y index: 0 to 31: -15 to 15 (5 bit)
 Restrictions:
  - first brick must be a 000 config
  - lower lv. 0 must be minimal (lex on bricks)
  - bricks must be x,y lex sorted on layers	
 */
int main(int argc, char** argv) {
		bool saveFiles = true;
		switch(argc) {
		case 1:
				//rectilinear::build_all_combinations<2>(saveFiles);
				//rectilinear::build_all_combinations<3>(saveFiles);
				//rectilinear::build_all_combinations<4>(saveFiles);
				rectilinear::build_all_combinations<5>(saveFiles);
				//rectilinear::build_all_combinations<6>(saveFiles);
				break;
		case 2:
				countRefinements(argv[1]);
				break;
		default:
				std::cout << "Usage: Run without arguments to call all models. Or specify refinement like 121 to run for specific refinement A(4,3,1,2,1) as an example." << std::endl;
				break;
		}
		return 0;
}
