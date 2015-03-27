# BrickCounting
Count all combinations of six 2x4 LEGO bricks

See paper on-line at at http://c-mt.dk/counting

Findings:

 StronglyConnectedComponents:

  Size 1: 1

  Size 2: 20

  Size 3: 1004

  Size 4: 58862

  Size 5: 3817291

  Size 6: 261534637


 
TODO:

1: Construct all possible SCCs (StronglyConnectedConfigurationBuilder) DONE SEE RESULTS ABOVE

Written 5554 unsorted elements of hash 1125 to stream. Now counting.
Read 2790 sorted elements of hash 1125
Written 4486 unsorted elements of hash 1175 to stream. Now counting.
Read 2242 sorted elements of hash 1175
Written 15344 unsorted elements of hash 1215 to stream. Now counting.
Read 9244 sorted elements of hash 1215
Written 478 unsorted elements of hash 1355 to stream. Now counting.
Read 154 sorted elements of hash 1355
Written 12306 unsorted elements of hash 1715 to stream. Now counting.
Read 7819 sorted elements of hash 1715
Written 8632 unsorted elements of hash 2115 to stream. Now counting.
Read 2790 sorted elements of hash 2115
Written 2678 unsorted elements of hash 2255 to stream. Now counting.
Read 1176 sorted elements of hash 2255
Written 2044 unsorted elements of hash 2755 to stream. Now counting.
Read 958 sorted elements of hash 2755
Written 605 unsorted elements of hash 3155 to stream. Now counting.
Read 254 sorted elements of hash 3155
Written 7298 unsorted elements of hash 7115 to stream. Now counting.
Read 2145 sorted elements of hash 7115
Written 2492 unsorted elements of hash 7255 to stream. Now counting.
Read 1637 sorted elements of hash 7255
Written 2031 unsorted elements of hash 7755 to stream. Now counting.
Read 1034 sorted elements of hash 7755
Number of StronglyConnectedConfigurations of size 4: 59683


Written 478 unsorted elements of hash 1355 to stream. Now counting.
Read 154 sorted elements of hash 1355
Written 605 unsorted elements of hash 3155 to stream. Now counting.
Read 254 sorted elements of hash 3155




Written 10040 unsorted elements of hash 1120 to stream. Now counting.
Read 5032 sorted elements of hash 1120
Written 15930 unsorted elements of hash 2110 to stream. Now counting.
Read 4935 sorted elements of hash 2110

Written 27650 unsorted elements of hash 1210 to stream. Now counting.
Read 17063 sorted elements of hash 1210
Written 9245 unsorted elements of hash 2200 to stream. Now counting.
Read 3917 sorted elements of hash 2200
Number of StronglyConnectedConfigurations of size 4: 58795




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
