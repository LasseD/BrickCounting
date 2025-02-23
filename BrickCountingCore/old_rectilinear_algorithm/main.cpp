#include <iostream>
#include <stdlib.h>
#include "rectilinear.h"

/*
  TODO:
  - countX3(), X >= 3
   - Use CutCombination of 2 layers with bit-array of connectivity for layer 1:
    - CutCombination:
    - There are Y bricks in layer 1.
    - For i = 0 .. Y-1:
     - For j = i+1 .. Y-1
      Set bit if brick i connects to brick j above layer 1.
  - Generalize to countXY(...)
  - Create Max2LayerReader for <22..2> combinations:
   - Consider layer2 instead of layer1:
    - layer2 of size 2.
    - Construct cache for all positions of the bricks of layer2 up to what can be connected from the two layers below.
    - Reuse logic countX2()

  Performance:
  Eiler et. al. can compute A(6) in 3 second.
  Current performance of this code base:
   Writing all files up to size 6: 11 minutes, 53 seconds
   Counting for size 6 without file writing: 1 minute, 6 seconds
  Significant performance improvements are required!
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
  bool saveFiles = false;
  rectilinear::Counter c;
  switch(argc) {
  case 1:
    for(int i = 2; i <= 6; i++)
      c.buildAllCombinations(i, saveFiles);
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
