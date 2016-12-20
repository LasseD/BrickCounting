#include "RobotArmMotionPrimitives.h"

namespace geometry {
	TwinJointRobotArmMotionAnalyzer::TwinJointRobotArmMotionAnalyzer(double ab, double bc) : ab(ab), bc(bc) {}

	void TwinJointRobotArmMotionAnalyzer::analyzePointToCircle(double Ey, double radiusE, bool restrictBGivenA, AngleRestrictorFunctionList &restrictions) {
		// TODO!
	}
}