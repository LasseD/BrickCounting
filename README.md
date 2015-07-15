# All Combinations of Six 2x4 LEGO Bricks

[Main project page](http://c-mt.dk/counting)

## Motivation

[Previous results](http://www.math.ku.dk/~eilers/lego.html) count combinations where 2x4 LEGO bricks are connected at straight and right angles.

![Bricks connected at right angles](http://c-mt.dk/counting/images/rectilinearintrosmall.png "There are 915.103.765 ways to combine 6 bricks at straight and right angles")

We seek to find the number of ways six bricks can be connected when not restricting the angles.

![Bricks connected at odd angles](http://c-mt.dk/counting/images/modelsintrosmall.png "It is currently unknown how many ways 6 bricks can be combined at other angles")


## Terminology

* Brick: A standard "2 by 4" LEGO brick with 8 studs on top.
* Configuration: Some bricks connected by their studs. The pictures above each show three configurations.
* Strongly Connected Configuration (SCC): The configuration consisting of one brick is an SCC. An SCC with N bricks is made by adding a brick b to an SCC s with N-1 bricks so that there is a brick in s connecting to at least 2 studs of b. 

![SCCs](http://c-mt.dk/counting/images/sccconstructionsmall.png "Notice that for any SCC with more than one brick, any additional brick has to connect to a single other brick using at least two studs")

* Rectilinear configuration (RC): A configuration where all bricks are connected at right angles. The top-most image above shows three different RCs.
* Non-rectilinear configuration (Non-RC): A configuration with odd angles, such as shown in the second image above.
* Model: A set of Non-RC's with the property that any configuration in the set can be turned into any other combination of the set simply by turning bricks at turn points. 

## Findings

|  Bricks    | 1 |  2 |     3 |       4 |          5 |           6 | 
|:----------:|--:|---:|------:|--------:|-----------:|------------:|
| **SCC**    | 1 | 20 | 1.004 |  58.862 |  3.817.291 | 261.534.637 |
| **RC**     | 1 | 24 | 1.560 | 119.580 | 10.166.403 | 915.103.765 |
| **Models** | 0 |  0 |     0 | At least 552 | At least 116.998 | At least 19.701.710 |

## Current optimization progress

The times are measured (in seconds) by finding models with the program running in debug mode. "i7" is a Lenovo Thinkpad T440p from 2014 while "i5" is a Sony Vaio VPCEB4X1E from 2011 running in low performance power saving mode. 

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

### Test with 4 bricks

![Configuration consisting of 4 bricks](http://c-mt.dk/counting/images/test4brickssmall.png "An example of how to connect 3 SCCs at the corners")

The following improvements are for finding models with 4 bricks for a given configuration. This configuration consists of an SCC with two bricks connected by 6 studs, and two SCCs with a single brick each.

| Optimization           | i7    | i5  |
|:-----------------------|------:|----:|
| The best performance achieved in the test above | 49 | - |
| Improve TSB handling by using SML-mapping for moving studs intersecting bricks | 49 | - |
| Improve TSB handling by using SML-mapping for moving bricks intersecting studs | - | - |
| Improve TSB handling by using SML-mapping for moving bricks intersecting bricks | - | - |
| Modify the SML-result set to accomodate intervals from TSBs | - | - |
| Handle any single brick SCCs separately - not just the last one | - | - |

