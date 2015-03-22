# BrickCounting
Count all combinations of six 2x4 LEGO bricks

See paper on-line at at http://c-mt.dk/counting

Findings:

 StronglyConnectedComponents:

  Size 1: 1 (100% of all rectilinear configurations)

  Size 2: 20 (83% of all rectilinear configurations)

  Size 3: 1027 (66% of all rectilinear configurations)

  Size 4: 62582 (52% of all rectilinear configurations)

  Size 5: 4178600 (41% of all rectilinear configurations)

  Size 6: 287500793 (31% of all rectilinear configurations)


 
TODO:

1: Construct all possible SCCs (StronglyConnectedConfigurationBuilder) DONE SEE RESULTS ABOVE

2: Construct all possible Rectilinear configurations (Super set)

3: Construct all possible configurations using SCCs.

 3a: First for connections, no circles, simple check "isRelializable"

 3b: Add different models with same circles (motion map)

 3c: Add circles (near angle search)

Detailed TODO:
2:
 Overview: Combine all combinations of SCCs at corners to form all RCs.
 Details: 
  Use Brick type: 
   - Vertical at origin is like base RectilinearBrick: Angle is from vertical. ConnectionPoints are from angle.
   - Built from RectilinearBrick: 
    - Base: Vertical is angle=0, x=0, y=0
     	   Horizontal is angle=-PI/2, x+=1, y-=1


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
