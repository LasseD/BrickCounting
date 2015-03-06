#ifndef STRONGLY_CONNECTED_CONFIGURATION_LIST_HPP
#define STRONGLY_CONNECTED_CONFIGURATION_LIST_HPP

#include "LDRPrinter.h"
#include "StronglyConnectedConfiguration.hpp"

#include <set>
#include <map>
#include <iostream>
#include <fstream>
#include <assert.h> 
#include <sstream>
#include <windows.h>

template <unsigned int ELEMENT_SIZE>
class StronglyConnectedConfigurationHashList {
public:
  std::ofstream *os;
  std::string fileName;
  unsigned long written;
  int hash;
  
  StronglyConnectedConfigurationHashList() {} // to satisfy array constructor.

  StronglyConnectedConfigurationHashList(const int hash) : written(0), hash(hash) {}

  void open() {
    std::stringstream ss;
    ss << "scc\\" << ELEMENT_SIZE;
    CreateDirectory(ss.str().c_str(), NULL);
    ss << "\\hash" << hash << ".dat";

    fileName = ss.str();    
    os = new std::ofstream;
    os->open(fileName.c_str(), std::ios::binary | std::ios::out);
  }

  void output(const StronglyConnectedConfiguration<ELEMENT_SIZE> &c) {
    c.serialize(*os);
    ++written;
  }

  // Close stream, then read and count. 
  unsigned long count() {
    os->close();
    delete os;
    std::cout << "Written " << written << " unsorted elements of hash " << hash << " to stream. Now counting." << std::endl;

    // Read and count!
    std::ifstream infile;
    infile.open(fileName.c_str(), std::ios::binary | std::ios::in);

    std::set<StronglyConnectedConfiguration<ELEMENT_SIZE> > s; 
    for(unsigned long i = 0; i < written; ++i) {
      StronglyConnectedConfiguration<ELEMENT_SIZE> candidate;
      candidate.deserialize(infile);

      if(s.find(candidate) != s.end()) {
        continue;
      }
      if(candidate.canTurn90()) {
        for(int i = 0; i < 3; ++i) {
          candidate.turn90();
          if(s.find(candidate) != s.end()) {
            continue;
          }
        }
      }
      else {
        candidate.turn180();
        if(s.find(candidate) != s.end()) {
          continue;	
        }
      }
      s.insert(candidate);
    }
    infile.close();
    unsigned long res = s.size();
    std::cout << "Read " << res << " sorted elements of hash " << hash << std::endl;
    s.clear();
    return res;    
  }
};

template <unsigned int ELEMENT_SIZE>
class StronglyConnectedConfigurationList {
public:
  StronglyConnectedConfiguration<ELEMENT_SIZE> *v; // used sorted for retrieval.
  std::set<StronglyConnectedConfiguration<ELEMENT_SIZE> > s; // only used for construction.
  std::map<unsigned int,StronglyConnectedConfigurationHashList<ELEMENT_SIZE> > hashLists;

  void serialize(std::ofstream &os) {
    unsigned long size = s.size();
    os.write(reinterpret_cast<const char *>(&size), sizeof(unsigned long));

    typename std::set<StronglyConnectedConfiguration<ELEMENT_SIZE> >::const_iterator it;
    for(it = s.begin(); it != s.end(); ++it) {
      it->serialize(os);
    }
  }
  void deserialize(std::ifstream &is) {
    unsigned long size;
    is.read((char*)&size, sizeof(unsigned long));
    v = new StronglyConnectedConfiguration<ELEMENT_SIZE>[size];
    for(int i = 0; i < size; ++i) {
      v[i].deserialize(is);
    }
  }

  void getAllNewBricks(const StronglyConnectedConfiguration<ELEMENT_SIZE-1> &smaller, std::set<RectilinearBrick> &newBricks) {
    // Add for all bricks in smaller:
    int tmpSize = 0;
    RectilinearBrick tmp[STRONGLY_CONNECTED_BRICK_POSITIONS];

    // Handle origin as special brick (because it doesn't exist in otherBricks):
    RectilinearBrick b;
    b.constructAllStronglyConnected(tmp, tmpSize);
    assert (tmpSize <= STRONGLY_CONNECTED_BRICK_POSITIONS);
    //std::cout << "addAllFor(" << smaller << ")" << std::endl;
    for(int j = 0; j < tmpSize; ++j) {
      b = tmp[j];
      if(!smaller.intersects(b))
        newBricks.insert(RectilinearBrick(b));
    }

    // Handle normal bricks:
    if(ELEMENT_SIZE > 1) {
      for(int i = 0; i < ELEMENT_SIZE-2; ++i) {
        tmpSize = 0;
        smaller.otherBricks[i].constructAllStronglyConnected(tmp, tmpSize);
        assert (tmpSize <= STRONGLY_CONNECTED_BRICK_POSITIONS);

        for(int j = 0; j < tmpSize; ++j) {
          b = tmp[j];
          if(!smaller.intersects(b))
            newBricks.insert(RectilinearBrick(b));
        }
      }
    }
  }

  /*
  Add all scc's of this size from the scc c of a size smaller:
  1) Have a small set s in which to put all possible bricks that can connect strongly to c
  1a) Ensure no intersection with c when doing this!
  2) Add all x from s to c (and sort to c') to self. First rotate c' to check for duplicates. 
  */
  void addAllFor(const StronglyConnectedConfiguration<ELEMENT_SIZE-1> &smaller) {
    std::set<RectilinearBrick> newBricks;
    getAllNewBricks(smaller, newBricks);

    // Try all new scc's:
    //std::cout << " Found " << newBricks.size() << " potential new scc's" << std::endl;
    for(std::set<RectilinearBrick>::const_iterator it = newBricks.begin(); it != newBricks.end(); ++it) {
      StronglyConnectedConfiguration<ELEMENT_SIZE> candidate(smaller,*it);
      //std::cout << " Trying candidate " << candidate << ", can turn 90: " << candidate.canTurn90() << std::endl;

      if(s.find(candidate) != s.end()) {
        continue;
      }
      if(candidate.canTurn90()) {
        for(int i = 0; i < 3; ++i) {
          candidate.turn90();
          if(s.find(candidate) != s.end()) {
            continue;
          }
        }
      }
      else {
        candidate.turn180();
        if(s.find(candidate) != s.end()) {
          continue;	
        }
      }
      s.insert(candidate);
    }
  }

  void addAllFor(StronglyConnectedConfigurationList<ELEMENT_SIZE-1> &smaller) {
    typename std::set<StronglyConnectedConfiguration<ELEMENT_SIZE-1> >::const_iterator it;
    for(it = smaller.s.begin(); it != smaller.s.end(); ++it) {
      addAllFor(*it);
    }
  }

  void countAllFor(const StronglyConnectedConfiguration<ELEMENT_SIZE-1> &smaller) {
    std::set<RectilinearBrick> newBricks;
    getAllNewBricks(smaller, newBricks);

    // Try all new scc's:
    for(std::set<RectilinearBrick>::const_iterator it = newBricks.begin(); it != newBricks.end(); ++it) {
      StronglyConnectedConfiguration<ELEMENT_SIZE> candidate(smaller,*it);
      unsigned int hash = candidate.layerHash();

      // ensure existence:
      if(hashLists.find(hash) == hashLists.end()) {
	StronglyConnectedConfigurationHashList<ELEMENT_SIZE> newList(hash);
	newList.open();
	hashLists.insert(std::make_pair<unsigned int,StronglyConnectedConfigurationHashList<ELEMENT_SIZE> >(hash, newList));
      }

      hashLists.find(hash)->second.output(candidate);
    }
  }

  unsigned long countAllFor(StronglyConnectedConfigurationList<ELEMENT_SIZE-1> &smaller) {
    typename std::set<StronglyConnectedConfiguration<ELEMENT_SIZE-1> >::const_iterator it;
    for(it = smaller.s.begin(); it != smaller.s.end(); ++it) {
      countAllFor(*it);
    }

    // Now do the counting:
    unsigned long sum = 0;
    typename std::map<unsigned int,StronglyConnectedConfigurationHashList<ELEMENT_SIZE> >::iterator it2;
    for(it2 = hashLists.begin(); it2 != hashLists.end(); ++it2) {
      sum += it2->second.count();
    }
    std::cout << "Number of StronglyConnectedConfigurations of size " << ELEMENT_SIZE << ": " << sum << std::endl;
    return sum;
  }

  void printLDRFile() {
    LDRPrinterHandler h;

    typename std::set<StronglyConnectedConfiguration<ELEMENT_SIZE> >::const_iterator it = s.begin();
    for(int i = 0; it != s.end(); ++it, ++i) {
      LDRPrinter const * p = &(*it);
      h.add(p);
    }

    std::stringstream ss;
    ss << "StronglyConnectedConfigurationOfSize" << ELEMENT_SIZE;
    h.print(ss.str());
  }
};

template <unsigned int ELEMENT_SIZE>
std::ostream& operator<<(std::ostream& os, const StronglyConnectedConfigurationList<ELEMENT_SIZE>& l) {
  os << "Combinations of size " << ELEMENT_SIZE << ": " << l.s.size() << std::endl;
  /*
  typename std::set<StronglyConnectedConfiguration<ELEMENT_SIZE> >::const_iterator it = l.s.begin();
  for(int i = 0; it != l.s.end(); ++it, ++i) {
    os << " " << *it << std::endl;
    if(i > 10) {
      os << "..." << std::endl;
      break;
    }      
  }//*/
  return os;
};

#endif
