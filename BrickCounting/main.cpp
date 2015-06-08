#include "RectilinearBrick.h"
#include "StronglyConnectedConfiguration.hpp"
#include "StronglyConnectedConfigurationManager.h"
#include "ConfigurationManager.h"
#include <iostream>
#include <fstream>
#include <assert.h>

void ensureFoldersAreCreated() {
  std::cout << "Ensuring that the necessary folders exist." << std::endl;
  std::cout << "Please ensure this program has permissions to create folders and write files here." << std::endl;

#ifdef _WIN32
  CreateDirectory(L"scc", NULL);
/*  CreateDirectory(L"scc\\1", NULL);
  CreateDirectory(L"scc\\2", NULL);
  CreateDirectory(L"scc\\3", NULL);
  CreateDirectory(L"scc\\4", NULL);
  CreateDirectory(L"scc\\5", NULL);*/
  CreateDirectory(L"scc\\6", NULL); // For hash lists.
  CreateDirectory(L"models", NULL);
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

bool sccFilesExist() {
  if(!fileExist("scc\\1.dat"))
    return false;
  if(!fileExist("scc\\2.dat"))
    return false;
  if(!fileExist("scc\\3.dat"))
    return false;
  if(!fileExist("scc\\4.dat"))
    return false;
#ifdef _DEBUG
  return true;
#endif
  if(!fileExist("scc\\5.dat"))
    return false;
  return true;
}

/*
* MAIN ENTRY POINT!
*/
int main(int, char**) {  
#ifdef _DEBUG
  std::cout << "DEBUG MODE" << std::endl;
#endif
  ensureFoldersAreCreated();

  if(!sccFilesExist()) { // Create SCC data files:
    StronglyConnectedConfigurationManager mgr;
    mgr.create();
  }

  // RC:
  ConfigurationManager mgr;
  mgr.test();
  //mgr.runForSize(2); 
  //mgr.runForSize(3);
  //mgr.runForSize(4);
  //mgr.runForSize(5);
  //mgr.runForSize(6);
  return 0;
}
