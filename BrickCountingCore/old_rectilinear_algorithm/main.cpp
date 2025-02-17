#include <iostream>
#include <stdlib.h>
#include "rectilinear.h"

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

int main(int argc, char** argv) {
  bool saveFiles = true;
  rectilinear::Counter c;
  switch(argc) {
  case 1:
    c.buildAllCombinations(2, saveFiles);
    c.buildAllCombinations(3, saveFiles);
    c.buildAllCombinations(4, saveFiles);
    c.buildAllCombinations(5, saveFiles);
    c.buildAllCombinations(6, saveFiles);
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
