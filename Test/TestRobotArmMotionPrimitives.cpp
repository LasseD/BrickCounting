#include "stdafx.h"
#include "../BrickCountingCore/geometry/RobotArmMotionPrimitives.h"

using namespace System;
using namespace System::Text;
using namespace System::Collections::Generic;
using namespace Microsoft::VisualStudio::TestTools::UnitTesting;

namespace Test
{
	[TestClass]
	public ref class TestRobotArmMotionPrimitives
	{
	private:
		TestContext^ testContextInstance;

	public: 
		/// <summary>
		///Gets or sets the test context which provides
		///information about and functionality for the current test run.
		///</summary>
		property Microsoft::VisualStudio::TestTools::UnitTesting::TestContext^ TestContext
		{
			Microsoft::VisualStudio::TestTools::UnitTesting::TestContext^ get()
			{
				return testContextInstance;
			}
			System::Void set(Microsoft::VisualStudio::TestTools::UnitTesting::TestContext^ value)
			{
				testContextInstance = value;
			}
		};

		#pragma region Additional test attributes
		//
		//You can use the following additional attributes as you write your tests:
		//
		//Use ClassInitialize to run code before running the first test in the class
		//[ClassInitialize()]
		//static void MyClassInitialize(TestContext^ testContext) {};
		//
		//Use ClassCleanup to run code after all tests in a class have run
		//[ClassCleanup()]
		//static void MyClassCleanup() {};
		//
		//Use TestInitialize to run code before running each test
		//[TestInitialize()]
		//void MyTestInitialize() {};
		//
		//Use TestCleanup to run code after each test has run
		//[TestCleanup()]
		//void MyTestCleanup() {};
		//
		#pragma endregion 

		[TestMethod]
		void TestAnalyzePointToCircleOutOfReach1()
		{
			double ab = 1, bc = 1, Ey = 3, radiusE = 1;
			geometry::AngleRestrictorFunctionList arfl;
			geometry::TwinJointRobotArmMotionAnalyzer analyzer(ab, bc);

			analyzer.analyzePointToCircle(Ey, radiusE, true, arfl);

			assert(arfl.empty());
		};

		[TestMethod]
		void TestAnalyzePointToCircleOutOfReach2()
		{
			double ab = 3, bc = 1, Ey = 1, radiusE = 1;
			//geometry::AngleRestrictorFunctionList arfl;
			//geometry::TwinJointRobotArmMotionAnalyzer analyzer(ab, bc);

			//analyzer.analyzePointToCircle(Ey, radiusE, true, arfl);

			//assert(arfl.empty());
		};
	};
}
