#ifndef MODELLING_MODEL_ENCODER_H
#define MODELLING_MODEL_ENCODER_H

#include "RectilinearBrick.h"
#include "ConnectionPoint.h"
#include "Block.hpp"
#include "Model.hpp"
#include "../util/TinyVector.hpp"

namespace modelling {
	typedef std::pair<uint64_t, uint64_t> Encoding;

	/*
	Used for uniquely encoding:
	- A model, not necessarily valid, without given angles, into a uint64_t.
	- A model with angles into a EncodedModel.
	Performs necessary rotations and permutations on FatBlocks of model in order to minimize encoded value.
	*/
	class ModelEncoder {
		FatBlock fatBlocks[6];
		unsigned int fatBlockSize;
		// Compressed index for bricks enables brick ID saving using 3 bit:
		BrickIdentifier compressedToIdentifier[6];
		int identifierToCompressed[36]; // 6*modelI + brickI in block.
										// Duplicate mapping assists in quick lookup for running indices of duplicate Blocks:
		unsigned int duplicateMapping[6];

	public:
		ModelEncoder(const util::TinyVector<FatBlock, 6> &combination);
		ModelEncoder& operator=(const ModelEncoder &) {
			assert(false); // Assignment operator should not be used.
			util::TinyVector<FatBlock, 6> c;
			ModelEncoder *cn = new ModelEncoder(c); // Not deleted - fails on invoce.
			return *cn;
		}

		/*
		Setup:
		- The blocks are sorted by size,diskI (done in constructor).
		- The connection points of connections are sorted and listed for each block.

		Min finding Algorithms:
		- base block index (encodedI) = 0.
		- iterate on connections. Any found index ++.
		- Now iterate on found of base.

		Encoding algorithm:
		- Encode connections ordered by brick::encodedI, connectionPoint of brick.
		- Encode a connection:
		-- aboveBrickI(3 bit),
		-- aboveConnectionPointType(2 bit)
		-- belowBrickI(3 bit),
		-- belowConnectionPointType(2 bit)
		-- => 5*10 bit < 64 bit.

		Additions:
		- If multiple min(size,diskI): Try all min.
		- If min is rotationally symmetric, try turned 180.
		*/
		Encoding encode(const IConnectionPairSet &list) const;

		void decode(Encoding encoded, IConnectionPairSet &list) const;
		void decode(uint64_t encoded, IConnectionPairSet &list) const;

		void testCodec(const IConnectionPairSet &list1) const;
		void writeFileName(std::ostream &ss, const util::TinyVector<AngledConnection, 5> &l, bool includeAngles) const;

	private:
		void rotateBlock(int i, util::TinyVector<ConnectionPoint, 8> *connectionPoints, std::map<ConnectionPoint, IConnectionPair> *connectionMaps) const;

		/*
		When encoding:
		- perform permutation so that Block of same size,diskIndex are ordered by visiting order.
		- Rotate rotationally symmetric Blocks so that they are initially visited at the minimally rotated position.
		*/
		Encoding encode(unsigned int baseIndex, bool rotate, util::TinyVector<ConnectionPoint, 8> *connectionPoints, std::map<ConnectionPoint, IConnectionPair> *connectionMaps) const;
		uint64_t encodeList(const util::TinyVector<IConnectionPair, 5> &toEncode, int * const perm) const;
	};
}

#endif // MODELLING_MODEL_ENCODER_H
