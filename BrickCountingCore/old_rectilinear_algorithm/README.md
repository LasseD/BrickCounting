# Old Algorithm

This is a copy of the old code base for computing all rectilinear models of 1-6 LEGO bricks.

## How to run

Create the folders 2, 3, 4, 5, 6:

```
mkdir 2 3 4 5 6
```

Compile the code:

```
g++ -o2 -DNDEBUG *.cpp -o run.o
```

Run the code:

```
./run.o
```

It takes some time for this code to compute all combinations for 6 bricks due to the code being optimized for minimal memory usage. Expect up to 3 hours of running time!

## Code Overview

This code computes all models a(X) and saves them in files on disk:

- Files are grouped in refinements.

- Refinements are computed one by one.

- A refinement a(X,Y,Z1,Z2,Z3,...) is computed from the refinements a(X-1,Y1,Z1-1,Z2,Z3,...), a(X-1,Y2,Z1,Z2-1,Z3,...), ... by trying to place a brick in the “reduced” layer of the smaller models.

- A set of all discovered models of a(X,Y,Z1,Z2,Z3,...) is kept in memory and used to ignore duplicates.


This code has some obvious inefficiencies:

- All models of a refinement are in memory when computing them.

- The code is single threaded.

- Layer-symmetries are not culled, causing nearly double the required amount of work to be performed.

- All models, including a(X,X), a(X,X-1) and a(X,X-2) are computed by brute force, disregarding the insights discovered below.

- Standard sorting is used when rotating models.

- Only a minimal amount of care is taken to reduce disk space usage: The initial brick of a model is not stored (assumed placed horizontally at 0,0,0) and bricks are stored in 2 bytes.


## Proposals for improvements

### File Compression

TODO

### Removing the Need for Keeping Models in Memory

For clarity, let's assume we build models of height 4: a(X,4,Z1,Z2,Z3,Z4).

Models are constructed by adding a single brick to each model from the following "pre-refinements":

1) a(X-1,4,Z1-1,Z2,Z3,Z4)

2) a(X-1,4,Z1,Z2-1,Z3,Z4)

3) a(X-1,4,Z1,Z2,Z3-1,Z4)

4) a(X-1,4,Z1,Z2,Z3,Z4-1)

With "4" being replaced with "3" if Z1 or Z4 are 1.

A model in a(X,4,Z1,Z2,Z3,Z4) can potentially be constructed in multiple ways, so we have previously been checking for duplicates against all constructed models.

The following observations are made to remove the need for keeping models in memory:

- A model should only be counted when the model from which it is constructed is "first"

- We declare the models to come before another if its pre-refinement comes before the pre-refinement of the other in the numbered list above.

- Models in the same refinement are comparable and we seek the first among these.

