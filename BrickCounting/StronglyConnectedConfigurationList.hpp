#ifndef STRONGLY_CONNECTED_CONFIGURATION_LIST_HPP
#define STRONGLY_CONNECTED_CONFIGURATION_LIST_HPP

#include "LDRPrinter.h"
#include "StronglyConnectedConfiguration.hpp"
#include "Util.hpp"

#include <set>
#include <map>
#include <iostream>
#include <fstream>
#include <assert.h> 
#include <sstream>
#include <windows.h>

template <unsigned int ELEMENT_SIZE>
class CombinationTypeList; // Forward declaration

template <unsigned int ELEMENT_SIZE>
class RectilinearConfigurationHashList {
public:
  std::ofstream *os;
  std::string fileName;
  unsigned long written, hash;

  RectilinearConfigurationHashList() {} // to satisfy array constructor.

  RectilinearConfigurationHashList(const unsigned long hash) : written(0), hash(hash) {}

  void open() {
    std::stringstream ss;
    ss << "scc\\" << ELEMENT_SIZE;
    std::string s = ss.str();
    ss << "\\hash" << hash << ".dat";

    fileName = ss.str();    
    os = new std::ofstream; // deleted in count()
    os->open(fileName.c_str(), std::ios::binary | std::ios::out);
  }

  void output(const RectilinearConfiguration<ELEMENT_SIZE> &c) {
    c.serialize(*os);
    ++written;
  }

  void printMPDFile(const unsigned long hash, std::set<RectilinearConfiguration<ELEMENT_SIZE> > &s) {
    MPDPrinter h;

    typename std::set<RectilinearConfiguration<ELEMENT_SIZE> >::const_iterator it = s.begin();
    for(int i = 0; it != s.end(); ++it, ++i) {
      LDRPrintable const * p = &(*it);
      std::stringstream ss;
      ss << "scc_" << i;
      h.add(ss.str(), p);
    }

    std::stringstream ss;
    ss << "scc\\HashList_" << ELEMENT_SIZE;
    ss << "_" << hash;
    h.print(ss.str());
  }

  // Close stream, then read and count. 
  unsigned long count(std::map<uint32_t,CombinationTypeList<ELEMENT_SIZE>* > &combinationTypeLists) {
    os->close();
    delete os;
    std::cout << "Written " << written << " unsorted elements of hash " << hash << " to stream. Now counting." << std::endl;

    // Read and count!
    std::ifstream infile;
    infile.open(fileName.c_str(), std::ios::binary | std::ios::in);

    std::set<RectilinearConfiguration<ELEMENT_SIZE> > s; 
    for(unsigned long i = 0; i < written; ++i) {
      RectilinearConfiguration<ELEMENT_SIZE> candidate;
      candidate.deserialize(infile);

      if(s.find(candidate) != s.end()) {
        continue;
      }
      if(candidate.canTurn90()) {
        bool found = false;
        for(int i = 0; i < 3; ++i) {
          candidate.turn90();
          if(s.find(candidate) != s.end()) {
            found = true;
            break;
          }
        }
        if(found)
          continue;
      }
      else {
        candidate.turn180();
        if(s.find(candidate) != s.end()) {
          continue;
        }
      }
      s.insert(candidate);

      if(ELEMENT_SIZE > 2) {
        util::TinyVector<int, 6> combinationType;
        candidate.getCombinationType(combinationType);
        uint32_t encoding = math::encodeCombinationType(combinationType);

        assert(combinationTypeLists.find(encoding) != combinationTypeLists.end());
        combinationTypeLists[encoding]->writeConfiguration(candidate);
      }
    }
    infile.close();
    unsigned int res = (unsigned int)s.size();
    std::cout << "Read " << res << " sorted elements of hash " << hash << std::endl;
    //printMPDFile(hash, s); // Comment in for great visual debugging!
    s.clear();
    return res;    
  }
};

template <unsigned int ELEMENT_SIZE>
class CombinationTypeList {
public:
  std::ofstream *os;
  std::string fileName;
  unsigned long written;
  util::TinyVector<int, 6> combinationType;

  CombinationTypeList() : written(0) {} // to satisfy array constructor.

  CombinationTypeList(const util::TinyVector<int, 6> &combinationType) : written(0), combinationType(combinationType) {
    std::stringstream ss;
    ss << "scc\\" << ELEMENT_SIZE;
    ss << "\\combination_type";
    for(const int* it = combinationType.begin(); it != combinationType.end(); ++it)
      ss << "_" << *it;
    ss << ".dat";
    //std::cout << " Creating combination type list for combination type " << ss.str() << std::endl;

    fileName = ss.str();
  }

  void openForWrite() {
    os = new std::ofstream; // deleted in closeForWrite()
    os->open(fileName.c_str(), std::ios::binary | std::ios::out);
  }
  void writeConfiguration(const RectilinearConfiguration<ELEMENT_SIZE> &c) {
    c.serialize(*os);
    ++written;
  }
  void closeForWrite() {
    std::cout << "Written " << written << " configurations of combination type ";
    for(const int* it = combinationType.begin(); it != combinationType.end(); ++it)
      std::cout << " " << *it;
    std::cout << std::endl;

    // Add end element:
    RectilinearConfiguration<ELEMENT_SIZE> endElement;
    RectilinearBrick baseBrick;
    endElement.otherBricks[0] = baseBrick;
    writeConfiguration(endElement);

    os->close();
    delete os;
  }

  void readFile(std::set<RectilinearConfiguration<ELEMENT_SIZE> > &s) {
    // Read and count!
    std::ifstream infile;
    infile.open(fileName.c_str(), std::ios::binary | std::ios::in);

    while(true) {
      RectilinearConfiguration<ELEMENT_SIZE> configuration;
      configuration.deserialize(infile);

      if(configuration.otherBricks[0].isBase())
        break;

      s.insert(configuration);
    }
    infile.close();
  }
};

template <unsigned int ELEMENT_SIZE>
class RectilinearConfigurationList {
public:
  std::set<RectilinearConfiguration<ELEMENT_SIZE> > s; // only used for construction.
private:
  std::map<unsigned int,RectilinearConfigurationHashList<ELEMENT_SIZE> > hashLists; // Only used in countAllFor, that is, when we can't store all.
  std::map<uint32_t,CombinationTypeList<ELEMENT_SIZE>* > combinationTypeLists;

  void createCombinationTypeLists(const util::TinyVector<int, 6> &combinationType, int remaining) {
    if(remaining == 0) {
      uint32_t encodedCombinationType = math::encodeCombinationType(combinationType);
      CombinationTypeList<ELEMENT_SIZE> *list = new CombinationTypeList<ELEMENT_SIZE>(combinationType); // deleted in ~RectilinearConfigurationList()
      combinationTypeLists.insert(std::make_pair(encodedCombinationType,list));
      return;
    }
    for(int i = MIN(remaining,combinationType[combinationType.size()-1]); i > 0; --i) {
      util::TinyVector<int, 6> ct(combinationType);
      ct.push_back(i);
      createCombinationTypeLists(ct, remaining-i);
    }
  }

public:
  RectilinearConfigurationList() {
    if(ELEMENT_SIZE <= 2) 
      return;
    //std::cout << "Creating RectilinearConfigurationList of size " << ELEMENT_SIZE << std::endl;
    // Create combinationTypeLists:
    for(int i = ELEMENT_SIZE; i > 0; --i) {
      util::TinyVector<int, 6> combinationType;
      combinationType.push_back(i);
      createCombinationTypeLists(combinationType, ELEMENT_SIZE-i);
    }
  }
  ~RectilinearConfigurationList() {
    typename std::map<uint32_t,CombinationTypeList<ELEMENT_SIZE>* >::iterator it;
    for(it = combinationTypeLists.begin(); it != combinationTypeLists.end(); ++it) {
      CombinationTypeList<ELEMENT_SIZE>* list = it->second;
      delete list;
    }
    combinationTypeLists.clear();
  }

  void serialize(std::ofstream &os) {
    unsigned int size = (unsigned int)s.size();
    os.write(reinterpret_cast<const char *>(&size), sizeof(unsigned long));

    typename std::set<RectilinearConfiguration<ELEMENT_SIZE> >::const_iterator it;
    for(it = s.begin(); it != s.end(); ++it) {
      it->serialize(os);
    }
  }
  FatSCC* deserialize(std::ifstream &is, unsigned long &size) const {
    is.read((char*)&size, sizeof(unsigned long));
    FatSCC* v = new FatSCC[size]; // Saved stream is never deleted.
    for(unsigned int i = 0; i < size; ++i) {
      RectilinearConfiguration<ELEMENT_SIZE> scc;
      scc.deserialize(is);
      v[i] = FatSCC(scc, i);
    }
    return v;
  }
  void deserialize(std::ifstream &is, std::set<RectilinearConfiguration<ELEMENT_SIZE> > &s) const {
    unsigned long size;
    is.read((char*)&size, sizeof(unsigned long));
    for(int i = 0; i < size; ++i) {
      RectilinearConfiguration<ELEMENT_SIZE> scc;
      scc.deserialize(is);
      s.insert(scc);
    }
  }

private:
  void getAllNewBricks(const RectilinearConfiguration<ELEMENT_SIZE-1> &smaller, std::set<RectilinearBrick> &newBricks, bool includeNonSCCs) {
    // Add for all bricks in smaller:
    int tmpSize;
    RectilinearBrick tmp[STRONGLY_CONNECTED_BRICK_POSITIONS];

    // Handle origin as special brick (because it doesn't exist in otherBricks):
    RectilinearBrick b;
    for(int i = 0; i < ELEMENT_SIZE-1; b = smaller.otherBricks[i++]) {
      tmpSize = 0;
      b.constructAllStronglyConnected(tmp, tmpSize, includeNonSCCs);

      for(int j = 0; j < tmpSize; ++j) {
        RectilinearBrick &b2 = tmp[j];
        if(!smaller.intersects(b2))
          newBricks.insert(b2);
      }
    }
  }

public:
  /*
  Add all scc's of this size from the scc c of a size smaller:
  1) Have a small set s in which to put all possible bricks that can connect strongly to c
  1a) Ensure no intersection with c when doing this!
  2) Add all x from s to c (and sort to c') to self. First rotate c' to check for duplicates. 
  */
  void addAllFor(const RectilinearConfiguration<ELEMENT_SIZE-1> &smaller, bool includeNonSCCs) {
    std::set<RectilinearBrick> newBricks;
    getAllNewBricks(smaller, newBricks, includeNonSCCs);

    // Try all new scc's:
    for(std::set<RectilinearBrick>::const_iterator it = newBricks.begin(); it != newBricks.end(); ++it) {
      RectilinearConfiguration<ELEMENT_SIZE> candidate(smaller,*it);

      if(s.find(candidate) != s.end()) {
        continue;
      }
      if(candidate.canTurn90()) {
        bool found = false;
        for(int i = 0; i < 3; ++i) {
          candidate.turn90();
          if(s.find(candidate) != s.end()) {
            found = true;
            break;
          }
        }
        if(found)
          continue;
      }
      else {
        candidate.turn180();
        if(s.find(candidate) != s.end()) {
          continue;
        }
      }
      s.insert(candidate);
      if(ELEMENT_SIZE > 2) {
        util::TinyVector<int, 6> combinationType;
        candidate.getCombinationType(combinationType);
        uint32_t encoding = math::encodeCombinationType(combinationType);

        assert(combinationTypeLists.find(encoding) != combinationTypeLists.end());
        combinationTypeLists[encoding]->writeConfiguration(candidate); // Notize: Not minimized! rotateToMin has to be done on read!
      }
    }
  }

  void addAllFor(RectilinearConfigurationList<ELEMENT_SIZE-1> &smaller, bool includeNonSCCs) {
    if(ELEMENT_SIZE > 2) {
      typename std::map<uint32_t,CombinationTypeList<ELEMENT_SIZE>* >::const_iterator it;
      for(it = combinationTypeLists.begin(); it != combinationTypeLists.end(); ++it)
        it->second->openForWrite();
    }

    typename std::set<RectilinearConfiguration<ELEMENT_SIZE-1> >::const_iterator it;
    for(it = smaller.s.begin(); it != smaller.s.end(); ++it) {
      addAllFor(*it, includeNonSCCs);
    }

    if(ELEMENT_SIZE > 2) {
      typename std::map<uint32_t,CombinationTypeList<ELEMENT_SIZE>* >::const_iterator it;
      for(it = combinationTypeLists.begin(); it != combinationTypeLists.end(); ++it)
        it->second->closeForWrite();
    }
  }

private:
  void countAllFor(const RectilinearConfiguration<ELEMENT_SIZE-1> &smaller, bool includeNonSCCs) {
    std::set<RectilinearBrick> newBricks;
    getAllNewBricks(smaller, newBricks, includeNonSCCs);

    // Try all new scc's:
    for(std::set<RectilinearBrick>::const_iterator it = newBricks.begin(); it != newBricks.end(); ++it) {
      RectilinearConfiguration<ELEMENT_SIZE> candidate(smaller,*it);
      unsigned long hash = candidate.layerHash();

      // ensure existence:
      if(hashLists.find(hash) == hashLists.end()) {
        RectilinearConfigurationHashList<ELEMENT_SIZE> newList(hash);
        newList.open();
        hashLists.insert(std::make_pair(hash, newList));
      }

      hashLists.find(hash)->second.output(candidate);
    }
  }

public:
  unsigned long countAllFor(RectilinearConfigurationList<ELEMENT_SIZE-1> &smaller, bool includeNonSCCs) {
    if(ELEMENT_SIZE > 2) {
      typename std::map<uint32_t,CombinationTypeList<ELEMENT_SIZE>* >::const_iterator it;
      for(it = combinationTypeLists.begin(); it != combinationTypeLists.end(); ++it)
        it->second->openForWrite();
    }

    typename std::set<RectilinearConfiguration<ELEMENT_SIZE-1> >::const_iterator it;
    for(it = smaller.s.begin(); it != smaller.s.end(); ++it) {
      countAllFor(*it, includeNonSCCs);
    }

    // Now do the counting:
    unsigned long sum = 0;
    typename std::map<unsigned int,RectilinearConfigurationHashList<ELEMENT_SIZE> >::iterator it2;
    for(it2 = hashLists.begin(); it2 != hashLists.end(); ++it2) {
      RectilinearConfigurationHashList<ELEMENT_SIZE> &configuration = it2->second;
      sum += configuration.count(combinationTypeLists);
    }
    std::cout << "Number of RectilinearConfigurations of size " << ELEMENT_SIZE << ": " << sum << std::endl;

    if(ELEMENT_SIZE > 2) {
      typename std::map<uint32_t,CombinationTypeList<ELEMENT_SIZE>* >::const_iterator it;
      for(it = combinationTypeLists.begin(); it != combinationTypeLists.end(); ++it)
        it->second->closeForWrite();
    }

    return sum;
  }

  void printMPDFile(bool includeNonSCCs = false, int limitSubModelsInFile = 2147483647) {
    MPDPrinter h;

    typename std::set<RectilinearConfiguration<ELEMENT_SIZE> >::const_iterator it = s.begin();
    for(int i = 0; it != s.end() && i < limitSubModelsInFile; ++it, ++i) {
      LDRPrintable const * p = &(*it);
      std::stringstream ss;
      ss << "scc_" << i;
      h.add(ss.str(), p);
    }

    std::stringstream ss;
    if(includeNonSCCs)
      ss << "old_rc";
    else
      ss << "scc";
    ss << "\\RectilinearConfigurationOfSize" << ELEMENT_SIZE;
    h.print(ss.str());
  }
};

template <unsigned int ELEMENT_SIZE>
std::ostream& operator<<(std::ostream& os, const RectilinearConfigurationList<ELEMENT_SIZE>& l) {
  os << "Combinations of size " << ELEMENT_SIZE << ": " << l.s.size() << std::endl;
  return os;
};

#endif // STRONGLY_CONNECTED_CONFIGURATION_LIST_HPP
