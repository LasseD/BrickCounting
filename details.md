## Geometry:

- P = 8mm

- Width of brick: 15.8mm

- Length of brick: 31.8mm

- Stud: 4.8mm

- Stud to side: 3.9mm

- Maximum angle for a turn point is thus A = acrcos(6.3/8) = 0.664054277 radians (38.0475075 degrees). 


Maximal angle steps based on number of brick of models on each side of the turn point:

Simple formulas for an isosceles triangle implies the bound A on the angle that can be allowed given the distance d (in mm):

sin(A/2)*d < 0.1/2 => A < asin(1/20/d)*2

- Size 1: Max distance: d=sqrt(12^2+28^2)=30.46mm. Max angle: a<asin(1/20/d)*2=0.0032826623  radians. Steps: A/a < 203 => 2*203+1=407 steps in total.

- Size 2: Max distance: d=sqrt(20^2+52^2)=55.71mm. Max angle: a<asin(1/20/d)*2=0.00179489564 radians. Steps: A/a < 370 => 2*370+1=741 steps in total.

- Size 3: Max distance: d=sqrt(28^2+76^2)=80.99mm. Max angle: a<asin(1/20/d)*2=0.00123466207 radians. Steps: A/a < 538 => 2*538+1=1077 steps in total.


## Optimizations

The times (in seconds) of this section are measured by finding models with the program running in debug mode. "i7" and "i5" refer to test machines. "i7" is a Lenovo Thinkpad T440p from 2014 while "i5" is a Sony Vaio VPCEB4X1E from 2011 running in low performance power saving mode. 

Different tests are used to give insight into the performance improvements of the various optimizations.

### Test with 3 bricks

![Configuration consisting of 3 bricks](http://c-mt.dk/counting/images/test3brickssmall.png "An example of how to connect 3 bricks at the corners")

The following improvements are for finding all models with 3 bricks. Notice that no models are actually found in this test.

| Optimization           | i7    | i5  |
|:-----------------------|------:|----:|
| None | 180 | - |
| Replacing diagonal union-find merging with angle locking | 153 | 212 |
| Compute indices for SML-mapping dynamically | - | 202 |
| Compute configuration for SML-mapping dynamically | - | 199 |
| Precompute bricks that might intersect SML-mapping | - | 96 |
| Don't consider connecting brick to be intersectable | - | 62 |
| Split handling of the SML sets and detect early if a model is impossible | - | 50 |
| Special handling for models with turning single brick SCCs (TSB) at the end of model: Speed up when angle is free | 6 | 11 |
| Modify the SML-result set to accomodate intervals from TSBs | 2 | 3 |
| Run using release-build | 0 | 1 |

## Further details

To be added.