#include "stdint.h"
#include <iostream>
#include <fstream>
#include <map>
#include <algorithm>
#include <assert.h>
#include <chrono>
#include <thread>
#include <sstream>

#include "rectilinear.h"

namespace rectilinear {

  Counts::Counts() : all(0), symmetric180(0), symmetric90(0) {
  }
  Counts::Counts(uint64_t all, uint64_t symmetric180, uint64_t symmetric90) : all(all), symmetric180(symmetric180), symmetric90(symmetric90) {
  }
  Counts::Counts(const Counts& c) : all(c.all), symmetric180(c.symmetric180), symmetric90(c.symmetric90) {
  }
  Counts& Counts::operator +=(const Counts& c) {
    all += c.all;
    symmetric180 += c.symmetric180;
    symmetric90 += c.symmetric90;
    return *this;
  }
  Counts Counts::operator -(const Counts& c) {
    return Counts(all-c.all, symmetric180-c.symmetric180, symmetric90-c.symmetric90);
  }
  std::ostream& operator << (std::ostream &os,const Counts &c) {
    os << c.all;
    if(c.symmetric180 > 0)
      os << " (" << c.symmetric180 << ")";
    if(c.symmetric90 > 0)
      os << " {" << c.symmetric90 << "}";
    return os;
  }
  void Counts::reset() {
    all = 0;
    symmetric180 = 0;
    symmetric90 = 0;
  }

  Brick::Brick() : isVertical(true), x(0), y(0) {
  }
  Brick::Brick(bool iv, int8_t x, int8_t y) : isVertical(iv), x(x), y(y) {	
  }
  Brick::Brick(const Brick &b) : isVertical(b.isVertical), x(b.x), y(b.y) {
  }

  bool Brick::operator <(const Brick& b) const {
    return cmp(b) < 0;
  }
  bool Brick::operator ==(const Brick& b) const {
    return x == b.x && y == b.y && isVertical == b.isVertical;
  }
  bool Brick::operator !=(const Brick& b) const {
    return !(*this == b);
  }
  std::ostream& operator << (std::ostream &os,const Brick &b) {
    os << (b.isVertical?"|":"=") << (int)b.x << "," << (int)b.y << (b.isVertical?"|":"=");
    return os;
  }
  int Brick::cmp(const Brick& b) const {
    if(isVertical != b.isVertical)
      return - isVertical + b.isVertical;
    if(x != b.x)
      return x - b.x;
    return y - b.y;	
  }
  bool Brick::intersects(const Brick &b) const {
    if(isVertical != b.isVertical)
      return DIFFLT(b.x, x, 3) && DIFFLT(b.y, y, 3);
    if(isVertical)
      return DIFFLT(b.x, x, 2) && DIFFLT(b.y, y, 4);
    else
      return DIFFLT(b.x, x, 4) && DIFFLT(b.y, y, 2);
  }
  uint64_t Brick::encode15() const {
    uint64_t X = x+30;
    uint64_t Y = y+30;
    return (((X << 7) | Y) << 1) | isVertical;
  }

  Combination::Combination() {
    bricks[0][0] = FirstBrick;
    layerSizes[0] = 1;
    height = 1;
  }
  Combination::Combination(const Combination &b) {
    copy(b);
  }

  bool Combination::operator <(const Combination& b) const {
    assert(height == b.height);
    for(int i = 0; i < height; i++) {
      assert(layerSizes[i] == b.layerSizes[i]);
      int s = layerSizes[i];
      for(int j = 0; j < s; j++) {
	int res = bricks[i][j].cmp(b.bricks[i][j]);
	if(res != 0) {
	  return res < 0;
	}
      }   
    }
    return false;
  }

  bool Combination::operator ==(const Combination& b) const {
    assert(height == b.height);
    for(int i = 0; i < height; i++) {
      assert(layerSizes[i] == b.layerSizes[i]);
      int s = layerSizes[i];
      for(int j = 0; j < s; j++) {
	int res = bricks[i][j].cmp(b.bricks[i][j]);
	if(res != 0) {
	  return false;
	}
      }   
    }
    return true;
  }

  std::ostream& operator << (std::ostream &os, const Combination &b) {
    os << "<";
    for(int i = 0; i < b.height; i++) {
      os << b.layerSizes[i];
    }
    os << "> combination:";
    for(int i = 0; i < b.height; i++) {
      for(int j = 0; j < b.layerSizes[i]; j++) {
	os << " " << b.bricks[i][j] << " ";
      }
    }
    return os;
  }

  void Combination::copy(const Combination &b) {
    assert(b.height > 0);
    height = b.height;
    for(int i = 0; i < height; i++) {
      int s = b.layerSizes[i];
      layerSizes[i] = s;
      for(int j = 0; j < s; j++) {
	bricks[i][j] = b.bricks[i][j];
      }
    }
  }

  bool Combination::isValid(const int token, const std::string s) {
    // Verify against token:
    int tkn = token;
    for(int i = height-1; i >= 0; i--) {
      int tokenLayer = tkn%10;
      tkn/=10;
      if(tokenLayer != layerSizes[i]) {
	std::cerr << "Validation error: Layer " << i << " (layer size from token: " << tokenLayer << ") in " << *this << " does not match token '" << token << "'" << std::endl;
	return false;						
      }
    }
		
    if(bricks[0][0] != FirstBrick) {
      std::cerr << "Error: First brick not |0,0|: " << *this << " at " << s << std::endl;
      return false;
    }
    // Check ordering of bricks:
    for(int layer = 0; layer < height; layer++) {
      int s = layerSizes[layer];
      for(int i = 0; i+1 < s; i++) {
	if(!(bricks[layer][i] < bricks[layer][i+1])) {
	  std::cerr << "Error: Invalid " << s << " combination: " << *this << " at layer " << layer << " for bricks " << i << " and " << (i+1) << std::endl;
	  return false;
	}
      }
    }
    // Check for overlaps:
    for(int layer = 0; layer < height; layer++) {
      int s = layerSizes[layer];
      for(int i = 0; i+1 < s; i++) {
	const Brick &bi = bricks[layer][i];
	for(int j = i+1; j < s; j++) {
	  const Brick &bj = bricks[layer][j];
	  if(bi.intersects(bj)) {
	    std::cout << "Bricks " << i << " and " << j << " of layer " << layer << " intersect in " << *this << std::endl;
	    return false;
	  }
	}
      }
    }
    // Check connectivity:
    if(!isConnected()) {
      std::cerr << "Not connected! " << *this << std::endl;
      return false;
    }
    return true;
  }

  void Combination::translateMinToOrigo() {
    int8_t minx = 127, miny = 127;

    for(int i = 0; i < layerSizes[0]; i++) {
      Brick &b = bricks[0][i];
      if(b.isVertical) {
	// Vertical bricks in layer 0 can be 'min':
	if(b.x < minx || (b.x == minx && b.y < miny)) {
	  minx = b.x;
	  miny = b.y;
	}
      }
    }
    // Move all to new min:
    for(int i = 0; i < height; i++) {
      for(int j = 0; j < layerSizes[i]; j++) {
	bricks[i][j].x -= minx;
	bricks[i][j].y -= miny;
      }
    }
  }

  void Combination::sortBricks() {
    for(int layer = 0; layer < height; layer++) {
      int layerSize = layerSizes[layer];
      if(layerSize > 1) {
	std::sort(bricks[layer], &bricks[layer][layerSize]);
      }
    }
  }

  void Combination::rotate90() {
    for(int i = 0; i < height; i++) {
      for(int j = 0; j < layerSizes[i]; j++) {
	const Brick &b = bricks[i][j];
	bricks[i][j] = Brick(!b.isVertical, b.y, -b.x);
      }
    }
    translateMinToOrigo();
    sortBricks();

  }

  void Combination::rotate180() {
    // Perform rotation:
    for(int i = 0; i < height; i++) {
      for(int j = 0; j < layerSizes[i]; j++) {
	Brick &b = bricks[i][j];
	b.x = -b.x;
	b.y = -b.y;
      }
    }
    translateMinToOrigo();
    sortBricks(); // TODO: Is std::reverse fast enough?
  }

  // TODO: Optimize by:
  // 1) Finding center of first layer.
  // 2) For each layer:
  //  - Check same center as first. If not, return false.
  //  - If a brick is at center, disregard it.
  //  - If odd number of remaining bricks: return false.
  //  - All bricks must have a corresponding brick across the center.
  // 3) return true
  bool Combination::is180Symmetric() const {
    Combination c(*this);
    c.rotate180();
    return *this == c;
  }

  bool Combination::isSymmetricAboveFirstLayer() const {
    Combination c;
    c.height = height-1;
    for(int i = 1; i < height; i++) {
      c.layerSizes[i-1] = layerSizes[i];
      for(int j = 0; j < layerSizes[i]; j++) {
	c.bricks[i-1][j] = bricks[i][j];
      }
    }
    c.normalize();
    Combination c2(c);
    c2.rotate180();
    return c == c2;
  }

  bool Combination::isConnectedAboveFirstLayer() const {
    Combination c;
    c.height = height-1;
    for(int i = 1; i < height; i++) {
      c.layerSizes[i-1] = layerSizes[i];
      for(int j = 0; j < layerSizes[i]; j++) {
	c.bricks[i-1][j] = bricks[i][j];
      }
    }
    //c.normalize(); // TODO: Needed?
    return c.isConnected();
  }

  bool Combination::can_rotate90() const {
    for(int i = 0; i < layerSizes[0]; i++) {
      if(!bricks[0][i].isVertical) {
	return true;
      }
    }
    return false;
  }

  /**
   * Copy this combination and add b at layer.
   * Assume the brick is connected to one of the existing bricks.
   */
  bool Combination::addBrick(const Brick &b, const uint8_t layer, Combination &out, int &rotated) const {
    int layerSize = layer == height ? 0 : layerSizes[layer];
    for(int i = 0; i < layerSize; i++) {
      const Brick &b2 = bricks[layer][i];
      if(b2.intersects(b)) {
	return false; // Intersects!
      }
    }

    // Set up out-configuration:
    out.height = height;
    for(int i = 0; i < height; i++) {
      int s = layerSizes[i];
      if(i == layer) {
	out.layerSizes[i] = s+1;
	continue; // Handle this layer separately
      }
      out.layerSizes[i] = s;
      for(int j = 0; j < s; j++) {
	out.bricks[i][j] = bricks[i][j];
      }
    }
    if(height == layer) {
      out.height = height + 1;
      out.layerSizes[height] = 1;
    }
				
    // For layer, add by finding first brick not smaller than b (simple merge):
    int in = 0;
    for(int i = 0; i < layerSize; i++) {
      if(in == 0) {
	const Brick &b2 = bricks[layer][i];
	if(b < b2) { // Merge in:
	  in = 1;
	  out.bricks[layer][i] = b;
	}
      }
      out.bricks[layer][i+in] = bricks[layer][i];
    }
    // Finish up:
    if(in == 0) {
      out.bricks[layer][layerSize] = b; // Add to the end
    }

    out.normalize(rotated);
    return true;
  }

  void Combination::removeSingleLowerBrick(Combination &out) const {
    out.height = height-1;
    for(int layer = 0; layer+1 < height; layer++) {
      int s = layerSizes[layer+1];
      out.layerSizes[layer] = s;
      for(int i = 0; i < s; i++) {
	out.bricks[layer][i] = bricks[layer+1][i];
      }
    }
  }
				
  void Combination::removeSingleTopBrick(Combination &out) const {
    out.height = height-1;
    for(int layer = 0; layer < height-1; layer++) {
      int s = layerSizes[layer];
      out.layerSizes[layer] = s;
      for(int i = 0; i < s; i++) {
	out.bricks[layer][i] = bricks[layer][i];
      }
    }
  }

  bool Combination::removeBrickAt(int layer, int idx, Combination &out) const {
    int layerSize = layerSizes[layer];
    if(layerSize == 1) {
      if(layer == 0) {
	// Special case: Single lower brick removal
	removeSingleLowerBrick(out);
      }
      else if(layer == height-1) {
	// Special case: Top brick
	removeSingleTopBrick(out);
      }
      else {
	return false; // Single last brick in non-extreme layer!
      }
    }

    out.height = height;
    for(int i = 0; i < height; i++) {
      int s = layerSizes[i];
      if(i == layer) {
	out.layerSizes[i] = s-1;
	continue;
      }
      out.layerSizes[i] = s;
      for(int j = 0; j < s; j++) {
	out.bricks[i][j] = bricks[i][j];
      }
    }
				
    // Remove brick at index idx:
    for(int i = 0, outIdx = 0; i < layerSizes[layer]; i++) {
      if(idx == i) {
	continue;
      }
      out.bricks[layer][outIdx++] = bricks[layer][i];
    }

    // Check that combination is still connected:
    if(!out.isConnected()) {
      return false;
    }
		
    out.normalize();

    return true; // All OK.
  }

  void Combination::normalize(int &rotated) {
    // Ensure FirstBrick is first and all is sorted:
    bool hasVerticalLayer0Brick = false;
    for(int i = 0; i < layerSizes[0]; i++) {
      Brick &b = bricks[0][i];
      if(b.isVertical) {
	hasVerticalLayer0Brick = true;
	break;
      }
    }

    // Check if first brick is horizontal at 0,0:
    if(hasVerticalLayer0Brick) {
      translateMinToOrigo();
      sortBricks();
    }
    else {
      rotate90();
      rotated = 90;
    }
				
    Combination c(*this);
    if(can_rotate90()) {
      for(int i = 0; i < 3; i++) {
	c.rotate90();
	if(c < *this) {
	  copy(c);
	  rotated = 90*(i+1);
	}
      }
    }
    else {
      c.rotate180();
      if(c < *this) {
	copy(c);
	rotated = 180;
      }
    }
  }
  void Combination::normalize() {
    int ignore;
    normalize(ignore);
  }

  int Combination::countConnected(int layer, int idx) {
    connected[layer][idx] = true;
    const Brick &b = bricks[layer][idx];
    int ret = 1;
    // Add for layer below:
    if(layer > 0) {
      int s = layerSizes[layer-1];
      for(int i = 0; i < s; i++) {
	if(connected[layer-1][i]) {
	  continue;
	}
	const Brick &b2 = bricks[layer-1][i];
	if(b.intersects(b2)) {
	  ret += countConnected(layer-1, i);
	}
      }
    }
    // Add for layer above:
    if(layer < height-1) {
      int s = layerSizes[layer+1];
      for(int i = 0; i < s; i++) {
	if(connected[layer+1][i]) {
	  continue;
	}
	const Brick &b2 = bricks[layer+1][i];
	if(b.intersects(b2)) {
	  ret += countConnected(layer+1, i);
	}
      }
    }
    return ret;
  }

  bool Combination::isConnected() {
    int Z = 0;

    // Reset state:
    for(int i = 0; i < height; i++) {
      Z += layerSizes[i];
      int s = layerSizes[i];
      for(int j = 0; j < s; j++) {
	connected[i][j] = false;
      }
    }
    // Run DFS:
    int cnt = countConnected(0, 0);

    return cnt == Z;
  }

  void Combination::flip() {
    Brick tmp[MAX_BRICKS-1];
    for(int layer1 = 0, layer2 = height-1; layer1 < layer2; layer1++, layer2--) {
      int s1 = layerSizes[layer1];
      int s2 = layerSizes[layer2];
      // Save layer 1 in tmp:
      for(int i = 0; i < s1; i++)
	tmp[i] = bricks[layer1][i];
      // Move layer2 to layer1:
      for(int i = 0; i < s2; i++)
	bricks[layer1][i] = bricks[layer2][i];
      // Move tmp to layer 2:
      for(int i = 0; i < s1; i++)
	bricks[layer2][i] = tmp[i];
    }
    std::reverse(layerSizes, &layerSizes[height]);
    normalize();
  }

  uint64_t Combination::encodeFor12(bool &topSymmetric, bool &topConnected) const {
    assert(layerSizes[0] == 1);
    assert(layerSizes[1] == 2);

    uint64_t ret = 0;

    // Encode bricks:
    for(int i = 0; i < 2; i++) {
      for(int j = 0; j < layerSizes[i]; j++) {
	ret = (ret << 15) | bricks[i][j].encode15();
      }
    }
				
    // Additional bits:
    topSymmetric = isSymmetricAboveFirstLayer();
    topConnected = isConnectedAboveFirstLayer();
				
    return (ret << 2) | (topSymmetric << 1) | topConnected;
  }

  uint64_t Combination::encodeForYY(bool &topSymmetric, bool &topConnected) const {
    return 0; // TODO!
  }

  uint64_t Combination::countSymmetricLayer0Siblings() const {
    assert(height >= 2);
    assert(layerSizes[0] > 1);

    uint64_t ret = 0;
    int X = bricks[1][0].x + bricks[1][1].x;
    int Y = bricks[1][0].y + bricks[1][1].y;
    std::set<std::pair<int,int> > v, h, *seen;
    for(int i = 0; i < layerSizes[0]; i++) {
      const Brick &b = bricks[0][i];
      int x = b.x;
      int y = b.y;
      std::pair<int,int> p2(X-x, Y-y);
      seen = b.isVertical ? &v : &h;
      if(seen->find(p2) == seen->end()) {
	std::pair<int,int> p1(x,y);
	seen->insert(p1);
      }
      else {
	ret++;
      }
    }
    return ret;
  }

  uint64_t Combination::countSymmetricLayer0SiblingsThatConnectAbove() const {
    assert(layerSizes[0] >= 2);
    assert(layerSizes[1] == 2);

    uint64_t ret = 0;
    int X = bricks[1][0].x + bricks[1][1].x;
    int Y = bricks[1][0].y + bricks[1][1].y;
    std::set<std::pair<int,int> > v, h, *seen;
    for(int i = 0; i < layerSizes[0]; i++) {
      const Brick &b = bricks[0][i];
      if(!b.intersects(bricks[1][0]) || !b.intersects(bricks[1][1])) {
	continue; // Not connecting above
      }
						
      int x = b.x;
      int y = b.y;
      std::pair<int,int> p2(X-x, Y-y);
      seen = b.isVertical ? &v : &h;
      if(seen->find(p2) == seen->end()) {
	std::pair<int,int> p1(x,y);
	seen->insert(p1);
      }
      else {
	ret++;
      }
    }
    return ret;
  }

  bool CombinationReader::readBit() {
    if(bitIdx == 8) {
      istream->read((char*)&bits, 1);
      bitIdx = 0;
    }
    bool bit = (bits >> (7-bitIdx)) & 1;
    bitIdx++;
    return bit;
  }

  int8_t CombinationReader::readInt8() {
    int8_t ret = 0;
    bool negate = readBit();
    for(int i = 0; i < 6; i++) {
      ret = ret | ((int)readBit() << i);
    }
    if(negate)
      ret = -ret;
    return ret;
  }

  uint8_t CombinationReader::readUInt4() {
    uint8_t ret = 0;
    for(int i = 0; i < 4; i++) {
      ret = ret | ((int)readBit() << i);
    }
    return ret;
  }
		
  uint32_t CombinationReader::readUInt32() {
    uint32_t ret = 0;
    for(int block = 0; block < 4; block++) {
      for(int i = 0; i < 8; i++) {
	ret = ret | ((int)readBit() << i);
      }
    }
    return ret;
  }
		
  Brick CombinationReader::readBrick() {
    return Brick(readBit(), readInt8(), readInt8());
  }

  CombinationReader::CombinationReader(const int layerSizes[], int Z) : height(0), Z(Z), token(0), bits(0), bitIdx(8), done(false), combinationCounter(0) {
    // bitIdx = 8 -> Ensure a byte is read next time

    std::stringstream ss, ss2;
    ss << Z << "/";

    int size_total = 0;
    for(int i = 0; size_total < Z; i++) {
      int layerSize = layerSizes[i];
      this->layerSizes[i] = layerSize;
      ss2 << layerSize;
      size_total += layerSize;
      height++;
    }
    for(int i = height-1; i >= 0; i--) {
      token = token * 10 + layerSizes[i];
    }
    assert(Z == size_total);

    std::string layerString = ss2.str(), layerStringReverse = layerString;
    std::reverse(layerStringReverse.begin(), layerStringReverse.end());				
    reverse = layerString < layerStringReverse; // Reverse => Read upside-down.

    name = layerString; // Do not reverse
    ss << (reverse ? layerStringReverse : layerString);
    std::string file_name = ss.str();

    bool invalid = height == 2 && size_total >= 8 && (layerSizes[0] == 1 || layerSizes[1] == 1);

    if(Z > 1 && !invalid) {
      istream = new std::ifstream(file_name.c_str(), std::ios::binary);
    }
    else {
      istream = NULL;
    }
    if(invalid) {
      done = true;
    }

    if(reverse) {
      std::reverse(this->layerSizes, &this->layerSizes[height]);
    }

    baseCombination.bricks[0][0] = FirstBrick;
    combinationsLeft = 0;

    std::cout << "   READER for " << layerString << ": " << file_name << ", reversed: " << reverse << ", invalid: " << invalid << std::endl;
  }

  CombinationReader::~CombinationReader() {
    if(istream != NULL) {
      std::cout << "  Closing combination reader " << name << ". Combinations read: " << combinationCounter << std::endl;
				
      istream->close();
      delete istream;
    }
  }

  // Assume c already has height and layerSizes set.
  bool CombinationReader::nextCombination(Combination &c) {
    std::lock_guard<std::mutex> guard(read_mutex); // Ensure no double-reading.
				
    if(done)
      return false;
    if(Z == 1) {
      c.layerSizes[0] = 1;
      c.bricks[0][0] = FirstBrick;
      c.height = 1;
      done = true;
      return true;
    }

    if(combinationsLeft == 0) { // Read new base combination:
      brickLayer = readUInt4(); // Brick to be added to base combination.
      if(brickLayer == 15) {
	done = true;
	return false;
      }

      baseCombination.height = height - (brickLayer == height-1 && layerSizes[brickLayer] == 1);

      for(int i = 0; i < baseCombination.height; i++) {
	int s = layerSizes[i] - (brickLayer == i);
	baseCombination.layerSizes[i] = s;

	for(int j = (i == 0 ? 1 : 0); j < s; j++)
	  baseCombination.bricks[i][j] = readBrick();
      }						
      combinationsLeft = readUInt32();
    }

    // Copy base combination and add one brick:
    c.height = height;
    for(int i = 0; i < height; i++) {
      c.layerSizes[i] = layerSizes[i];
      int in = 0;
      if(i == brickLayer) {
	c.bricks[i][0] = readBrick();
	in = 1;
      }
      int s = (i == baseCombination.height) ? 0 : baseCombination.layerSizes[i];
      for(int j = 0; j < s; j++) {
	c.bricks[i][j+in] = baseCombination.bricks[i][j];
      }
    }
    combinationsLeft--;

    if(reverse)
      c.flip();
    else
      c.normalize(); // Because brickLayer might not be sorted

#ifdef TRACE
    std::cout << "  Reading <" << name << "> combination " << combinationCounter << ": " << c << std::endl;
#endif
    combinationCounter++;
    if((combinationCounter-1)%10000000 == 10000000-1)
      std::cout << " " << combinationCounter/1000000 << " million <" << name << "> combinations read" << std::endl;
    return true;
  }

  CombinationWriter::CombinationWriter(const int token, const bool saveOutput) : height(0), writtenFull(0), writtenShort(0), token(token), counts(), Z(0) {
#ifdef DEBUG
    std::cout << " Create Combination writer for token " << token << ", output?: " << saveOutput << std::endl;
#endif

    int inverseToken = 0, tkn = token;
    while(tkn > 0) {
      int size_add = tkn % 10;
      inverseToken = inverseToken * 10 + size_add;
      Z += size_add;
      tkn /= 10;
      height++;
    }

    std::stringstream ss;
    ss << Z << "/";
	
    while(inverseToken > 0) {
      ss << (inverseToken % 10);
      inverseToken /= 10;
    }

    std::string file_name = ss.str();

    if(saveOutput) {
      ostream = new std::ofstream(file_name.c_str(), std::ios::binary);
      bits = cntBits = 0;
    }
    else
      ostream = NULL;
  }

  CombinationWriter::CombinationWriter(const CombinationWriter &cw) : ostream(cw.ostream), height(cw.height), writtenFull(0), writtenShort(0), token(cw.token), counts() {
    
  }

  CombinationWriter::~CombinationWriter() {
#ifdef DEBUG
    if(writesToFile())
      std::cout << "  Written combinations full: " << writtenFull << ", short: " << writtenShort << " Compression% " << (writtenShort)*100.0/(writtenFull+writtenShort) << std::endl;
#endif
    if(ostream != NULL) {
      std::cout << "   Closing write stream for " << token << std::endl;
      writeUInt4(15); // Mark end of file.
      flushBits();
      ostream->flush();
      ostream->close();
      delete ostream;
    }
  }

  void CombinationWriter::writeBit(bool bit) {
    bits = (bits << 1) + (bit ? 1 : 0);
    cntBits++;
    if(cntBits == 8) {
      ostream->write((char*)&bits, 1);
      cntBits = 0;
    }
  }

  void CombinationWriter::flushBits() {
    while(cntBits > 0) {
      writeBit(0);
    }
  }

  // Max difference from first brick is for 11 bricks: 10*3 = 30 studs, so 6 bits suffice:
  void CombinationWriter::writeInt8(const int8_t toWrite) {
    int i = toWrite;
    if(i < 0) {
      writeBit(true); // minus
      i = -i;
    }
    else {
      writeBit(false); // plus
    }
    for(int j = 0; j < 6; j++) {
      writeBit(i & 1);
      i >>= 1;
    }
    assert(i == 0); // At most 6 bits used.
  }

  void CombinationWriter::writeUInt4(const uint8_t toWrite) {
    uint8_t i = toWrite;
    for(int j = 0; j < 4; j++) {
      writeBit(i & 1);
      i >>= 1;
    }
  }

  void CombinationWriter::writeUInt32(uint32_t toWrite) {
    for(int j = 0; j < 32; j++) {
      writeBit(toWrite & 1);
      toWrite >>= 1;
    }
  }

  void CombinationWriter::writeBrick(const Brick &b) {
    writeBit(b.isVertical);
    writeInt8(b.x);
    writeInt8(b.y);
  }

  void CombinationWriter::writeCombinations(const Combination &baseCombination, uint8_t brickLayer, std::vector<Brick> &v) {
    if(ostream == NULL || v.empty()) {
      return;
    }

    writeUInt4(brickLayer);
    for(int i = 0; i < baseCombination.height; i++) {
      int s = baseCombination.layerSizes[i];
      for(int j = (i == 0 ? 1 : 0); j < s; j++) {
	writeBrick(baseCombination.bricks[i][j]);
      }
    }
    writeUInt32(v.size());

    for(int i = 0; i < v.size(); i++) {
      writeBrick(v[i]);
    }

    writtenFull++;
    writtenShort+=v.size();
  }

  Counts CombinationWriter::add(Combination &cOld, const Brick &addedBrick, const uint8_t layer) {
#ifdef TRACE
    std::cout << "   TRY TO ADD " << addedBrick << " at layer " << (int)layer << std::endl;
#endif
    Combination c; // Build completely in addBrick()
    int rotated = 0;
    if(!cOld.addBrick(addedBrick, layer, c, rotated)) {
#ifdef TRACE
      std::cout << " Does not fit!" << std::endl;
#endif
      return Counts(); // Does not fit.
    }

    // If cOld is symmetric, then check if we are first:
    if(cOld.is180Symmetric() && rotated > 90) {
#ifdef TRACE
      std::cout << " Not first version of symmetric combination!" << std::endl;
#endif
      return Counts(); // Not first version of this symmetric combination.
    }

    // Try to remove bricks b from c up to and including layer of addedBrick (unless layer only has 1 brick)
    // If b is lower layer than addedBrick and combination stays connected, then do not add!
    // If b same layer as addedBrick: Do not add if combination without b < combination without addedBrick
    Combination ignore, sibling;
    if(layer > 0) {
      for(int i = 0; i < layer; i++) {
	for(int j = 0; j < c.layerSizes[i]; j++) {
	  if(c.layerSizes[i] > 1 && c.removeBrickAt(i, j, ignore)) {
#ifdef TRACE
	    std::cout << "  Could be constructed from lower refinement " << ignore << std::endl;
#endif
	    return Counts(); // Sibling comes before cOld, so we do not add
	  }
	}
      }
    }

    int s = c.layerSizes[layer];
    if(s >= 2) { // If there is at least one other brick at the layer:
      for(int j = 0; j < s; j++) {
	if(c.removeBrickAt(layer, j, sibling)) { // Can remove siblingBrick:
	  if(sibling < cOld) {
#ifdef TRACE
	    std::cout << "  Could be constructed from lesser sibling" << sibling << std::endl;
#endif
	    return Counts(); // Lesser sibling!
	  }
	}
      }
    }

    // All OK! c can be added and counted:
#ifdef DEBUG
    std::cout << " ALL OK! Adding " << c << " for token '" << token << "'" << std::endl;
#endif

    assert(c.isValid(token, "Counting"));

    // Construct return value:
    Combination rotated180(c);
    rotated180.rotate180();
    if(c == rotated180) {
      if(c.height == 2 && c.layerSizes[0] == 4 && c.layerSizes[1] == 4) {
	Combination rotated90(c);
	rotated90.rotate90();
	if(c == rotated90)
	  return Counts(1, 1, 1);
      }
      return Counts(1, 1, 0);
    }
    return Counts(1, 0, 0);
  }

  void CombinationWriter::makeNewCombinations(Combination &c, const int layer) {
#ifdef TRACE
    std::cout << "-----------------------------" << std::endl;
    std::cout << "Building on " << c << " by adding to layer " << layer << std::endl;
#endif

    // Stop early if c is stable with brick below layer-1 removed!
    // Notice: This does not include layer-1, as we place on layer!
    Combination evenSmaller;
    for(int i = 0; i < layer-1; i++) {
      int s = c.layerSizes[i];
      if(s == 1) {
	continue; // Do not remove brick if it is the only one in the layer!
      }
      for(int j = 0; j < s; j++) {
	if(c.removeBrickAt(i, j, evenSmaller)) {
#ifdef TRACE
	  std::cout << "EARLY EXIT due to stable base on layer " << i << ": " << c << " -> " << evenSmaller << std::endl;
#endif
	  return;
	}
      }
    }

    // If there is at least 2 bricks on layer-1 and all bricks on layer-1 are expendable,
    // then we can skip this model too!
    if(layer > 0 && c.layerSizes[layer-1] > 1) {
      bool allExpendable = true;
      int s = c.layerSizes[layer-1];
      for(int j = 0; j < s; j++) {
	if(!c.removeBrickAt(layer-1, j, evenSmaller)) {
	  allExpendable = false;
	  break;
	}
      }
      if(allExpendable) {
#ifdef TRACE
	std::cout << "EARLY EXIT due to all expendable on layer " << layer-1 << ": " << c << std::endl;
#endif
	return;
      }
    }
				
    // Build new combinations by adding to all existing bricks:
    std::set<Brick> alreadyAdded, neighbours;
    std::vector<Brick> v;

    // Find all potential neighbours above and below:
    if(layer > 0) {
      int s = c.layerSizes[layer-1];
      for(int j = 0; j < s; j++) {
	Brick &b = c.bricks[layer-1][j];
	neighbours.insert(b);
      }
    }
    if(layer+1 < c.height) {
      int s = c.layerSizes[layer+1];
      for(int j = 0; j < s; j++) {
	Brick &b = c.bricks[layer+1][j];
	neighbours.insert(b);
      }
    }
				
    for(std::set<Brick>::const_iterator it = neighbours.begin(); it != neighbours.end(); it++) {
      const Brick &brick = *it;
#ifdef TRACE
      std::cout << " Try adding brick " << brick << " on layer " << layer << std::endl;
#endif

      bool isVertical = brick.isVertical;

      // Add crossing bricks (one vertical, one horizontal):
      for(int x = -2; x < 3; x++) {
	for(int y = -2; y < 3; y++) {
	  Brick b(!isVertical, brick.x+x, brick.y+y);
	  if(alreadyAdded.insert(b).second) { // Brick not already added:
	    Counts added = add(c, b, layer);
	    if(added.all > 0) {
	      if(writesToFile())
		v.push_back(b);
	    }
	    counts += added;
	  }
	}
      }

      // Add parallel bricks:
      int w = 4, h = 2;
      if(isVertical) {
	w = 2;
	h = 4;
      }		
      for(int y = -h+1; y < h; y++) {
	for(int x = -w+1; x < w; x++) {
	  Brick b(isVertical, brick.x+x, brick.y+y);
	  if(alreadyAdded.insert(b).second) {
	    Counts added = add(c, b, layer);
	    if(added.all > 0) {
	      if(writesToFile())
		v.push_back(b);
	    }
	    counts += added;
	  }
	}
      }
#ifdef TRACE
      std::cout << " Done adding to brick " << brick << std::endl;
#endif
    }

    // Write:
    writeCombinations(c, layer, v);
				
#ifdef TRACE
    std::cout << "DONE Building on " << c << std::endl;
    std::cout << "-----------------------------" << std::endl;
#endif
  }

  void CombinationWriter::fillFromReader(int const * const layerSizes, const int reducedLayer, CombinationReader * reader) {
    // Combination to read:
    Combination c;
    c.height = height;
    for(int i = 0; i < height; i++) {
      c.layerSizes[i] = layerSizes[i];
    }

    while(reader->nextCombination(c)) {
      makeNewCombinations(c, reducedLayer);
    }
  }

  bool CombinationWriter::writesToFile() const {
    return ostream != NULL;
  }

  void Counter::fillFromReaders(CombinationWriter &writer) {
    // Build layers:
    int layers = 0, tkn = writer.token;
    int layerSizes[MAX_HEIGHT];
    while(true) {
      int size_add = tkn % 10;
      if(size_add == 0)
	break;
      layerSizes[layers++] = size_add;
      tkn /= 10;
    }
    // Flip the layer sizes:
    for(int i = 0; i < layers/2; i++) {
      std::swap(layerSizes[i], layerSizes[layers-i-1]);
    }

    for(int i = 0; i < layers; i++) { // Reader from reader where brick on layer i was added:
      if(layerSizes[i] == 1 && i != layers-1)
	continue; // Size 1 layer only handled if last.
      if(layers == 2 && i == 1 && layerSizes[i-1] == writer.Z-1 && writer.Z != 2)
	continue; // Size 1 last layer will split all!

      layerSizes[i]--;
      CombinationReader reader(layerSizes, writer.Z-1);

      unsigned int processor_count = std::thread::hardware_concurrency() - 2; // allow 2 processors for OS and other...
      if(!writer.writesToFile() && processor_count > 1) {
	std::cout << "Splitting into " << processor_count << " threads!" << std::endl;
	// Fill from readers in threads:
	std::vector<std::thread*> threads;
	std::vector<CombinationWriter*> writers;

	for(unsigned int j = 0; j < processor_count; j++) {
	  writers.push_back(new CombinationWriter(writer));
	  std::thread *t = new std::thread(&CombinationWriter::fillFromReader, std::ref(*writers[j]), layerSizes, i, &reader);
	  threads.push_back(t);
	}
	for(unsigned int j = 0; j < processor_count; j++) {
	  (*threads[j]).join();
	  counts += writers[j]->counts;
	}
      }
      else { // Single threaded if output is written:
	writer.counts.reset();
	writer.fillFromReader(layerSizes, i, &reader);
	counts += writer.counts;
      }
						
      layerSizes[i]++;
    }
  }

  void Counter::handleFinalCombinationWriters(const int token, const bool saveOutput) {
    std::chrono::time_point<std::chrono::steady_clock> t_init = std::chrono::steady_clock::now();
    Counts before = counts;
    std::cout << " Handling combinations for token " << token << std::endl;

    CombinationWriter writer(token, saveOutput);
    fillFromReaders(writer);

    std::cout << "  Constructed " << (counts-before) << " combinations for token " << token << " in ";
    std::chrono::time_point<std::chrono::steady_clock> t_end = std::chrono::steady_clock::now();
    std::chrono::duration<double> t = t_end - t_init;
    std::cout << t.count() << "s." << std::endl;
  };

  /*
    Construct the writers.
    If remaining is 0, the writer of name token is constructed and added to writers.
    Otherwise, combinations of all remaining are constructed recursively.
  */
  void Counter::handleCombinationWriters(int token, int remaining, bool add_self, bool saveOutput) {
    if(remaining == 0) {
      handleFinalCombinationWriters(token, saveOutput);
      return;
    }

#ifdef TRACE
    std::cout << " Building recurse writers for token " << token << ", remaining: " << remaining << ", add self?: " << add_self << std::endl;
#endif
    for(int i = remaining - (add_self ? 0 : 1); i > 0; i--) {
      int ntoken = 10*token + i;
      handleCombinationWriters(ntoken, remaining - i, true, saveOutput);
    }
  }

  Counts Counter::countLayer0P(int Z, int layer0Size, int layer1Size, Combination &symmetryChecker, int idx, std::set<Brick>::const_iterator itBegin, std::set<Brick>::const_iterator itEnd, const uint64_t divisor, bool topSymmetric, bool topConnected, std::set<Combination> &seen) {
    Counts ret;

    if(idx == layer0Size-1) {
      Combination c(symmetryChecker);
      c.normalize();

      if(seen.find(c) != seen.end()) {
	return ret; // Already seen!
      }
      seen.insert(c);

      assert(c.height >= 2);
      assert(c.layerSizes[0] == layer0Size);
      assert(c.layerSizes[1] == layer1Size);

      bool isSymmetric = c.is180Symmetric();
      uint64_t timesCounted;

      if(!topConnected) {
	uint64_t connectsAbove = 0;
	for(int i = 0; i < layer0Size; i++) {
	  Brick &b = c.bricks[0][i];
	  if(b.intersects(c.bricks[1][0]) && b.intersects(c.bricks[1][1])) {
	    connectsAbove++; // TODO: Currently assumes layer1Size == 2
	  }
	}

	if(topSymmetric) {
	  uint64_t s = c.countSymmetricLayer0SiblingsThatConnectAbove();
	  timesCounted = connectsAbove - s;
	}
	else { // Top is not connected and not symmetric:
	  timesCounted = connectsAbove;
	}
      }
      else { // topConnected:
	// Top is connected, so each brick in layer 0 causes a count... unless
	// top is symmetric and bricks below have symmetric sibling
	if(topSymmetric) {
	  uint64_t s = c.countSymmetricLayer0Siblings();
	  timesCounted = layer0Size - s;
	}
	else {
	  timesCounted = layer0Size; // Once for each brick
	}
      }

      ret.all += divisor / timesCounted;
      if(topSymmetric && isSymmetric) {
	ret.symmetric180 += divisor / timesCounted;
      }

      return ret;
    }

    // Recursive branch:
    for(std::set<Brick>::const_iterator it = itBegin; it != itEnd; it++) {
      const Brick b = *it;
      // Check that b does not collide:
      bool ok = true;
      for(int i = layer1Size-1; i < idx+1; i++) {
	if(symmetryChecker.bricks[0][i].intersects(b)) {
	  ok = false;
	  break;
	}
      }
      if(!ok)
	continue; // Intersection with previously placed brick!
				
      symmetryChecker.bricks[0][idx+1] = b;
      std::set<Brick>::const_iterator nxt = it;
      nxt++;
      ret += countLayer0P(Z, layer0Size, layer1Size, symmetryChecker, idx+1, nxt, itEnd, divisor, topSymmetric, topConnected, seen);
    }

    return ret;
  }

  /*
    Place "toPlace" bricks on layer 0 in all ways possible.
    Z = Total size of what to build
  */
  Counts Counter::countLayer0Placements(int Z, int layer0Size, int layer1Size, Combination &c, const uint64_t divisor, bool topSymmetric, bool topConnected, std::set<Combination> &seen) {
    assert(c.height >= 2);
    assert(layer0Size >= 2);
    assert(layer1Size == 2);
    assert(c.layerSizes[0] == 1);
    assert(c.layerSizes[1] == layer1Size);

    Brick *blockers = c.bricks[0]; // Existing bricks on first layer, which might block/overlap with additional bricks

    // Find legal placements v:
    std::set<Brick> s;

    for(int i = 0; i < layer1Size; i++) {
      Brick &b = c.bricks[1][i]; // For each brick on second layer
      bool isVertical = b.isVertical;

      // Add crossing bricks (one vertical, one horizontal):
      for(int x = -2; x < 3; x++) {
	for(int y = -2; y < 3; y++) {
	  Brick b2(!isVertical, b.x+x, b.y+y);
	  bool ok = true;
	  for(int j = 0; j < layer0Size; j++) {
	    if(blockers[j].intersects(b2)) {
	      ok = false;
	      break;
	    }
	  }
	  if(ok)
	    s.insert(b2);
	}
      }
      // Add parallel bricks:
      int w = 4, h = 2;
      if(isVertical) {
	w = 2;
	h = 4;
      }		
      for(int y = -h+1; y < h; y++) {
	for(int x = -w+1; x < w; x++) {
	  Brick b2(isVertical, b.x+x, b.y+y);
	  bool ok = true;
	  for(int j = 0; j < layer0Size; j++) {
	    if(blockers[j].intersects(b2)) {
	      ok = false;
	      break;
	    }
	  }
	  if(ok)
	    s.insert(b2);
	}
      }
    }

    Combination symmetryChecker(c);
    symmetryChecker.layerSizes[0] = layer0Size;

    return countLayer0P(Z, layer0Size, layer1Size, symmetryChecker, 0, s.begin(), s.end(), divisor, topSymmetric, topConnected, seen);
  }

  /*
    Special case: First layer has at least 2 bricks, while second layer has 2.
    Call it <X2...>, X >= 2.
    Iterate over <12...> of same height.
    Add remaining bricks to first layer and count how many ways that can be done.
    Multiply with models in <12...> instead of repeating the full process for each model therein.
  */
  void Counter::countX2(int Z, int layer0Size, char* input) {
    int smallerToken = 1; // Just have 1 in first layer of 'smaller'
    int smallerLayerSizes[MAX_HEIGHT];
    smallerLayerSizes[0] = 1;
    char c;
    for(int i = 1; (c = input[i]); i++) {
      smallerToken = smallerToken * 10 + (c-'0');
      smallerLayerSizes[i] = (c-'0');
    }
    std::cout << "Filling to " << layer0Size << " bricks for first layer of <" << smallerToken << ">" << std::endl;

    // Go through all smaller combinations:
    Counts ret;
    uint64_t divisor = 5*6*7*8*9, countSmaller = 0, cntSkip = 0;

    std::map<uint64_t,Counts> cache;

    const int smallerSize = Z-layer0Size+1; // Smaller only has 1 brick in first layer
    CombinationReader reader(smallerLayerSizes, smallerSize);
    Combination smaller; // <12...>

    while(reader.nextCombination(smaller)) {
      countSmaller++;

      // Find all locations to place a brick on first layer:
      bool topSymmetric, topConnected;
      uint64_t encoded = smaller.encodeFor12(topSymmetric, topConnected);

      if(cache.find(encoded) != cache.end()) {
	ret += cache[encoded];
	cntSkip++;
	if((cntSkip-1)%100000000 == 100000000-1)
	  std::cout << " " << (cntSkip/1000000) << " million skipped of " << countSmaller << " read" << std::endl;
	else if((cntSkip-1)%10000000 == 10000000-1)
	  std::cout << ":" << std::flush;
	else if((cntSkip-1)%1000000 == 1000000-1)
	  std::cout << "." << std::flush;
      }
      else {
	std::set<Combination> seen;
	Counts cnt = countLayer0Placements(Z, layer0Size, 2, smaller, divisor, topSymmetric, topConnected, seen);

	cache[encoded] = cnt;

	ret += cnt;

	if(cache.size()%1000 == 0)
	  std::cout << "| " << cache.size() << " |" << std::flush;
      }
    }
    std::cout << " Read " << countSmaller << " smaller combinations." << std::endl;
    std::cout << " Counted*" << divisor << ": "  << ret << std::endl;
    std::cout << " Skipped " << cntSkip << " (" << (cntSkip*100.0)/countSmaller << "%)" << std::endl;
    std::cout << " Cache size " << cache.size() << std::endl;

    assert(ret.all % divisor == 0);
    assert(ret.symmetric180 % divisor == 0);

    ret.all /= divisor;
    ret.symmetric180 /= divisor;
    std::cout << "Final results: " << ret << std::endl;
  }

  void Counter::countRefinements(const int Z, int token, const int height, char* input,const bool saveOutput) {
    // Special case handling:
    if(height >= 2 && input[0] >= '2' && input[1] == '2') {
      std::cout << "Special case <X2...>, X >= 2" << std::endl;
      const int layer0Size = input[0]-'0';
      countX2(Z, layer0Size, input);
    }
    else { // Normal case:
      int reverseToken = 0;
      char c;
      for(int i = height-1; i >= 0; i--) {
	c = input[i];
	reverseToken = reverseToken * 10 + (c-'0');
      }

      if(token < reverseToken) {
	token = reverseToken;
      }

      handleFinalCombinationWriters(token, saveOutput);
    }
  }

  void Counter::buildAllCombinations(int Z, bool saveOutput) {
    std::chrono::time_point<std::chrono::steady_clock> t_init = std::chrono::steady_clock::now();
    if(Z == 1) {
      return; // Trivial case with 1 brick.
    }

    std::cout << "Building all combinations of size " << Z << std::endl;
    handleCombinationWriters(0, Z, false, saveOutput);

    std::cout << "Constructed " << counts << " combinations of size " << Z << " in ";
    std::chrono::time_point<std::chrono::steady_clock> t_end = std::chrono::steady_clock::now();
    std::chrono::duration<double> t = t_end - t_init;
    std::cout << t.count() << "s." << std::endl;
    if(Z == 2)
      assert(counts.all == 24);
    else if(Z == 3)
      assert(counts.all == 1560);
    else if(Z == 4)
      assert(counts.all == 119580);
    else if(Z == 5)
      assert(counts.all == 10166403);
    else if(Z == 6)
      assert(counts.all == 915103765);
  }

} // namespace rectilinear
