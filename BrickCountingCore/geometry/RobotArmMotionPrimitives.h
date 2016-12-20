#ifndef GEOMETRY_ROBOT_ARM_MOTION_PRIMITIVES_H
#define GEOMETRY_ROBOT_ARM_MOTION_PRIMITIVES_H

#include "BasicGeometry.h"
#include <vector>

namespace geometry {
	struct AngleRestrictorFunction; // Forward declaration.
	typedef std::vector<AngleRestrictorFunction> AngleRestrictorFunctionList;

	struct Function {
		// TODO: Analyze requirements!
	};

	struct AngleRestrictorFunction {
		Interval primaryInterval;
		Function restrictionFunction;
	};

	struct TwinJointRobotArmMotionAnalyzer {
	private:
		double ab, bc; // Lengths |AB| and |BC|.

	public:
		TwinJointRobotArmMotionAnalyzer(double ab, double bc);

		/*
		Robot: Base A=(0,0), first joint -> B, second joint to C
		Obstacle: Circle E at (0,Ey) and radius r.
		*/
		void analyzePointToCircle(double Ey, double r, bool restrictBGivenA, AngleRestrictorFunctionList &restrictions);
	};
}

#endif GEOMETRY_ROBOT_ARM_MOTION_PRIMITIVES_H