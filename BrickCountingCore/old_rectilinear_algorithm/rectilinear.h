#ifndef BRICKS_RECTILINEAR_H
#define BRICKS_RECTILINEAR_H

// DIFFLT = Difference less than is used to check for brick intersection
#define DIFFLT(a,b,c) ((a) < (b) ? ((b)-(a)<(c)) : ((a)-(b)<(c)))

// Goal of 2025 is to construct models with at most 11 bricks
#define MAX_BRICKS 11
// For up to 11 bricks, only heights up to 6 are non-trivially computed
#define MAX_HEIGHT 6
// At most 9 bricks can be in a single layer if we consider 11 to be maximal number of bricks
#define MAX_LAYER_SIZE 9

#include "stdint.h"
#include <stdarg.h>
#include <iostream>
#include <fstream>
#include <set>
#include <map>
#include <mutex>

namespace rectilinear {

  /**
   * Struct used for totalling the number of models.
   * all includes the models counted for symmetric180 and symmetric90.
   * symmetric90 is 0 unless counting for <44>. In this case symmetric180
   * also includes the models counted for symmetric90.
   */
  struct Counts {
    uint64_t all, symmetric180, symmetric90;

    Counts();
    Counts(uint64_t all, uint64_t symmetric180, uint64_t symmetric90);
    Counts(const Counts& c);
    Counts& operator +=(const Counts &c);
    Counts operator -(const Counts &c);
    friend std::ostream& operator <<(std::ostream &os, const Counts &c);
    void reset(); // Sets counts to 0.
  };

  /**
   * A Brick represents a 2x4 LEGO brick with an orientation and x,y-position.
   * x,y is the position of the middle of the brick (like in LDRAW).
   */
  struct Brick {
    bool isVertical:1; // :1 gives the compiler the hint that only 1 bit is needed.
    // Could use :6 when 6 bricks is maximum size, but it would not provide practical benefits,
    // as 7+7+1 still packs in 2 bytes:
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
    uint64_t encode15() const; // Encode the brick into the lowest 15 bits.
  };

  const Brick FirstBrick = Brick(); // At 0,0, horizontal

  class Combination {
  private:
    // State to check connectivity:
    bool connected[MAX_HEIGHT][MAX_LAYER_SIZE];
  public:
    int layerSizes[MAX_HEIGHT], height;
    Brick bricks[MAX_HEIGHT][MAX_LAYER_SIZE];

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
    void stack(const Combination &top, Combination &ret) const;
    static int heightOfToken(int token);
    static int sizeOfToken(int token);
    static void getLayerSizesFromToken(int token, int *layerSizes);
    static int getTokenFromLayerSizes(int *layerSizes, int height);
    static int reverseToken(int token);

    // Helper methods for countX2:
    uint64_t encodeFor12(bool &topSymmetric, bool &topConnected) const;
    uint64_t countSymmetricLayer0Siblings() const;
    uint64_t countSymmetricLayer0SiblingsThatConnectAbove() const;
  };

  class CutCombination {
  public:
    int layerSizes[2]; // height is 2
    Brick bricks[2][8]; // At most 8 bricks per layer (Used for layer1Size >= 3, and max total size is 11)
    uint64_t connectivity;

    CutCombination(const Combination &c);
    bool operator <(const CutCombination& b) const;
    bool operator ==(const CutCombination& b) const;
  private:
    static void colorGraph(int color, int layer, int idx, int colors[MAX_HEIGHT][MAX_LAYER_SIZE], const Combination &c);
  };

  class ICombinationProducer {
  public:
    //virtual ~ICombinationProducer();
    virtual bool nextCombination(Combination &c) = 0;
    virtual bool hasNextCombination() = 0;
    static ICombinationProducer* get(int token);
  };

  class CombinationReader final : public ICombinationProducer { // 'final' to avoid delete called on derived classes.
  private:
    std::ifstream *istream; // Reads reversed combinations
    int layerSizes[MAX_HEIGHT], // Reversed
      height, combinationsLeft, Z, token;
    uint8_t bits, bitIdx, brickLayer;
    Combination baseCombination; // Reversed!
    bool reverse, done, invalid;
    uint64_t combinationCounter;
    std::string name;
    std::mutex read_mutex;

    bool readBit();
    int8_t readInt8();
    uint8_t readUInt4();
    uint32_t readUInt32();
    Brick readBrick();
    void ensureCombinationsLeft();
  public:
    CombinationReader(const int layerSizes[], int Z);
    ~CombinationReader();
    bool nextCombination(Combination &c);
    bool hasNextCombination();
    bool isInvalid() const;
  };

  class SingleBrickAdder : public ICombinationProducer {
    const int Z, height, token;
    Counts counts;
    int layer;
    ICombinationProducer *smallerProducer;
    std::stack<Combination> toProduce;
    std::mutex read_mutex;

    bool ensureSmallerProducer();
    void ensureToProduce();

  public:
    SingleBrickAdder(const int token);

    bool nextCombination(Combination &c);
    bool hasNextCombination();
    static Counts add(Combination &cOld, const Brick &addedBrick, const uint8_t layer);
    static Counts addBricksToCombination(Combination &c, const int layer, std::vector<Brick> &v);
  };

  class SpindleBuilder final : public ICombinationProducer {
    std::vector<Combination> cache;
    std::vector<bool> cacheSymmetric;
    int cacheIndex;
    bool cacheIsLower, produced;
    ICombinationProducer *smallerProducer;
    Combination smaller;
    bool smallerSymmetric;
    std::mutex read_mutex;

  public:
    SpindleBuilder(int token);
    bool nextCombination(Combination &c);
    bool hasNextCombination();
    static bool canHandle(int token);
    static void splitTokenToTokens(int *layerSizes, int height, int splitLayer, int &lower, int &upper);
    static void setup(const int token, int &height, int &Z, int *layerSizes, std::vector<int> &candidates);
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

    bool writesToFile() const;
    void fillFromReader(int const * const layerSizes, const int reducedLayer, ICombinationProducer *reader);
  private:
    void writeBit(bool bit);
    void flushBits();
    void writeInt8(const int8_t toWrite);
    void writeUInt4(const uint8_t toWrite);
    void writeUInt32(uint32_t toWrite);
    void writeBrick(const Brick &b);
    void writeCombinations(const Combination &baseCombination, uint8_t brickLayer, std::vector<Brick> &v);
  };

  class Counter {
    std::map<int,Counts> cache;

    void writeToCache(int token, Counts counts);
    Counts fastRunToWriter(CombinationWriter &writer);
    Counts buildCombinations(int token, int remaining, bool add_self, bool saveOutput);

    // Helper methods for countX2():
    Counts countLayer0P(int Z, int layer0Size, int layer1Size, Combination &symmetryChecker, int idx, std::set<Brick>::const_iterator itBegin, std::set<Brick>::const_iterator itEnd, const uint64_t divisor, bool topSymmetric, bool topConnected, std::set<Combination> &seen);
    Counts countLayer0Placements(int Z, int layer0Size, int layer1Size, Combination &c, const uint64_t divisor, bool topSymmetric, bool topConnected, std::set<Combination> &seen);
    Counts countX2(int Z, int layer0Size, char* input);

  public:
    Counts countRefinements(const int Z, int token, const int height, char* input, const bool saveOutput);
    void buildAllCombinations(int Z, bool saveOutput);
  };
}

#endif
