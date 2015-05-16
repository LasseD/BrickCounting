# BrickCounting
Count all combinations of six 2x4 LEGO bricks

Paper at http://c-mt.dk/counting

Findings:

 StronglyConnectedComponents (SCC):

  Size 1: 1

  Size 2: 20

  Size 3: 1004

  Size 4: 58862

  Size 5: 3817291

  Size 6: 261534637

 Rectilinear configurations (excluding SCC's. Should match the previous results)

  Size 2: 4 new, for a total of 24 rectilinear configurations, matching previous results.

  Size 3: 556 new, for a total of 1560.

  Size 4: 60718 new, for a total of 119580.

  Size 5: 6349112 new, for a total of 10166403.

  Size 6: In progress...
 
 Distinctly corner connected strongly connected configurations (groups models on the set of connection points)

  Size 4: In progress...

  Size 5: In progress...

  Size 6: In progress...

 Models:

  Size 4: In progress...

  Size 5: In progress...

  Size 6: In progress...

 Models requiring manual verification:

  Size 4: In progress...

  Size 5: In progress...

  Size 6: In progress...


TODO: 

Construct all models using SCCs turned at angles (distinctly corner connected strongly connected configurations are a sub-result of finding the models). 

Getting to this point requires the following tasks:

- Construct angle mapping structure

- Expand modelling of bricks to the size sets 'S' (brick dimensions 15.6mm x 31.6mm), 'M' (brick dimensions 15.8mm x 31.8mm), and 'L' (brick dimensions 16mm x 32mm).

- Perform angle mapping for each possible model and for size sets S, M and L.

- Extract model information from mapping of size sets.

- Construct humanly readable and verification information for any model requiring manual verification.

- Optimize code to allow computations for models of size 5 and 6:

-- Add timing information to code to track progression of performance.

-- Use profiling to identify places to improve performance.

-- Add multi processor support.


Geometry:
 P = 8mm
 Width of brick: 15.8mm
 Length of brick: 31.8mm
 Stud: 4.8mm
 Stud to side: 3.9mm
 Maximum angle for a turn point is thus acrcos(6.3/8) = 0.664054277 radians (38.0475075 degrees). 


Using a byte for angle means precision of 0.664054277/127 = 0.00522877383 Consider the distance between two studs of models consisting of 3 bricks. It is less than sqrt((38)^2+(98)^) < 75.9mm Using two of such models with a shared turn point means that the distance of two studs can at most change 2sin(0.00522877383)75.9 = 0.79 mm between two models