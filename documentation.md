
# Source Code Documentation

## Overview

This page gives a high level introduction to the documentation by providing an overview of the directory structure, files, and descriptions of each component (class, struct and typedef) within the files.

## Structure and Content of Directories

* /Test: Contains unit tests on a file-by-file basis. For example. The methods in "RobotArmMotionPrimitives.h" and "RobotArmMotionPrimitives.h" are tested in "TestRobotArmMotionPrimitives.cpp". All methods of the files in "/BrickCountingCore" should be tested unless there is a good reason not to. These reasons must be presented in the test files.

* /BrickCountingRunner: Contains the main entry point for the software in "BrickCountingRunner.cpp".

* /BrickCountingCore: Main directory of source files. Further divided into directories:
* * /counting: Managers for counting blocks, homotopies and configuration.
* * /geometry: Helper classes and functions focused on geometry. There should be no mention of bricks or the geometry and construction of bricks in here.
* * /modelling: Components for representations of bricks, blocks, and models.
* * /util: Utility classes such as data structures and for visualization using LDraw.

## Individual Component Description

This section contains a high level description of each component in the main directory (/BrickCountingCore). The components are grouped by directory/namespace.

### Counting TODO

- AngleMapping(+MIsland,SIsland)
- ConfigurationManager

### Geometry

The Geometry namespace contains geometric structures and functions.

#### BasicGeometry (files BasicGeometry.h and BasicGeometry.cpp)

All basic geometric structures and functions are found in the BasicGeometry files. This includes representations of points, intervals, line segments, and functions such as for finding intersections between circles and half planes.

#### RobotArmMotionPrimitives (files RobotArmMotionPrimitives.h and RobotArmMotionPrimitives.cpp)

TODO! This is the starting point for implementing phase 2.

### Modelling TODO

- Brick
- Configuration (TODO: Perhaps rename?)
- ConfigurationEncoder
- ConnectionPoint

### Util

The Util (short for "Utilities") namespace contains a collection of general utility classes that can be used anywhere. The classes might even be useful for other projects.

#### LDRPrintable ("interface" defined in LDRPrintable.h)

The interface (C++ class with a virtual un-implemented function) LDRPrintable is used to allow for printing of models to the LDR (LDraw) format. 

#### MPDPrinter (class defined in MPDPrinter.h and MPDPrinter.cpp)

MPDPrinter is a class that can write an MPD file with a set of LDRPrintable objects. It is used for visualizing the models that are found.

#### ProgressWriter (class defined in ProgressWriter.hpp)

ProgressWriter offers simple pretty printing progress for a task with a given number of steps. If all steps take roughly the same amount of time, then the predicted remaining time will be a good estimate.

#### TinyVector (class defined in TinyVector.hpp)

TinyVector<T, CAPACITY> Represents a generic vector with elements of type T and a capacity (maximal size) of CAPACITY provided by generic argument. The minimal public interface reflects that of std::vector<T>. TinyVector can be used in place of std::vector in hot loops where the maximal number of elements is known. It offers a significant performance benefit as there is no concurrency considerations, resizing or pointers. It is simply a thin wrapper around an array. TinyVector was created when it was found that most of the running time of an early version of the software was spent on thread locking and new/delete calls from std::vectors.






TODO:
- Rewrite UnionFind to actually use union find! https://en.wikipedia.org/wiki/Disjoint-set_data_structure


Definitions:

Brick: 2x4
Block: Start with a brick. Add more bricks by sharing at least 2 studs with a brick already in the block.
"Block set: Given blocks."
Connection point
Model: blocks put together at connection points
 Non-rectilinear model 
 Rectilinear model
Model size: Number of bricks.
Model type: 4/2/1 
Homotopy
Configuration: Way to put together set of blocks.
Goals:
 - Compute homotopies and configurations Non-