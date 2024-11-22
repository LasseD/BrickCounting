# Rectilinear Algorithm

This is a code base for computing all rectilinear models of up to 11 LEGO bricks.

## How to run

Create the folders for output models:

```
mkdir 2 3 4 5 6 7
```

Compile the code:

```
g++ -Ofast -DNDEBUG *.cpp -o run.o
```

Count up to 6 bricks:

```
./run.o
```

Count for a specific refinement, such as <422>

```
./run.o 422
```

Count and save the models for a specific refinement, such as <222>

```
./run.o 222 SAVE
```

There are optimizations for some refinements, but not for all. Running times are thus not comparable between the types of refinements.
The code is in public domain, and you may copy and add to it as you see fit.

## Code Overview

The code computes all models a(X) and saves them in files on disk:

- Files are grouped in refinements.

- Refinements are computed one by one.

- A refinement a(X,Y,Z1,Z2,Z3,...), shortly written as <Z1Z2Z3...> is computed from the refinements a(X-1,Y1,Z1-1,Z2,Z3,...), a(X-1,Y2,Z1,Z2-1,Z3,...), ... by trying to place a brick in the “reduced” layer of the smaller models.

- A set of all discovered models of a(X,Y,Z1,Z2,Z3,...) was kept in memory and used to ignore duplicates.


This code has some obvious inefficiencies:

- All models of a refinement are in memory when computing them.

- The code is single threaded.

- Layer-symmetries are not culled, causing nearly double the required amount of work to be performed.

- All models, including a(X,X), a(X,X-1) and a(X,X-2) are computed by brute force, disregarding the insights discovered below.

- Standard sorting is used when rotating models.

- Only a minimal amount of care is taken to reduce disk space usage: The initial brick of a model is not stored (assumed placed horizontally at 0,0,0) and bricks are stored in 2 bytes.


## Proposals for improvements

### File Compression

When constructing models from a given base model, only the base model and the locations of the additional bricks are save dto disk, thus saving more than 50% of disk space for most refinements.

### Removing the Need for Keeping Models in Memory

For simplicity, let's assume we build models of height 4: a(X,4,Z1,Z2,Z3,Z4). The method here works for models of all refinements.

Models are constructed by adding a single brick to each model from the following "pre-refinements":

1) a(X-1,4,Z1-1,Z2,Z3,Z4) for Z1 > 1

2) a(X-1,4,Z1,Z2-1,Z3,Z4)

3) a(X-1,4,Z1,Z2,Z3-1,Z4)

4) a(X-1,4,Z1,Z2,Z3,Z4-1) or a(X-1,3,Z1,Z2,Z3) if Z4 is 1.

A model in a(X,4,Z1,Z2,Z3,Z4) can potentially be constructed in multiple ways, so we have previously been checking for duplicates against all constructed models.

The following observations are made to remove the need for keeping models in memory:

- A model should only be counted when the model from which it is constructed is "first"

- We declare the models to come before another if its pre-refinement comes before the pre-refinement of the other in the numbered list above.

- Models in the same refinement are comparable and we seek the first among these.

### Special handling for <X2...>, X > 1

See the method countX2() in rectilinear.h.

For the refinements with at least 2 bricks in the first layer and exactly 2 bricks in the second, we can speed up the computation by the following observation:

Since there are only 2 bricks in second layer, at most one brick from the first layer is used to connect all bricks in second layer. It thus suffices to observe the models of the refinement <12...> and add X-1 bricks to the first layer.

Since the two bricks of the second layer "isolate" the bricks above, a lookup can be used to reuse previous results for counting for a specific configuration of the two bricks of the second layer (plus the location for the brick in the first layer, as well as information about symmetry and if the two bricks of the second layer are connected by the bricks above them).

Special care for counting has to be taken for symmetries and the case where the two bricks of the second layer are not connected by the bricks above.

See the code base for details.

By using countX2(), massive improvements to running time have been observed.

### Special handling of <X3...>, X > 2

The analysis for <X2...> can be expanded to <X3...> by observing <23...> includes all configurations of the three bricks of the second layer.

#### Detailed Analysis of <X3...>

The method of countX2() uses the knowledge of how many times a specific model is being encountered (or counted) to derive the correct result.

Let A, B denote the two bricks of the first layer, while C, D, E are the three bricks of the second layer of a model in <23..>. Consider the bricks of TOP in <3...> consisting of C-E and all bricks in layers above, while F (for FULL) in <X3...> is the full model including A-B and the R remaining bricks of the first layer (so X = 2 + R).

1) If TOP is not connected, then either A, B or both are needed to ensure connectivity of F. Ie. Removing one of them can cause bricks to no longer be connected.



OLD BELOW

1) If C-E are all connected above, then A-B can be placed without care for connecting the bricks C-E: 

1.1) If TOP is symmetric, then F is counted once for each brick in the first layer, except those that have an identical "sibling" under rotation.

1.2) If TOP is _not_ symmetric, then F is counted once for each brick in first layer.

2) If one of the bricks C-E (let's say C) is connected in TOP, while D, E are not, then either A, B or both connect to D and E (as otherwise the model would not be connected).

2.1) If TOP is symmetric, then


TODO:

"top connected" needs to be more precise: top connection needs 0, 1, ... X-1 bricks.
X = 3:
Bricks A,B,C so either AB, BC or ABC connection required.
X = 4:
Bricks A,B,C,D: AB, AC, AD, BC, BD, ABC, ABD, BCD, AB+CD, AC+BD, ABCD
CONCLUSION: Not controllable as input division!

So required in Configuration that is used as key!

