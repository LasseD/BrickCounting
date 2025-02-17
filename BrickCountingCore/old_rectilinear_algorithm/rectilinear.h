#ifndef BRICKS_RECTILINEAR_H
#define BRICKS_RECTILINEAR_H

#define ABS(a) ((a) < 0 ? -(a) : (a))
#define MAX(a,b) ((a) < (b) ? (b) : (a))
#define DIFF(a,b) ((a) < (b) ? (b)-(a) : (a)-(b))

#define MAX_BRICKS 11
#define MAX_HEIGHT 6
#define MAX_LAYER_SIZE 9

#include "stdint.h"
#include <stdarg.h>
#include <iostream>
#include <fstream>
#include <set>
#include <mutex>

namespace rectilinear {

  struct Counts {
    uint64_t all, symmetric180, symmetric90;

    Counts();
    Counts(uint64_t all, uint64_t symmetric180, uint64_t symmetric90);
    Counts(const Counts& c);
    Counts& operator +=(const Counts &c);
    Counts operator -(const Counts &c);
    friend std::ostream& operator <<(std::ostream &os, const Counts &c);
    void reset();
  };

  /**
   * A Brick represents a 2x4 LEGO brick with an orientation and x,y-position.
   * x,y is the position of the middle of the brick (like in LDRAW).
   */
  struct Brick {
    bool is_vertical:1;
    // Could use :6 when 6 bricks is maximum size, but it would not provide practical benefits,
    // as 7+7+1 still packs in 2 bytes.
    int8_t x:7, y:7;

    Brick();
    Brick(bool iv, int8_t x, int8_t y);
    Brick(const Brick &b);

    bool operator <(const Brick& b) const;
    bool operator ==(const Brick& b) const;
    bool operator !=(const Brick& b) const;
    friend std::ostream& operator <<(std::ostream &os, const Brick &b);
    int cmp(const Brick& b) const;
    bool intersects(const Brick &b) const;
    uint64_t encode15() const;
  };

  const Brick FirstBrick = Brick(); // At 0,0, horizontal

  class Combination {
  private:
    // State to check connectivity:
    bool connected[MAX_HEIGHT][MAX_LAYER_SIZE];
  public:
    int layerSizes[MAX_HEIGHT], height; // Combination with Z bricks
    Brick bricks[MAX_HEIGHT][MAX_LAYER_SIZE]; // Layer, idx

    /*
      Rectilinear models with restrictions on representation:
      - first brick must be vertical at first layer and placed at 0,0
      - model is minimal of all rotations (vs rotated 90, 180, 270 degrees)
      - bricks are lexicographically sorted (orientation,x,y) on each layer
      Init with one brick on layer 0 at 0,0.
    */
    Combination();
    Combination(const Combination &b);

    bool operator <(const Combination& b) const;
    bool operator ==(const Combination& b) const;
    friend std::ostream& operator << (std::ostream &os, const Combination &b);

    void copy(const Combination &b);
    bool isValid(int token, const std::string s);
    void translateMinToOrigo();
    void sortBricks();
    void rotate90();
    void rotate180();
    bool is180Symmetric() const;
    bool isSymmetricAboveFirstLayer() const;
    bool isConnectedAboveFirstLayer() const;
    bool can_rotate90() const;
    bool addBrick(const Brick &b, const uint8_t layer, Combination &out, int &rotated) const;
    void removeSingleLowerBrick(Combination &out) const;				
    void removeSingleTopBrick(Combination &out) const;
    bool removeBrickAt(int layer, int idx, Combination &out) const;		
    void normalize(int &rotated);
    void normalize(); 
    int countConnected(int layer, int idx);
    bool isConnected();
    void flip();
    uint64_t encodeFor12(bool &topSymmetric, bool &topConnected) const;
    uint64_t encodeForYY(bool &topSymmetric, bool &topConnected) const;
    uint64_t countSymmetricLayer0Siblings() const;
    uint64_t countSymmetricLayer0SiblingsThatConnectAbove() const;
  };

  class CombinationReader {
  private:
    std::ifstream *istream; // Reads reversed combinations
    int layerSizes[MAX_HEIGHT], // Reversed
      height, combinationsLeft, Z, token;
    uint8_t bits, bitIdx, brickLayer;
    Combination baseCombination; // Reversed!
    bool reverse, done;
    uint64_t combinationCounter;
    std::string name;
    std::mutex read_mutex;

    bool readBit();
    int8_t readInt8();
    uint8_t readUInt4();		
    uint32_t readUInt32();		
    Brick readBrick();
  public:
    CombinationReader(const int layerSizes[], int Z);
    ~CombinationReader();
    bool readCombination(Combination &c);
  };

  class CombinationWriter {
    std::ofstream *ostream;
    int height; // eg. 211 for 4 bricks in config 2-1-1
    std::set<Combination> combinations; // Only used by assertion!
    uint8_t bits, cntBits;
    uint64_t writtenFull, writtenShort;

  public:
    const int token;
    Counts counts;
    int Z;

    CombinationWriter(const int token, const bool saveOutput);
    CombinationWriter(const CombinationWriter &cw);
    ~CombinationWriter();

    void writeBit(bool bit);
    void flushBits();
    void writeInt8(const int8_t toWrite);
    void writeUInt4(const uint8_t toWrite);
    void writeUInt32(uint32_t toWrite);
    void writeBrick(const Brick &b);
    void writeCombinations(const Combination &baseCombination, uint8_t brickLayer, std::vector<Brick> &v);
    Counts add(Combination &cOld, const Brick &addedBrick, const uint8_t layer);
    void makeNewCombinations(Combination &c, const int layer);
    void fillFromReader(int const * const layerSizes, const int reducedLayer, CombinationReader * reader);
    bool writesToFile() const;
  };

  class Counter {
    Counts counts;

    void fillFromReaders(CombinationWriter &writer);
    void handleFinalCombinationWriters(const int token, const bool saveOutput);
    void handleCombinationWriters(int token, int remaining, bool add_self, bool saveOutput);
    void countLayer0P(int Z, int Y, int layer0Size, Combination &symmetryChecker, int idx, std::set<Brick>::const_iterator itBegin, std::set<Brick>::const_iterator itEnd, uint64_t &cnt, uint64_t &cntSymmetric, const uint64_t divisor, bool topSymmetric, bool topConnected, std::set<Combination> &seen);
    void countLayer0Placements(int Z, int Y, int layer0Size, Combination &c, uint64_t &cnt, uint64_t &cntSymmetric, const uint64_t divisor, bool topSymmetric, bool topConnected, std::set<Combination> &seen);
    void countX2(int Z, int layer0Size, char* input);
    void countXY(int Z, int Y, int layer0Size, char* input);
  public:
    void countRefinements(const int Z, int token, const int height, char* input, const bool saveOutput);
    void buildAllCombinations(int Z, bool saveOutput);
  };
}

#endif
