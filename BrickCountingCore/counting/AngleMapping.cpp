#include "AngleMapping.h"

#include <time.h>
#include <sstream>

#include "../Common.h"
#include "../util/MPDPrinter.h"
#include "../modelling/Brick.h"
#include "../modelling/Model.hpp"
#include "../modelling/TurningSingleBrick.h"

namespace geometry {
	void intervalToArray(const IntervalList &l, bool *array, unsigned int sizeArray) {
		const Interval* it = l.begin();
		for (unsigned int i = 0; i < sizeArray; ++i) {
			double v = -MAX_ANGLE_RADIANS + ((MAX_ANGLE_RADIANS + MAX_ANGLE_RADIANS)*i) / ((double)sizeArray - 1);
			if (it == l.end() || v < it->first) {
				array[i] = false; // sizeArray-1-i
			}
			else if (v > it->second) {
				array[i] = false; // sizeArray-1-i
				++it;
			}
			else
				array[i] = true; // sizeArray-1-i
		}
	}
}

namespace counting {
	void AngleMapping::init() {
		// Boosts:
		for (int i = 0; i < BOOST_STAGES; ++i)
			boosts[i] = 0;

		setupAngleTypes();
		int numFreeAngles = 0;
		// Set up angle steps:
		for (unsigned int i = 0; i < numAngles; ++i) {
			switch (angleTypes[i]) {
			case 0:
				angleSteps[i] = STEPS_0;
				break;
			case 1:
				angleSteps[i] = STEPS_1 * (boostPrecision ? PRECISION_BOOST_MULTIPLIER : 1);
				++numFreeAngles;
				break;
			case 2:
				angleSteps[i] = STEPS_2 * (boostPrecision ? PRECISION_BOOST_MULTIPLIER : 1);
				++numFreeAngles;
				break;
			case 3:
				angleSteps[i] = STEPS_3 * (boostPrecision ? PRECISION_BOOST_MULTIPLIER : 1);
				++numFreeAngles;
				break;
			}
		}
		singleFreeAngle = numFreeAngles == 1;

		if (findExtremeAnglesOnly)
			return; // no setup of smappings.

		  // sizeMappings:
		sizeMappings = 1;
		for (unsigned int i = 0; i < numAngles - 1; ++i) {
			sizeMappings *= (2 * angleSteps[i] + 1);
		}
#ifdef _TRACE
		std::cout << "SML size " << sizeMappings << std::endl;
#endif

		// S, M & L:
		SS = new geometry::IntervalListVector(sizeMappings, MAX_LOAD_FACTOR); // deleted in ~AngleMapping() and setDoublePrecision()
		MM = new geometry::IntervalListVector(sizeMappings, MAX_LOAD_FACTOR); // deleted in ~AngleMapping() and setDoublePrecision()
		LL = new geometry::IntervalListVector(sizeMappings, MAX_LOAD_FACTOR); // deleted in ~AngleMapping() and setDoublePrecision()
	}

	AngleMapping::AngleMapping(FatBlock const * const blocks, int numBlock, const util::TinyVector<IConnectionPair, 5> &cs, const ModelEncoder &encoder, std::ofstream &os, bool findExtremeAnglesOnly) :
		numAngles(numBlock - 1), numBricks(0), encoder(encoder), findExtremeAnglesOnly(findExtremeAnglesOnly), os(os), boostPrecision(false) {
		// Simple copying:
		for (int i = 0; i < numBlock; ++i) {
			this->blocks[i] = blocks[i];
			numBricks += blocks[i].size;
		}
		// Connection info:
		unsigned int i = 0;
		for (const IConnectionPair* it = cs.begin(); it != cs.end(); ++it) {
			const IConnectionPair &c = *it;
			const IConnectionPoint &icp1 = c.first;
			const IConnectionPoint &icp2 = c.second;
			points[i++] = icp1;
			points[i++] = icp2;
		}
		// initialize rest:
		init();
	}

	AngleMapping::~AngleMapping() {
		if (findExtremeAnglesOnly)
			return; // no setup of mappings.
		delete SS;
		delete MM;
		delete LL;
	}

	void AngleMapping::setupAngleTypes() {
		// First set up a counter for number of connections to blocks:
		unsigned int fatBlockConnectionCounts[6] = { 0,0,0,0,0,0 }; // block -> connections to it.
		unsigned int lastConnectedTo[6]; // block -> last seen connected block index.
		for (unsigned int i = 0; i < numAngles; ++i) {
			int i1 = points[2 * i].first.modelBlockI;
			int i2 = points[2 * i + 1].first.modelBlockI;
			fatBlockConnectionCounts[i1]++;
			fatBlockConnectionCounts[i2]++;
			lastConnectedTo[i1] = i2;
			lastConnectedTo[i2] = i1;
		}

		// Set up sizes (to be reduced later):
		unsigned int fatBlockSizes[6] = { 0,0,0,0,0,0 }; // block -> size of it.
		for (unsigned int i = 0; i < numAngles + 1; ++i) {
			fatBlockSizes[i] = blocks[i].size;
		}

		// Angle types computed based on size of components on side of turn point:
		for (unsigned int i = 0; i < numAngles; ++i)
			angleTypes[i] = 3;
		// Minimize angles connecting leaves:
		for (unsigned int i = 0; i < numAngles * 2; ++i) {
			int angleI = i / 2;
			int blockI = points[i].first.modelBlockI;
			if (fatBlockConnectionCounts[blockI] == 1)
				angleTypes[angleI] = MIN(angleTypes[angleI], MIN(fatBlockSizes[blockI], numBricks - fatBlockSizes[blockI]));
		}
		// Reduce graph by merging leaves into parents:
		for (unsigned int i = 0; i < numAngles + 1; ++i) {
			if (fatBlockConnectionCounts[i] == 1) {
				int otherI = lastConnectedTo[i];
				fatBlockSizes[otherI] += fatBlockSizes[i];
				fatBlockSizes[i] = 0;
				--fatBlockConnectionCounts[otherI];
				fatBlockConnectionCounts[i] = 0; // same as --
			}
		}

		// Perform initial minimizing again:
		for (unsigned int i = 0; i < numAngles * 2; ++i) {
			int angleI = i / 2;
			int blockI = points[i].first.modelBlockI;
			if (fatBlockConnectionCounts[blockI] == 1)
				angleTypes[angleI] = MIN(angleTypes[angleI], MIN(fatBlockSizes[blockI], numBricks - fatBlockSizes[blockI]));
		}

		// Finally. Set angle types of locked connections to type 0:
		// First set the ones locked directly on the Block:
		for (unsigned int i = 0; i < 2 * numAngles; ++i) {
			int angleI = i / 2;
			int blockI = points[i].first.modelBlockI;
			const ConnectionPoint &p = points[i].second;
			if (blocks[blockI].angleLocked(p)) {
				angleTypes[angleI] = 0;
			}
		}
		// Secondly, lock all touching connections:
		for (unsigned int i = 0; i < 2 * numAngles; ++i) {
			int angleI = i / 2;
			int blockI = points[i].first.modelBlockI;
			const ConnectionPoint &pi = points[i].second;

			for (unsigned int j = i + 1; j < 2 * numAngles; ++j) {
				int angleJ = j / 2;
				int blockJ = points[j].first.modelBlockI;
				if (blockI != blockJ)
					continue;
				const ConnectionPoint &pj = points[j].second;
				if (pi.angleLocks(pj)) {
					angleTypes[angleI] = 0;
					angleTypes[angleJ] = 0;
				}
			}
		}
	}

	Model AngleMapping::getModel(const MixedPosition &p) const {
		Model c(blocks[0]);
		for (unsigned int i = 0; i < numAngles - 1; ++i) {
			const IConnectionPoint &ip1 = points[2 * i];
			const IConnectionPoint &ip2 = points[2 * i + 1];
			const StepAngle angle((short)p.p[i] - (short)angleSteps[i], angleSteps[i] == 0 ? 1 : angleSteps[i]);
			const AngledConnection cc(ip1, ip2, angle);
			assert(ip2.P1.modelBlockI != 0);
			c.add(blocks[ip2.P1.modelBlockI], ip2.P1.modelBlockI, cc);
		}
		// Add last angle:
		const IConnectionPoint &ip1 = points[2 * (numAngles - 1)];
		const IConnectionPoint &ip2 = points[2 * (numAngles - 1) + 1];
		const StepAngle angle(p.lastAngle);
		const AngledConnection cc(ip1, ip2, angle);
		assert(ip2.P1.modelBlockI != 0);
		c.add(blocks[ip2.P1.modelBlockI], ip2.P1.modelBlockI, cc);

		return c;
	}

	Model AngleMapping::getModel(const Model &baseModel, int angleI, unsigned short angleStep) const {
		Model c(baseModel);

		const IConnectionPoint &ip1 = points[2 * angleI];
		const IConnectionPoint &ip2 = points[2 * angleI + 1];
		const StepAngle angle((short)angleStep - (short)angleSteps[angleI], angleSteps[angleI] == 0 ? 1 : angleSteps[angleI]);
		const AngledConnection cc(ip1, ip2, angle);
		assert(ip2.P1.modelBlockI != 0);
		c.add(blocks[ip2.P1.modelBlockI], ip2.first.modelBlockI, cc);

		return c;
	}

	Model AngleMapping::getModel(const Model &baseModel, double lastAngle) const {
		Model c(baseModel);

		const IConnectionPoint &ip1 = points[2 * (numAngles - 1)];
		const IConnectionPoint &ip2 = points[2 * (numAngles - 1) + 1];
		const StepAngle angle(lastAngle);
		const AngledConnection cc(ip1, ip2, angle);
		assert(ip2.P1.modelBlockI != 0);
		c.add(blocks[ip2.P1.modelBlockI], ip2.first.modelBlockI, cc);

		return c;
	}

	void AngleMapping::getModelConnections(const MixedPosition &p, util::TinyVector<AngledConnection, 5> &result) const {
		for (unsigned int i = 0; i < numAngles - 1; ++i) {
			const IConnectionPoint &ip1 = points[2 * i];
			const IConnectionPoint &ip2 = points[2 * i + 1];
			const StepAngle angle((short)p.p[i] - (short)angleSteps[i], angleSteps[i] == 0 ? 1 : angleSteps[i]);
			result.push_back(AngledConnection(ip1, ip2, angle));
		}

		const IConnectionPoint &ip1 = points[2 * (numAngles - 1)];
		const IConnectionPoint &ip2 = points[2 * (numAngles - 1) + 1];
		const StepAngle angle(p.lastAngle);
		result.push_back(AngledConnection(ip1, ip2, angle));
	}

	void AngleMapping::setBoostPrecision() {
		assert(!findExtremeAnglesOnly);
		delete SS;
		delete MM;
		delete LL;

		boostPrecision = true;
		init();
	}

	/*
	Evaluate the SML matrix.
	Parameters:
	- angleI: Used as recursion index in angles. Initialize with 0.
	- smlI: Index into SML. Initially 0.
	- c: Model being constructed dynamically. Initially containing blocks[0].
	- possibleCollisions: possible connections pre-computed to be used in next step. Remember to initialize!
	*/
	void AngleMapping::evalSML(unsigned int angleI, uint32_t smlI, const Model &c, bool noS, bool noM, bool noL) {
		// Find possible collisions:
		const IConnectionPoint &ip1 = points[2 * angleI];
		const IConnectionPoint &ip2 = points[2 * angleI + 1];
		unsigned int ip2I = ip2.first.modelBlockI;
		util::TinyVector<int, 5> possibleCollisions;
		c.getPossibleCollisions(blocks[ip2I], IConnectionPair(ip1, ip2), possibleCollisions);

		// Recursion:
		if (angleI < numAngles - 1) {
			// Update smlI:
			assert(smlI < sizeMappings);
			const unsigned short steps = 2 * angleSteps[angleI] + 1;
			smlI *= steps;

			for (unsigned short i = 0; i < steps; ++i) {
				Model c2 = getModel(c, angleI, i);
				bool noS2 = noS || !(singleFreeAngle ? c2.isRealizable<-EPSILON_TOLERANCE_MULTIPLIER>(possibleCollisions, blocks[ip2I].size) :
					(boostPrecision ? c2.isRealizable<-MOLDING_TOLERANCE_MULTIPLIER / PRECISION_BOOST_MULTIPLIER>(possibleCollisions, blocks[ip2I].size) :
						c2.isRealizable<-MOLDING_TOLERANCE_MULTIPLIER>(possibleCollisions, blocks[ip2I].size)));
				bool noM2 = noM || !c2.isRealizable<0>(possibleCollisions, blocks[ip2I].size);
				bool noL2 = noL || !(singleFreeAngle ? c2.isRealizable<EPSILON_TOLERANCE_MULTIPLIER>(possibleCollisions, blocks[ip2I].size) :
					(boostPrecision ? c2.isRealizable<MOLDING_TOLERANCE_MULTIPLIER / PRECISION_BOOST_MULTIPLIER>(possibleCollisions, blocks[ip2I].size) :
						c2.isRealizable<MOLDING_TOLERANCE_MULTIPLIER>(possibleCollisions, blocks[ip2I].size)));

				evalSML(angleI + 1, smlI + i, c2, noS2, noM2, noL2);
			}
			return;
		}

		// End of recursion:
		assert(angleI == numAngles - 1);

		// Speed up for noSML:
		if (noS) SS->insertEmpty(smlI);
		if (noM) MM->insertEmpty(smlI);
		if (noL) LL->insertEmpty(smlI);
		bool sDone = noS;
		bool mDone = noM;
		bool lDone = noL;
		if (sDone && mDone && lDone) {
			++boosts[0];
			return;
		}
#ifdef _TRACE
		std::cout << "NO SML statuses: " << noS << ", " << noM << ", " << noL << std::endl;
#endif

		if (angleTypes[angleI] == 0) {
			Model c2 = getModel(c, angleI, 0);
			geometry::IntervalList full;
			full.push_back(geometry::Interval(-EPSILON, EPSILON));

			if (!sDone) {
				bool realizable = singleFreeAngle ? c2.isRealizable<-EPSILON_TOLERANCE_MULTIPLIER>(possibleCollisions, blocks[ip2I].size) :
					(boostPrecision ? c2.isRealizable<-MOLDING_TOLERANCE_MULTIPLIER / PRECISION_BOOST_MULTIPLIER>(possibleCollisions, blocks[ip2I].size) :
						c2.isRealizable<-MOLDING_TOLERANCE_MULTIPLIER>(possibleCollisions, blocks[ip2I].size));
				if (realizable)
					SS->insert(smlI, full);
				else
					SS->insertEmpty(smlI);
			}
			if (!mDone) {
				if (c2.isRealizable<0>(possibleCollisions, blocks[ip2I].size))
					MM->insert(smlI, full);
				else
					MM->insertEmpty(smlI);
			}
			if (!lDone) {
				bool realizable = singleFreeAngle ? c2.isRealizable<EPSILON_TOLERANCE_MULTIPLIER>(possibleCollisions, blocks[ip2I].size) :
					(boostPrecision ? c2.isRealizable<MOLDING_TOLERANCE_MULTIPLIER / PRECISION_BOOST_MULTIPLIER>(possibleCollisions, blocks[ip2I].size) :
						c2.isRealizable<MOLDING_TOLERANCE_MULTIPLIER>(possibleCollisions, blocks[ip2I].size));
				if (realizable)
					LL->insert(smlI, full);
				else
					LL->insertEmpty(smlI);
			}
			++boosts[1];
			return;
		}

		const IConnectionPair icp(ip1, ip2);
		TurningBlockInvestigator tsbInvestigator(c, blocks[ip2I], ip2I, icp);

		// First check quick clear:
		if (!sDone && (singleFreeAngle ? tsbInvestigator.isClear<-EPSILON_TOLERANCE_MULTIPLIER>(possibleCollisions) :
			(boostPrecision ? tsbInvestigator.isClear<-MOLDING_TOLERANCE_MULTIPLIER / PRECISION_BOOST_MULTIPLIER>(possibleCollisions) :
				tsbInvestigator.isClear<-MOLDING_TOLERANCE_MULTIPLIER>(possibleCollisions)))) {
#ifdef _RM_DEBUG
			// Check that algorithms agree:
			geometry::IntervalList l;
			tsbInvestigator.allowableAnglesForBricks<-MOLDING_TOLERANCE_MULTIPLIER>(possibleCollisions, l);
			if (!geometry::isFullInterval(l, -MAX_ANGLE_RADIANS, MAX_ANGLE_RADIANS)) {
				MPDPrinter h;
				Model c3 = getModel(c, angleI, angleSteps[angleI]);
				h.add("isClearFail", &c3);
				h.print("isClearFail");

				tsbInvestigator.isClear<-MOLDING_TOLERANCE_MULTIPLIER>(possibleCollisions);
				assert(false);
			}
			assert(geometry::isFullInterval(l, -MAX_ANGLE_RADIANS, MAX_ANGLE_RADIANS));
#endif
			geometry::IntervalList full;
			full.push_back(geometry::Interval(-MAX_ANGLE_RADIANS, MAX_ANGLE_RADIANS));
			SS->insert(smlI, full);
			sDone = true;
		}
		if (!mDone && tsbInvestigator.isClear<0>(possibleCollisions)) {
			geometry::IntervalList full;
			full.push_back(geometry::Interval(-MAX_ANGLE_RADIANS, MAX_ANGLE_RADIANS));
			MM->insert(smlI, full);
			mDone = true;
		}
		if (!lDone && (singleFreeAngle ? tsbInvestigator.isClear<EPSILON_TOLERANCE_MULTIPLIER>(possibleCollisions) :
			(boostPrecision ? tsbInvestigator.isClear<MOLDING_TOLERANCE_MULTIPLIER / PRECISION_BOOST_MULTIPLIER>(possibleCollisions) :
				tsbInvestigator.isClear<MOLDING_TOLERANCE_MULTIPLIER>(possibleCollisions)))) {
			geometry::IntervalList full;
			full.push_back(geometry::Interval(-MAX_ANGLE_RADIANS, MAX_ANGLE_RADIANS));
			LL->insert(smlI, full);
			lDone = true;
		}
		if (sDone && mDone && lDone) {
			++boosts[2];
			return;
		}

		// Check using TSB:
		if (!sDone) {
			geometry::IntervalList l;
			if (singleFreeAngle)
				tsbInvestigator.allowableAnglesForBricks<-EPSILON_TOLERANCE_MULTIPLIER>(possibleCollisions, l);
			else if (boostPrecision)
				tsbInvestigator.allowableAnglesForBricks<-MOLDING_TOLERANCE_MULTIPLIER / 2>(possibleCollisions, l);
			else
				tsbInvestigator.allowableAnglesForBricks<-MOLDING_TOLERANCE_MULTIPLIER>(possibleCollisions, l);
			SS->insert(smlI, l);

#ifdef _RM_DEBUG
			//std::cout << smlI << ": " << l << std::endl;
			const unsigned short steps = 2 * angleSteps[angleI] + 1;
			bool *S = new bool[steps]; // deleted further down.
			geometry::intervalToArray(l, S, steps);

			int numDisagreements = 0;
			for (unsigned short i = 0; i < steps; ++i) {
				Model c2 = getModel(c, angleI, i);
				if (S[i] != c2.isRealizable<-MOLDING_TOLERANCE_MULTIPLIER>(possibleCollisions, blocks[ip2I].size)) {
					++numDisagreements;
				}
			}
			if (numDisagreements > 22) {
				std::cout << "Assertion error on S vs allowableAnglesForBricks." << std::endl;
				MPDPrinter h, d;

				std::cout << " Connection points: " << std::endl;
				for (unsigned int i = 0; i <= angleI; ++i) {
					const IConnectionPoint &ip1 = points[2 * i];
					const IConnectionPoint &ip2 = points[2 * i + 1];
					std::cout << "  " << ip1 << "-->" << ip2 << std::endl;
				}

				std::cout << "Content of S:" << std::endl;
				for (unsigned short j = 0; j < steps; ++j) {
					std::cout << (S[j] ? "X" : "-");
				}
				std::cout << std::endl;
				std::cout << "Using isRealizable on all angles:" << std::endl;
				bool first = true;
				for (unsigned short j = 0; j < steps; ++j) {
					Model c3 = getModel(c, angleI, j);
					bool realizable = c3.isRealizable<-MOLDING_TOLERANCE_MULTIPLIER>(possibleCollisions, blocks[ip2I].size);
					if (realizable != S[j]) {
						std::stringstream ss;
						ss << "fail_" << j;
						d.add(ss.str(), new Model(c3)); // OK Be cause we are about to die.
						if (first) {
							h.add("fail", &c3);
							h.print("assertion_fail");
							first = false;
						}
					}
					std::cout << (realizable ? "X" : "-");
				}
				first = true;
				std::cout << std::endl;
				d.print("disagreements");
				std::cout << "Number of disagreements: " << numDisagreements << ":" << std::endl;
				for (unsigned short j = 0; j < steps; ++j) {
					Model c3 = getModel(c, angleI, j);
					bool realizable = c3.isRealizable<-MOLDING_TOLERANCE_MULTIPLIER>(possibleCollisions, blocks[ip2I].size);
					if (realizable != S[j]) {
						if (first) {
							std::cout << "Model of first disagreement: " << c3 << std::endl;
							first = false;
						}
						const StepAngle angle((short)j - (short)angleSteps[angleI], angleSteps[angleI] == 0 ? 1 : angleSteps[angleI]);
						std::cout << " Angle step: " << j << ", radians: " << angle.toRadians() << (realizable ? ", IsRealizable OK, intervals not." : ", Intervals OK, isRealizable not") << std::endl;
					}
				}
				assert(false); std::cerr << "DIE X003" << std::endl;
				int *die = NULL; die[0] = 42;
			}
			delete[] S;
#endif
		}
		if (!mDone) {
			geometry::IntervalList l;
			tsbInvestigator.allowableAnglesForBricks<0>(possibleCollisions, l);
			MM->insert(smlI, l);
		}
		if (!lDone) {
			geometry::IntervalList l;
			if (singleFreeAngle)
				tsbInvestigator.allowableAnglesForBricks<EPSILON_TOLERANCE_MULTIPLIER>(possibleCollisions, l);
			else if (boostPrecision)
				tsbInvestigator.allowableAnglesForBricks<MOLDING_TOLERANCE_MULTIPLIER / 2>(possibleCollisions, l);
			else
				tsbInvestigator.allowableAnglesForBricks<MOLDING_TOLERANCE_MULTIPLIER>(possibleCollisions, l);
			LL->insert(smlI, l);
		}
		++boosts[3];
	}

	void AngleMapping::findIslands(std::vector<SIsland> &sIslands, bool &anyProblematic, const util::IntervalUnionFind &ufS, const util::IntervalUnionFind &ufM, const util::IntervalUnionFind &ufL) {
		// Add all S-islands:
		for (std::vector<uint32_t>::const_iterator it = ufS.rootsBegin(); it != ufS.rootsEnd(); ++it) {
			const uint32_t unionI = *it;
			MixedPosition rep;
			ufS.getRepresentativeOfUnion(unionI, rep);

			assert(ufS.getRootForPosition(rep) == unionI);
			SIsland sIsland(this, unionI, rep, ufS, ufM, ufL);
			if (sIsland.isProblematic())
				anyProblematic = true;
			sIslands.push_back(sIsland);

#ifdef _TRACE
			std::cout << sIsland << std::endl;
#endif
#ifdef _RM_DEBUG
			MPDPrinter h;
			int mIslandI = 0;
			for (std::vector<MIsland>::const_iterator itM = sIsland.mIslands.begin(); itM != sIsland.mIslands.end(); ++itM, ++mIslandI) {
				const MIsland &mIsland = *itM;

				Model c = getModel(mIsland.representative);
				std::stringstream ss;
				ss << "fail_" << mIslandI << "_" << mIsland << "_";
				util::TinyVector<AngledConnection, 5> cc;
				getModelConnections(mIsland.representative, cc);
				encoder.writeFileName(ss, cc, false);
				h.add(ss.str(), new Model(c)); // OK Be cause we are about to die.
			}
			std::stringstream ss;
			ss << "S-Island_";
			util::TinyVector<AngledConnection, 5> cc;
			getModelConnections(sIsland.representative, cc);
			encoder.writeFileName(ss, cc, true);
			h.print(ss.str());
#endif
		}
	}

	void AngleMapping::reportProblematic(const MixedPosition &p, int mIslandI, int mIslandTotal, int lIslandTotal, std::vector<util::TinyVector<AngledConnection, 5> > &manual, bool includeMappingFile) const {
		util::TinyVector<AngledConnection, 5> cc;
		getModelConnections(p, cc);

		manual.push_back(cc);
		// Report
		os << " Model requires manual verification!" << std::endl;

		// Special case: If this model contains no L-islands, but does contain a loop, then it is most likely OK:
		if (mIslandTotal == 1 && lIslandTotal == 0 && cc.size() > numAngles) {
			os << "  Special case for manual verification: Single M-island in S-island. With a loop, but without an L-island." << std::endl;
		}

		os << "  File: ";
		encoder.writeFileName(os, cc, true);
		os << std::endl;
		os << "  Angles: " << std::endl;
		for (unsigned int i = 0; i < numAngles - 1; ++i) {
			double radian = StepAngle(p.p[i] - angleSteps[i], angleSteps[i]).toRadians();
			os << "   " << (i + 1) << ": step " << p.p[i] << "/" << (2 * angleSteps[i] + 1) << ", fraction " << (p.p[i] - angleSteps[i]) << "/" << angleSteps[i] << ", radian " << radian << std::endl;
		}
		os << "   " << numAngles << ": radian " << p.lastAngle << std::endl;
		if (mIslandTotal > 0)
			os << "  This model represents M-island " << (mIslandI + 1) << "/" << mIslandTotal << ". There are " << lIslandTotal << " L-islands in this M-island" << std::endl;
		else
			os << "  S islands without M-islands inside!" << std::endl;
		os << std::endl;

		if (numAngles >= 3 || !includeMappingFile)
			return;

		//Create maping file:
		std::stringstream mappingFileName;
		mappingFileName << "manual\\" << numBricks << "\\";
		encoder.writeFileName(mappingFileName, cc, false);
		mappingFileName << "_mapping.txt";
		std::ofstream mappingFile;
		mappingFile.open(mappingFileName.str().c_str(), std::ios::out);
		mappingFile << numAngles << " ";
		for (unsigned int i = 0; i < numAngles - 1; ++i)
			mappingFile << " " << (2 * angleSteps[i] + 1);
		mappingFile << std::endl;

		// Print SML Mapping!
		if (numAngles == 1) {
			geometry::IntervalList s, m, l;
			SS->get(0, s);
			MM->get(0, m);
			LL->get(0, l);
			mappingFile << s << std::endl;
			mappingFile << m << std::endl;
			mappingFile << l << std::endl;
		}
		else {
			for (int i = 0; i < 2 * angleSteps[0] + 1; ++i) {
				geometry::IntervalList s, m, l;
				SS->get(i, s);
				MM->get(i, m);
				LL->get(i, l);
				mappingFile << s << std::endl;
				mappingFile << m << std::endl;
				mappingFile << l << std::endl;
			}
		}
		mappingFile.flush();
		mappingFile.close();
	}

	void AngleMapping::addFoundModel(const Model &c, bool rectilinear, std::set<uint64_t> &nonCyclic, std::set<Encoding> &cyclic, std::vector<Model> &modelsToPrint, counter &models, counter &rect, std::vector<std::pair<Model, Encoding> > &newRectilinear) {
		util::TinyVector<IConnectionPair, 8> found;
		bool checkRealizable = c.isRealizable<-MOLDING_TOLERANCE_MULTIPLIER>(found);
		if (!checkRealizable) {
			util::MPDPrinter h;
			h.add("ADD_FAIL", &c);
			h.print("ADD_FAIL");

			assert(false); std::cerr << "DIE NOT REALIZABLE ADD: " << c << std::endl;
			int *die = NULL; die[0] = 42;
		}
		bool isCyclic = found.size() > numAngles;
		Encoding encoding = encoder.encode(found);

		if (isCyclic) {
			if (cyclic.find(encoding) == cyclic.end()) {
				cyclic.insert(encoding);
				if (rectilinear) {
					newRectilinear.push_back(std::make_pair(c, encoding));
					++rect;
				}
				else {
					++models;
					//#ifdef _DEBUG
					modelsToPrint.push_back(c);
					//#endif
				}
			}
		}
		else {
			if (nonCyclic.find(encoding.first) == nonCyclic.end()) {
				nonCyclic.insert(encoding.first);
				if (rectilinear) {
					newRectilinear.push_back(std::make_pair(c, encoding));
					++rect;
				}
				else {
					++models;
					//#ifdef _DEBUG
					modelsToPrint.push_back(c);
					//#endif
				}
			}
		}
	}

	void AngleMapping::evalExtremeModels(unsigned int angleI, const Model &c, bool rectilinear, std::set<uint64_t> &nonCyclic, std::set<Encoding> &cyclic, std::vector<Model> &modelsToPrint, counter &models, counter &rect, std::vector<std::pair<Model, Encoding> > &newRectilinear) {
		// Find possible collisions:
		const IConnectionPoint &ip1 = points[2 * angleI];
		const IConnectionPoint &ip2 = points[2 * angleI + 1];
		unsigned int ip2I = ip2.first.modelBlockI;
		util::TinyVector<int, 5> possibleCollisions;
		c.getPossibleCollisions(blocks[ip2I], IConnectionPair(ip1, ip2), possibleCollisions);

		// Recursion:
		if (angleI + 1 < numAngles) {
			Model c2 = getModel(c, angleI, angleSteps[angleI]);
			if (c2.isRealizable<0>(possibleCollisions, blocks[ip2I].size))
				evalExtremeModels(angleI + 1, c2, rectilinear, nonCyclic, cyclic, modelsToPrint, models, rect, newRectilinear);

			if (angleSteps[angleI] != 0) {
				for (short i = 0; i <= 1; ++i) {
					c2 = getModel(c, angleI, 2 * i*angleSteps[angleI]);
					if (c2.isRealizable<0>(possibleCollisions, blocks[ip2I].size))
						evalExtremeModels(angleI + 1, c2, false, nonCyclic, cyclic, modelsToPrint, models, rect, newRectilinear);
				}
			}
			return;
		}

		// End of recursion:
		assert(angleI == numAngles - 1);
		const IConnectionPair icp(ip1, ip2);
		TurningBlockInvestigator tsbInvestigator(c, blocks[ip2I], ip2I, icp);

		// Speed up for noSML:
		if (angleTypes[angleI] == 0) {
			Model c2 = getModel(c, 0);
			if (!c2.isRealizable<0>(possibleCollisions, blocks[ip2I].size))
				return;
			addFoundModel(c2, rectilinear, nonCyclic, cyclic, modelsToPrint, models, rect, newRectilinear);
			return;
		}

		// First check quick clear:
		if (tsbInvestigator.isClear<0>(possibleCollisions)) {
			Model c2 = getModel(c, 0);
			addFoundModel(c2, rectilinear, nonCyclic, cyclic, modelsToPrint, models, rect, newRectilinear);
			return;
		}

		// Check using TSB:
		// First go for rectilinear:
		geometry::IntervalList l;
		tsbInvestigator.allowableAnglesForBricks<0>(possibleCollisions, l);
		// First check rectilinear:
		if (rectilinear && geometry::intervalContains(l, 0)) {
			Model c2 = getModel(c, 0);
			addFoundModel(c2, true, nonCyclic, cyclic, modelsToPrint, models, rect, newRectilinear);
		}

		// Then handle the non-rectilinear:
		for (const geometry::Interval* it = l.begin(); it != l.end(); ++it) {
			if (rectilinear && it->first <= 0 && 0 <= it->second)
				continue;
			double angle = (it->second + it->first) / 2;
			Model c2;
			if ((c2 = getModel(c, it->second)).isRealizable<0>(possibleCollisions, blocks[ip2I].size)) {
				angle = it->second;
			}
			else if ((c2 = getModel(c, it->first)).isRealizable<0>(possibleCollisions, blocks[ip2I].size)) {
				angle = it->first;
			}
			else
				c2 = getModel(c, angle);
			addFoundModel(c2, false, nonCyclic, cyclic, modelsToPrint, models, rect, newRectilinear);
		}
	}

	void AngleMapping::findNewExtremeModels(std::set<uint64_t> &nonCyclic, std::set<Encoding> &cyclic, std::vector<Model> &modelsToPrint, counter &models, counter &rect, std::vector<std::pair<Model, Encoding> > &newRectilinear) {
		assert(findExtremeAnglesOnly);
		time_t startTime, endTime;
		time(&startTime);

		// Evaluate:
		Model c(blocks[0]);
		evalExtremeModels(0, c, true, nonCyclic, cyclic, modelsToPrint, models, rect, newRectilinear);

		time(&endTime);
		double seconds = difftime(endTime, startTime);
		if (seconds > 2) {
			std::cout << "Extreme angle finding performed in " << seconds << " seconds for model sizes ";
			for (unsigned int i = 0; i < numAngles + 1; ++i)
				std::cout << blocks[i].size << " ";
			std::cout << "indices ";
			for (unsigned int i = 0; i < numAngles + 1; ++i)
				std::cout << blocks[i].index << " ";
			std::cout << std::endl;
		}
	}

	void AngleMapping::findNewModels(std::set<uint64_t> &nonCyclic, std::set<Encoding> &cyclic, std::vector<util::TinyVector<AngledConnection, 5> > &manual, std::vector<Model> &modelsToPrint, counter &models, std::vector<std::pair<Model, MIsland> > &newRectilinear, bool stopEarlyIfAnyProblematic, bool &anyProblematic) {
		assert(!findExtremeAnglesOnly);
		time_t startTime, endTime;
		time(&startTime);

		// Compute rectilinear index:
		rectilinearIndex = numAngles > 1 ? angleSteps[0] : 0;
		rectilinearPosition.p[0] = angleSteps[0];
		for (unsigned int i = 1; i < numAngles - 1; ++i) {
			const int push = 2 * angleSteps[i] + 1;
			rectilinearIndex = (rectilinearIndex * push) + angleSteps[i];
			rectilinearPosition.p[i] = angleSteps[i];
		}
		rectilinearPosition.lastAngle = 0;

		// Evaluate SML:
		Model c(blocks[0]);
		evalSML(0, 0, c, false, false, false);

		// Evaluate union-find:
		unsigned short sizes[MAX_DIMENSIONS - 1];
		for (unsigned int i = 0; i < numAngles - 1; ++i) {
			sizes[i] = 2 * angleSteps[i] + 1;
		}
		util::IntervalUnionFind ufS(numAngles, sizes, *SS);
		util::IntervalUnionFind ufM(numAngles, sizes, *MM);
		util::IntervalUnionFind ufL(numAngles, sizes, *LL);

		// Find islands:
		std::vector<SIsland> sIslands;
		anyProblematic = false;
		findIslands(sIslands, anyProblematic, ufS, ufM, ufL);

		if (stopEarlyIfAnyProblematic && anyProblematic) {
			return;
		}

		/* Perform analysis:
		Walk through islands in S and report M and L islands.
		- If no M island in an S-island: Report problematic. Don't count.
		- If more than one M-island, output problematic. Count all islands.
		- For each M island: Walk through island to find L-islands.
		 -- If no L-island. Report problematic. Count 1.
		 -- If more than one L-island: Report problematic. Still only count 1.
		*/
		std::set<uint64_t> newNonCyclic;
		std::set<Encoding> newCyclic;
		std::vector<Model> nrcs;

		for (std::vector<SIsland>::const_iterator it = sIslands.begin(); it != sIslands.end(); ++it) {
			const SIsland &sIsland = *it;
			if (sIsland.mIslands.size() == 0) { // No M-islands inside => problematic. No count.
				reportProblematic(sIsland.representative, 0, 0, 0, manual, true);
				continue;
			}

			int mIslandI = 0;
			bool anyMappingPrinted = false;
			for (std::vector<MIsland>::const_iterator itM = sIsland.mIslands.begin(); itM != sIsland.mIslands.end(); ++itM, ++mIslandI) {
				const MIsland &mIsland = *itM;

				Encoding encoding = mIsland.encoding;
				if (mIsland.isCyclic && cyclic.find(encoding) != cyclic.end())
					continue; // Already found. This can happen when there are cycles.

				Model model = getModel(mIsland.representative);
				if (mIsland.isRectilinear) {
					newRectilinear.push_back(std::make_pair(getModel(rectilinearPosition), mIsland));
				}

				// Multiple M-islands inside => problematic. Count only this M-island.
				// No L-islands => problematic, but still count.
				if (sIsland.mIslands.size() != 1 || mIsland.lIslands != 1) {
					reportProblematic(mIsland.representative, mIslandI, (int)sIsland.mIslands.size(), mIsland.lIslands, manual, !anyMappingPrinted);
					anyMappingPrinted = true;
				}
				else if (!mIsland.isRectilinear) {
#ifdef _DEBUG
					modelsToPrint.push_back(model);
#endif
					++models;
				}

				if (mIsland.isCyclic) {
					if (newCyclic.find(encoding) == newCyclic.end())
						newCyclic.insert(encoding);
				}
				else {
					if (newNonCyclic.find(encoding.first) == newNonCyclic.end())
						newNonCyclic.insert(encoding.first);
				}
			}
		}

		for (std::set<Encoding>::const_iterator it = newCyclic.begin(); it != newCyclic.end(); ++it) {
			cyclic.insert(*it);
		}
		for (std::set<uint64_t>::const_iterator it = newNonCyclic.begin(); it != newNonCyclic.end(); ++it) {
			nonCyclic.insert(*it);
		}

		time(&endTime);
		double seconds = difftime(endTime, startTime);
		if (seconds > 2) {
			std::cout << "Angle map finding performed in " << seconds << " seconds for model sizes ";
			for (unsigned int i = 0; i < numAngles + 1; ++i)
				std::cout << blocks[i].size << " ";
			std::cout << "indices ";
			for (unsigned int i = 0; i < numAngles + 1; ++i)
				std::cout << blocks[i].index << " ";
			std::cout << "connections";
			for (unsigned int i = 0; i < 2 * numAngles; i += 2)
				std::cout << " " << points[i] << "/" << points[i + 1];
			std::cout << std::endl;
		}
	}
}