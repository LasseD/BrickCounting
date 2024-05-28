# Old Algorithm

This is a copy of the old code base for computing all rectilinear models of 1-6 LEGO bricks.

## How to run

Create the folders 2, 3, 4, 5, 6:

```
mkdir 2 3 4 5 6
```

Compile the code:

```
g++ -o2 *.cpp -o run.o
```

Run the code:

```
./run.o
```

It takes some time for this code to compute all combinations for 6 bricks due to the code being optimized for minimal memory usage. Expect up to 3 hours of running time!