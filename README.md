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

|  Number of bricks    | 1 |  2 |     3 |       4 |          5 |           6 | 
|:----------:|--:|---:|------:|--------:|-----------:|------------:|
| **RCs (previous results)**     | 1 | 24 | 1.560 | 119.580 | 10.166.403 | 915.103.765 |
| **SCCs (intermediate results)**    | 1 | 20 | 1.004 |  58.862 |  3.817.291 | 261.534.637 |
| **Models (our main results)** | 0 |  0 |     0 |  1.100* |   161.724* | 12.802.643* |

See the detailed [Findings](http://c-mt.dk/counting/findings.php) page for a breakdown of these numbers.


## Terminology

In this section we define the terms used in the table of our current findings above.

* **Brick**: A standard "2 by 4" LEGO brick with 8 studs on top.
* **Configuration**: Any number of bricks connected by their studs. See any picture on this page for examples of configurations.
* **Strongly Connected Configuration (SCC)**: The configuration consisting of a single brick is an SCC. Any SCC can be constructed by taking an existing SCC and adding a new brick while obeying the following rule: The new brick has to connect to at least two studs of another brick. The picture below shows how to construct SCCs:

![SCCs](http://c-mt.dk/counting/images/sccconstructionsmall.png "Notice that for any SCC with more than one brick, any additional brick has to connect to a single other brick using at least two studs.")

* **Rectilinear configuration (RC)**: A configuration where all bricks are connected at right angles. Previous results counted all RCs. Notice that an SCC is an RC, but the opposite is not necessarily true. The picture below shows RCs which are not SCCs:

![RCs](http://c-mt.dk/counting/images/rcsnotsccssmall.png "These RCs are not SCCs because there are always a partitioning of the configurations where the bricks of one partition share at most one connection with any brick of the other partition.")

* **Model**: A set of configurations with the properties that there is no RC in the set and that any configuration in the set can be turned into any other configuration of the set simply by turning bricks at the connections. We typically paint the individual SCCs of a configuration different colors. This is only for illustrative purposes. The results assume that all bricks have the same color and are thus interchangeable. The picture below shows various configurations belonging to the same model:

![Models](http://c-mt.dk/counting/images/configurationsofamodelsmall.png "These configurations all belong to the same model.")

