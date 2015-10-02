#include "RectilinearBrick.h"
#include "StronglyConnectedConfiguration.hpp"
#include "StronglyConnectedConfigurationManager.h"
#include "ConfigurationManager.h"
#include <iostream>
#include <fstream>
#include <assert.h>
#include <algorithm>
#include <functional>

void ensureFoldersAreCreated() {
  std::cout << "Ensuring the necessary folders exist." << std::endl;
  std::cout << "Please ensure this program has permissions to create folders and write files here." << std::endl;

#ifdef _WIN32
  CreateDirectory(L"scc", NULL);
  CreateDirectory(L"scc\\6", NULL); // For hash lists and combination type lists.
  CreateDirectory(L"models", NULL);
  CreateDirectory(L"models\\4", NULL);
  CreateDirectory(L"models\\5", NULL);
  CreateDirectory(L"models\\6", NULL);
  CreateDirectory(L"manual", NULL);
  CreateDirectory(L"manual\\4", NULL);
  CreateDirectory(L"manual\\5", NULL);
  CreateDirectory(L"manual\\6", NULL);
  CreateDirectory(L"old_rc", NULL); // Using the simple (old) way of counting all rectilinear configurations.
#else
  // G++:
  CreateDirectory("scc", NULL);
  CreateDirectory("scc\\6", NULL);
  CreateDirectory("models", NULL);
  CreateDirectory("models\\4", NULL);
  CreateDirectory("models\\5", NULL);
  CreateDirectory("models\\6", NULL);
  CreateDirectory("manual", NULL);
  CreateDirectory("manual\\4", NULL);
  CreateDirectory("manual\\5", NULL);
  CreateDirectory("manual\\6", NULL);
  CreateDirectory("old_rc", NULL);
#endif

  std::cout << "Folder structure OK." << std::endl;
}

bool fileExist(char const * const fileName) {
  std::ifstream file(fileName);
  return file.good();
}

bool sccFilesExist(int maxSccSize) {
  if(!fileExist("scc\\1.dat"))
    return false;
  if(maxSccSize <= 1)
    return true;
  if(!fileExist("scc\\2.dat"))
    return false;
  if(maxSccSize <= 2)
    return true;
  if(!fileExist("scc\\3.dat"))
    return false;
  if(maxSccSize <= 3)
    return true;
  if(!fileExist("scc\\4.dat"))
    return false;
  if(maxSccSize <= 4)
    return true;
  if(!fileExist("scc\\5.dat"))
    return false;
  return true;
}

void printUsage() {
    std::cout << "Usage:" << std::endl;
    std::cout << " Computes all models of a given combination type or size." << std::endl;
    std::cout << " Specify the combination type using input paramenter." << std::endl;
    std::cout << " Examples:" << std::endl;
    std::cout << "  Combination type 3/1: BrickCounting.exe 3 1" << std::endl;
    std::cout << "  Combination type 5/1: BrickCounting.exe 5 1" << std::endl;
    std::cout << "  Combination type 2/2/1: BrickCounting.exe 2 2 1" << std::endl;
    std::cout << " Alternatively. Specify the size of the combination using a single input parameter. All combination types with the combined size will be computed." << std::endl;
    std::cout << " Examples:" << std::endl;
    std::cout << "  Combination size 3: BrickCounting.exe 3" << std::endl;
    std::cout << "  Combination size 4: BrickCounting.exe 4" << std::endl;
    std::cout << " Limitations:" << std::endl;
    std::cout << "  Max configuration size is 6." << std::endl;
    std::cout << "  Combination types with 3 or more SCCs might use too much memory (and time)." << std::endl;
    std::cout << " Special Use:" << std::endl;
    std::cout << "  Compute all Rectilinear configurations: BrickCounting.exe R" << std::endl;
    std::cout << "  Compute a lower bound for the number of models for a given size (such as size 4): BrickCounting.exe X 4" << std::endl;
}

/*
* MAIN ENTRY POINT!
*/
int main(int numArgs, char** argV) {  
#ifdef _DEBUG
  std::cout << "DEBUG MODE" << std::endl;
#endif
  if(numArgs <= 1) {
    printUsage();
    ConfigurationManager mgr(3, true);
    mgr.test();

    return 1;
  }
  ensureFoldersAreCreated();

  if(numArgs == 2 || (numArgs == 3 && argV[1][0] == 'X')) {
    if(argV[1][0] == 'R') {
      RectilinearConfigurationManager sccMgr;
      sccMgr.createOld();
      return 9;
    }

    int sccSize = argV[numArgs-1][0]-'0';

    if(!sccFilesExist(sccSize-1)) { // Create SCC data files:
      RectilinearConfigurationManager mgr;
      mgr.create(sccSize-1);
    }

    if(sccSize < 3 || sccSize > 6) {
      printUsage();
      return 2;
    }

#ifdef _COMPARE_ALGORITHMS
    ConfigurationManager mgr(sccSize, numArgs == 3);
#else
    ConfigurationManager mgr(sccSize-1, numArgs == 3);
#endif
    mgr.runForSize(sccSize);
    return 0;
  }

  int combinedSize = 0;
  int maxSccSize = 1;
  std::vector<int> combinationType;
  for(int i = 1; i < numArgs; ++i) {
    int sccSize = argV[i][0]-'0';
#ifdef _DEBUG
    std::cout << " SCC SIZE: " << sccSize << std::endl;
#endif
    combinedSize+=sccSize;
    combinationType.push_back(sccSize);
    if(sccSize > maxSccSize)
      maxSccSize = sccSize;
  }
  if(combinedSize < 4 || combinedSize > 6) {
    printUsage();
    return 3;
  }

  if(!sccFilesExist(maxSccSize)) { // Create SCC data files:
    RectilinearConfigurationManager mgr;
    mgr.create(maxSccSize);
  }

  std::sort(combinationType.begin(), combinationType.end(), std::greater<int>());
#ifdef _COMPARE_ALGORITHMS
  ConfigurationManager mgr(combinedSize, false);
#else
  ConfigurationManager mgr(maxSccSize, false);
#endif
  mgr.runForCombinationType(combinationType, combinedSize);
  return 0;
}
