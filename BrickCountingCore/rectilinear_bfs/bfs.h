#ifndef BFS_H
#define BFS_H

// DIFFLT = Difference less than is used to check for brick intersection
#define DIFFLT(a,b,c) ((a) < (b) ? ((b)-(a)<(c)) : ((a)-(b)<(c)))

// Goal of 2025 is to construct models with at most 11 bricks
#define MAX_BRICKS 8
// At most 9 bricks can be in a single layer if we consider 11 to be maximal number of bricks
#define MAX_LAYER_SIZE 7
#define BRICK first
#define LAYER second

#include "stdint.h"
#include <stdarg.h>
#include <iostream>
#include <fstream>
#include <map>
#include <mutex>

#ifdef PROFILING
typedef std::pair<uint64_t,std::string> InvocationPair;
typedef std::map<std::string,uint64_t> InvocationMap;
extern InvocationMap *invocationCounts;
struct Profiler {
  static void countInvocation(const std::string &s);
  static void reportInvocations();
};
#endif
  
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
    bool intersects(const Brick &b) const;
    void mirror(Brick &b, const int8_t &cx, const int8_t &cy) const;
    bool mirrorEq(const Brick &b, const int8_t &cx, const int8_t &cy) const;
  };

  typedef std::pair<Brick,uint8_t> LayerBrick;
  
  const Brick FirstBrick = Brick(); // At 0,0, horizontal

  typedef std::pair<uint8_t,uint8_t> BrickIdentifier; // layer, idx
  typedef std::map<int,Counts> CountsMap;

  class BrickPicker {
    const std::vector<LayerBrick> &v;
    int vIdx;
    const int numberOfBricksToPick, bricksIdx;
    LayerBrick *bricks;
    BrickPicker *inner;

    bool checkVIdx() const;
    void nextVIdx();
    
  public:
    BrickPicker(const std::vector<LayerBrick> &v, int vIdx, const int numberOfBricksToPick, LayerBrick *bricks, const int bricksIdx);
     ~BrickPicker();

    bool next();
  };
  
  class Combination {
  public:
    uint8_t layerSizes[MAX_BRICKS], height, size;
    Brick bricks[MAX_BRICKS][MAX_LAYER_SIZE];
    BrickIdentifier history[MAX_BRICKS];

    /*
      Rectilinear models with restrictions on representation:
      - first brick must be vertical at first layer and placed at 0,0
      - model is minimal of all rotations (vs rotated 90, 180, 270 degrees)
      - bricks are lexicographically sorted (orientation,x,y) on each layer
      Init with one brick on layer 0 at 0,0.
    */
    Combination();
    Combination(const Combination &b);

    bool operator ==(const Combination& b) const;
    friend std::ostream& operator << (std::ostream &os, const Combination &b);

    void copy(const Combination &b);
    void rotate90();

    void getLayerCenter(const uint8_t layer, int8_t &cx, int8_t &cy) const;
    bool isLayerSymmetric(const uint8_t layer, const int8_t &cx, const int8_t &cy) const;
    bool is180Symmetric() const;
    bool is90Symmetric() const;
    void sortBricks();
    void translateMinToOrigo();
    void addBrick(const Brick &b, const uint8_t layer);
    void removeLastBrick();
    int getTokenFromLayerSizes() const;
    static int reverseToken(int token);
    static uint8_t heightOfToken(int token);
    static uint8_t sizeOfToken(int token);
    static void getLayerSizesFromToken(int token, uint8_t *layerSizes);
  };

  class CombinationBuilder {
  public:
    Combination &baseCombination;
    const uint8_t waveStart, waveSize, maxSize;
    CountsMap counts; // token -> counts

    CombinationBuilder(Combination &c, const uint8_t waveStart, const uint8_t waveSize, const uint8_t maxSize);

    void build();
    void report();
  private:
    void fast();
  };

}

#endif
