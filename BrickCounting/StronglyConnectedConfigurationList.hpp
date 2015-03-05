#ifndef STRONGLY_CONNECTED_CONFIGURATION_LIST_HPP
#define STRONGLY_CONNECTED_CONFIGURATION_LIST_HPP

#include <set>
#include <iostream>
#include <fstream>
#include "StronglyConnectedConfiguration.hpp"
#include <assert.h> 

template <unsigned int ELEMENT_SIZE>
class StronglyConnectedConfigurationList {
public:
  StronglyConnectedConfiguration<ELEMENT_SIZE> *v; // used sorted for retrieval.
  std::set<StronglyConnectedConfiguration<ELEMENT_SIZE> > s; // only used for construction.

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

  /*
  Add all scc's of this size from the scc c of a size smaller:
  1) Have a small set s in which to put all possible bricks that can connect strongly to c
  1a) Ensure no intersection with c when doing this!
  2) Add all x from s to c (and sort to c') to self. First rotate c' to check for duplicates. 
  */
  void addAllFor(const StronglyConnectedConfiguration<ELEMENT_SIZE-1> &smaller) {
    std::cout << "addAllFor init" << std::endl;
    std::set<RectilinearBrick> newBricks;

    // Add for all bricks in smaller:
    int tmpSize = 0;
    RectilinearBrick tmp[STRONGLY_CONNECTED_BRICK_POSITIONS];

    // Handle origin as special brick (because it doesn't exist in otherBricks):
    RectilinearBrick b;
    b.constructAllStronglyConnected(tmp, tmpSize);
    assert (tmpSize <= STRONGLY_CONNECTED_BRICK_POSITIONS);
    std::cout << "addAllFor(" << smaller << ")" << std::endl;
    assert(smaller.verify());
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

    // Try all new scc's:
    std::cout << " Found " << newBricks.size() << " potential new scc's" << std::endl;
    for(std::set<RectilinearBrick>::const_iterator it = newBricks.begin(); it != newBricks.end(); ++it) {
      StronglyConnectedConfiguration<ELEMENT_SIZE> candidate(smaller,*it);
      std::cout << " Trying candidate " << candidate << std::endl;
      if(s.find(candidate) != s.end()) {
        //std::cout << "Already known! " << candidate << std::endl;
        continue;
      }
      if(candidate.canTurn90()) {
        for(int i = 0; i < 3; ++i) {
          candidate.turn90();
          if(s.find(candidate) != s.end()) {
            //std::cout << "Already known as " << candidate << std::endl;
            continue;
          }
        }
      }
      else {
        candidate.turn180();
        if(s.find(candidate) != s.end()) {
          //std::cout << "Already known 180 as " << candidate << std::endl;
          continue;	
        }
      }
      std::cout << " Yay! Inserting " << candidate << std::endl;
      assert(candidate.verify());
      s.insert(candidate);
    }

    std::cout << "Check set before destruction:" << std::endl;
    RectilinearBrick prev(0, 0, -1, false);
    for(std::set<RectilinearBrick>::const_iterator it = newBricks.begin(); it != newBricks.end(); ++it) {
      assert(prev < *it);
      prev = *it;
    }
    newBricks.clear();
    std::cout << "Done addForAll." << std::endl;
  }

  void addAllFor(StronglyConnectedConfigurationList<ELEMENT_SIZE-1> &smaller) {
    std::cout << "ADDALLFOR init on smaller of size " << smaller.s.size() << " Content: " << std::endl;
    typename std::set<StronglyConnectedConfiguration<ELEMENT_SIZE-1> >::const_iterator it;
    for(it = smaller.s.begin(); it != smaller.s.end(); ++it) {
      std::cout << " " << *it << std::endl;
    }
    std::cout << "ADDALLFOR GOGOGO! " << std::endl;

    for(it = smaller.s.begin(); it != smaller.s.end(); ++it) {
      std::cout << "ADDALLFOR for smaller " << *it << std::endl;
      addAllFor(*it);
    }
    std::cout << "ADDALLFOR end" << std::endl;
  }
};

template <unsigned int ELEMENT_SIZE>
std::ostream& operator<<(std::ostream& os, const StronglyConnectedConfigurationList<ELEMENT_SIZE>& l) {
  os << "Combinations of size " << ELEMENT_SIZE << ": " << l.s.size() << std::endl;

  typename std::set<StronglyConnectedConfiguration<ELEMENT_SIZE> >::const_iterator it = l.s.begin();
  for(int i = 0; it != l.s.end(); ++it, ++i) {
    os << " " << *it << std::endl;
    if(i > 10) {
      os << "..." << std::endl;
      break;
    }      
  }
  return os;
};

#endif
