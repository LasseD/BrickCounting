// -*- mode: c++; tab-width: 2; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=2 sts=2 sw=2 noet :

#include <iostream>
#include <stdlib.h>
#include "rectilinear.h"

uint64_t added, symmetries, earlyExits;

/*
	TODO:
	- Make CombinationReader into interface
	- Have SpindleReader for Lemma 1
	- Create Max2LayerReader for <22...2> combinations
 */
void countRefinements(char* input, bool saveOutput) {
		int size = 0, token = 0,  height = 0;
		char c;
		for(int i = 0; (c = input[i]); i++) {
				token = token * 10 + (c-'0');
				size += (c-'0');
				height++;
		}
		switch(size) {
		case 2:
				rectilinear::countRefinements<2>(token, height, input, saveOutput);
				break;
		case 3:
				rectilinear::countRefinements<3>(token, height, input, saveOutput);
				break;
		case 4:
				rectilinear::countRefinements<4>(token, height, input, saveOutput);
				break;
		case 5:
				rectilinear::countRefinements<5>(token, height, input, saveOutput);
				break;
		case 6:
				rectilinear::countRefinements<6>(token, height, input, saveOutput);
				break;
		case 7:
				rectilinear::countRefinements<7>(token, height, input, saveOutput);
				break;
		case 8:
				rectilinear::countRefinements<8>(token, height, input, saveOutput);
				break;
		case 9:
				rectilinear::countRefinements<9>(token, height, input, saveOutput);
				break;
		case 10:
				rectilinear::countRefinements<10>(token, height, input, saveOutput);
				break;
		case 11:
				rectilinear::countRefinements<11>(token, height, input, saveOutput);
				break;
		default:
				std::cout << "Model size not supported: " << size << std::endl;
				break;
		}
}

/*
 Rectilinear models with restrictions on representation:
  - first brick must be vertical at first layer and placed at 0,0
  - model is minimal of all rotations (vs rotated 90, 180, 270 degrees)
  - bricks are lexicographically sorted (orientation,x,y) on each layer
 */
int main(int argc, char** argv) {
		bool saveFiles = true;
		switch(argc) {
		case 1:
				rectilinear::build_all_combinations<2>(saveFiles);
				rectilinear::build_all_combinations<3>(saveFiles);
				rectilinear::build_all_combinations<4>(saveFiles);
				rectilinear::build_all_combinations<5>(saveFiles);
				rectilinear::build_all_combinations<6>(saveFiles);
				break;
		case 2:
				countRefinements(argv[1], false);
				break;
		case 3:
				countRefinements(argv[1], true);
				break;
		default:
				std::cout << "Usage: Run without arguments to call all models. Or specify refinement like 121 [save] to run for specific refinement A(4,3,1,2,1) as an example. A second argument will cause the output to be saved on disk." << std::endl;
				break;
		}
		return 0;
}
