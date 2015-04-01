# BrickCounting
Count all combinations of six 2x4 LEGO bricks

See paper on-line at at http://c-mt.dk/counting

Findings:

 StronglyConnectedComponents (SCC):

  Size 1: 1

  Size 2: 20

  Size 3: 1004

  Size 4: 58862

  Size 5: 3817291

  Size 6: 261534637

 Rectilinear configurations (excluding SCC's. Should match the previous results)

  Size 2: 4

  Size 3: In progress...
 
TODO:

1: Construct all possible Rectilinear configurations (Super set) CURRENTLY IN PROGRESS

2: Construct all models using SCCs turned at angles.


Geometry:
 P = 8mm
 Width of brick: 15.8mm
 Length of brick: 31.8mm
 Stud: 4.8mm
 Stud to side: 3.9mm
 Maximum angle for a turn point is thus acrcos(6.3/8) = 0.664054277 radians (38.0475075 degrees). 

 Using a byte for angle means precision of
  0.664054277/127 = 0.00522877383
 Consider the distance between two studs of models consisting of 3 bricks. It is less than sqrt((3*8)^2+(9*8)^) < 75.9mm Using two of such models with a shared turn point means that the distance of two studs can at most change 2*sin(0.00522877383)*75.9 = 0.79 mm between two models. 
