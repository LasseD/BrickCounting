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
  rectilinear::Counter counter;
  counter.countRefinements(size, token, height, input, saveOutput);
}

/*
  Rectilinear models with restrictions on representation:
  - first brick must be vertical at first layer and placed at 0,0
  - model is minimal of all rotations (vs rotated 90, 180, 270 degrees)
  - bricks are lexicographically sorted (orientation,x,y) on each layer
*/
int main(int argc, char** argv) {
  bool saveFiles = true;
  rectilinear::Counter c;
  switch(argc) {
  case 1:
    c.build_all_combinations(2, saveFiles);
    c.build_all_combinations(3, saveFiles);
    c.build_all_combinations(4, saveFiles);
    c.build_all_combinations(5, saveFiles);
    c.build_all_combinations(6, saveFiles);
    break;
  case 2:
    countRefinements(argv[1], false);
    break;
  case 3:
    countRefinements(argv[1], true);
    break;
  default:
    std::cout << "Usage: Run without arguments to construct all models up to size 6. Specify a refinement like 121 to run for specific refinement <121>. A second argument will cause the output to be saved on disk." << std::endl;
    break;
  }
  return 0;
}
