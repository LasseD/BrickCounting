# All Combinations of Six 2x4 LEGO Bricks

[Main project page](http://c-mt.dk/counting) | [Findings](http://c-mt.dk/counting/findings.php) | [Full paper](http://c-mt.dk/counting/?view=paper) | **Project on GitHub** | [Updates on Eurobricks](http://www.eurobricks.com/forum/index.php?showtopic=71971)

## Motivation

[Previous results](http://www.math.ku.dk/~eilers/lego.html) count all the ways that 2x4 LEGO bricks can be connected at straight angles.

![Bricks connected at right angles](http://c-mt.dk/counting/images/rectilinearintrosmall.png "There are 915.103.765 ways to combine 6 bricks at straight angles.")

We seek to find the number of ways up to six bricks can be connected when using all possible angles.

![Bricks connected at odd angles](http://c-mt.dk/counting/images/modelsintrosmall.png "It is currently unknown how many ways 6 bricks can be combined at other angles.")


## Our Findings

For an explanation of the terms used in this section, please refer to the [Terminology](#terminology) section below.
This is a work in progress, so the results are still incomplete. The tables below shows the current findings. The numbers marked with an asterix (*) are not yet final. 

|  Number of bricks             | 1 |  2 |     3 |       4 |          5 |           6 | 
|:-----------------------------:|--:|---:|------:|--------:|-----------:|------------:|
| **RC's (previous results)**   | 1 | 24 | 1.560 | 119.580 | 10.166.403 | 915.103.765 |
| **Models (our main results)** | 0 |  0 |     0 |  1.108* |   161.724* | 26.417.316* |

See the detailed [findings page](http://c-mt.dk/counting/findings.php) for a breakdown of these numbers, and the [full paper](http://c-mt.dk/counting/?view=paper) for all details of the theory behind the numbers, as well as the software used to find them.


## Terminology

In this section we define the terms used in the table of our current findings above.

* **Brick**: A brick is a standard "2 by 4" LEGO brick with 8 studs on top.
* **Configuration**: A configuration is any number of bricks connected by their studs. See any picture on this page for examples of configurations.
* **Rectilinear configuration (RC)**: An RC is a configuration where all bricks are connected at right angles. Previous results count the exact number of RC's.
* **Model**: Our main contribution is to count the number of models. A model is a set of configurations where any configuration in the set can be turned into any other configuration of the set simply by turning bricks where they are connected to other bricks using a single stud. Furthermore, there may be no RC in a model. The picture below shows various configurations belonging to the same model.

![Models](http://c-mt.dk/counting/images/configurationsofamodelsmall.png "These configurations all belong to the same model.")

We typically use colors to highlight the individual sub-configurations that can be turned. This is, however, only for illustrative purposes. Our results, as well as previous results, assume that all bricks have the same color and are thus interchangeable. 

