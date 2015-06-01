# BrickCounting
Count all combinations of six 2x4 LEGO bricks

Paper at http://c-mt.dk/counting

Findings:

Strongly Connected Configurations (SCC):

- Size 1: 1

- Size 2: 20

- Size 3: 1.004

- Size 4: 58.862

- Size 5: 3.817.291

- Size 6: 261.534.637

Rectilinear configurations (excluding SCC's. Should match the previous results)

- Size 2: 4 new, for a total of 24 rectilinear configurations, matching previous results.

- Size 3: 556 new, for a total of 1.560, matching previous results.

- Size 4: 60.718 new, for a total of 119.580, matching previous results.

- Size 5: 6.349.112 new, for a total of 10.166.403, matching previous results.

- Size 6: 653.569.128 new, for a total of 915.103.765, matching previous results.
 
Distinctly corner connected SCCs (groups models on the set of connection points)

- Size 4: At least 552.

- Size 5: At least 116.998.

- Size 6: At least 19.701.710.

Models:

- Size 4: In progress...

- Size 5: In progress...

- Size 6: In progress...

Models requiring manual verification:

- Size 4: In progress...

- Size 5: In progress...

- Size 6: In progress...


TODO: 

Construct all models using SCCs turned at angles (distinctly corner connected SCCs are a sub-result of finding the models). 

Getting to this point requires the following tasks:

- Expand modelling of bricks to the size sets 'S' (brick dimensions 15.6mm x 31.6mm), 'M' (brick dimensions 15.8mm x 31.8mm), and 'L' (brick dimensions 16mm x 32mm).

- Extract model information from mapping of the three size sets.

- Construct humanly readable information for any model requiring manual verification.

- Optimize code to allow computations for models of size 5 and 6:

 - Add timing information to code to track progression of performance.

 - Use profiling to identify places to improve performance.

 - Add multi processor support.


Geometry:

- P = 8mm

- Width of brick: 15.8mm

- Length of brick: 31.8mm

- Stud: 4.8mm

- Stud to side: 3.9mm

- Maximum angle for a turn point is thus A = acrcos(6.3/8) = 0.664054277 radians (38.0475075 degrees). 


Maximal angle steps based on number of brick of models on each side of the turn point:

Simple formulas for an isosceles triangle implies the bound a on the angle that can be allowed given the distance d (in mm):

sin(a/2)*d < 0.1/2 => a < asin(1/20/d)*2

- Size 1: Max distance: d=sqrt(12^2+28^2)=30.46mm. Max angle: a<asin(1/20/d)*2=0.0032826623  radians. Steps: A/a < 203 => 2*203+1=407 steps in total.

- Size 2: Max distance: d=sqrt(20^2+52^2)=55.71mm. Max angle: a<asin(1/20/d)*2=0.00179489564 radians. Steps: A/a < 370 => 2*370+1=741 steps in total.

- Size 3: Max distance: d=sqrt(28^2+76^2)=80.99mm. Max angle: a<asin(1/20/d)*2=0.00123466207 radians. Steps: A/a < 538 => 2*538+1=1077 steps in total.
