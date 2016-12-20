
# Source Code Documentation

## Overview

This page gives a high level introduction to the documentation by providing an overview of the directory structure, files, and descriptions of each component (class, struct and typedef) within the files.

## Structure and Content of Directories

* /Test: Contains unit tests on a file-by-file basis. For example. The methods in "RobotArmMotionPrimitives.h" and "RobotArmMotionPrimitives.h" are tested in "TestRobotArmMotionPrimitives.cpp". All methods of the files in "/BrickCountingCore" should be tested unless there is a good reason not to. These reasons must be presented in the test files.

* /BrickCountingRunner: Contains the main entry point for the software in "BrickCountingRunner.cpp".

* /BrickCountingCore: Main directory of source files. Further divided into directories:
* * /geometry: Helpers focused on geometry. There should be no mention of bricks or the geometry and construction of bricks in here.
* * /configuration: TODO Brick, Configuration, ...
* * /movement: TODO
* * /counting: TODO
* * /util: Utility classes such as data structures
* * /visualization: Classes focused on visualization using LDraw.

## Individual Component Description

This section contains a high level description of each component in the main directory (/BrickCountingCore). The components are grouped by directory/namespace and ordered so that the dependencies of a component are handled before the component itself.

### Util

Util (short for "Utilities") is a collection of general utility classes that can be used anywhere. The classes might even be useful for other projects.

#### TinyVector (class defined in TinyVector.hpp)

TinyVector<T, CAPACITY> Represents a generic vector with elements of type T and a capacity (maximal size) of CAPACITY provided by generic argument. The minimal public interface reflects that of std::vector<T>. TinyVector can be used in place of std::vector in hot loops where the maximal number of elements is known. It offers a significant performance benefit as there is no concurrency considerations, resizing or pointers. It is simply a thin wrapper around an array. TinyVector was created when it was found that most of the running time of an early version of the software was spent on thread locking and new/delete calls from std::vectors.

#### ProgressWriter (class defined in ProgressWriter.hpp)

ProgressWriter offers simple pretty printing progress for a task with a given number of steps. If all steps take roughly the same amount of time, then the predicted remaining time will be a good estimate.

### Geometry

Components in the geometry directory/namespace 


* Util: Split to "TinyVector.hpp" and "ProgressWriter.hpp". Move to util directory (new), use namespace "util".
* Math: Rename to geometry/BasicGeometry and use geometry namespace.
* UnionFind: DELETE!
* TurningSingleBrick: TODO: Structure for "counters" - also include managers here?
