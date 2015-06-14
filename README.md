# All Combinations of Six 2x4 LEGO Bricks

Project page at http://c-mt.dk/counting

## Teminology

* Brick: A standard "2 by 4" LEGO brick with 8 studs on top.
* Configuration: Some bricks connected by their studs.
* Strongly Connected Configuration (SCC): The configuration consisting of one brick is a SCC. An SCC with N bricks is made by adding a brick to an SCC with N-1 bricks where there is a brick connecting to at least 2 studs on the new brick.
* Rectilinear configuration (RC): A configuration where all bricks are connected at right angles. 
* Non-rectilinear configuration (NRC): A configuration with odd angles.
* Model: A set of congirutations with the property that any configuration in the set can be turned into all the other configurations simply by turning bricks at turn points. 

## Findings

|   Number of bricks | 1 | 2 | 3 | 4 | 5 | 6 | 
|:------------------:|--:|--:|--:|--:|--:|--:|
| **SCC** | 1 | 20 | 1.004 | 58.862 | 3.817.291 | 261.534.637
| **RC (exclusing SCC)** | 0 | 4  | 556   | 60.718  | 6.349.112  | 653.569.128 |
| **RC**                  | 1 | 24 | 1.560 | 119.580 | 10.166.403 | 915.103.765 |
| **NRC (* means at least)**               | 0 | 0 | 0 | *552 | *116.998 | *19.701.710 |
| **Models (? for unknown)**               | 0 | 0 | 0 | ? | ? | ? |
| **Models requiring manual verification** | 0 | 0 | 0 | ? | ? | ? |


## Current optimization progress

- Initial program running time for models of size 3: 180 seconds (debug mode).


## Geometry:

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


