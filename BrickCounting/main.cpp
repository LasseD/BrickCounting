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
  CreateDirectory(L"scc\\6", NULL); // For hash lists.
  CreateDirectory(L"nrc", NULL);
  CreateDirectory(L"nrc\\4", NULL);
  CreateDirectory(L"nrc\\5", NULL);
  CreateDirectory(L"nrc\\6", NULL);
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
  CreateDirectory("nrc", NULL);
  CreateDirectory("nrc\\4", NULL);
  CreateDirectory("nrc\\5", NULL);
  CreateDirectory("nrc\\6", NULL);
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
    std::cout << " Computes all models of a given combination type." << std::endl;
    std::cout << " Specify the combination type using input paramenter." << std::endl;
    std::cout << " Examples:" << std::endl;
    std::cout << "  For combination type 3/1 use the parameters 3 1" << std::endl;
    std::cout << "  For combination type 5/1 use the parameters 3 1" << std::endl;
    std::cout << "  For combination type 2/2/1 use the parameters 2 2 1" << std::endl;
    std::cout << " Limitations:" << std::endl;
    std::cout << "  Max configuration size is 6." << std::endl;
    std::cout << "  Combination types with 4 or more SCCs might use too much memory (and time)." << std::endl;
}

/*
* MAIN ENTRY POINT!
*/
int main(int numArgs, char** argV) {  
#ifdef _DEBUG
  std::cout << "DEBUG MODE" << std::endl;
#endif
  if(numArgs < 2) {
    printUsage();
    return 1;
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
    return 2;
  }

  ensureFoldersAreCreated();
  if(!sccFilesExist(maxSccSize)) { // Create SCC data files:
    StronglyConnectedConfigurationManager mgr;
    mgr.create(maxSccSize);
  }

  std::sort(combinationType.begin(), combinationType.end(), std::greater<int>());
  ConfigurationManager mgr;
  mgr.runForCombinationType(combinationType, combinedSize);
  //mgr.test();
  //mgr.runForSize(3);
  //mgr.runForSize(4);
  //mgr.runForSize(5);

  //system("pause");
  //return 0;
}
