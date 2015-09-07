# All Combinations of Six 2x4 LEGO Bricks

[Main project page](http://c-mt.dk/counting)

## Motivation

[Previous results](http://www.math.ku.dk/~eilers/lego.html) count combinations where 2x4 LEGO bricks are connected at straight angles.

![Bricks connected at right angles](http://c-mt.dk/counting/images/rectilinearintrosmall.png "There are 915.103.765 ways to combine 6 bricks at straight and right angles")

We seek to find the number of ways six bricks can be connected when using all possible angles.

![Bricks connected at odd angles](http://c-mt.dk/counting/images/modelsintrosmall.png "It is currently unknown how many ways 6 bricks can be combined at other angles")


## Terminology

* **Brick**: A standard "2 by 4" LEGO brick with 8 studs on top.
* **Configuration**: Any number of bricks connected by their studs. For examples of configurations, simply see any picture on this page.
* **Strongly Connected Configuration (SCC)**: The configuration consisting of a single brick is an SCC. Any SCC can be constructed by taking an existing SCC and adding a new brick while obeying the following rule: The new brick has to connect to at least two studs of another brick. The picture below shows how to construct SCCs:

![SCCs](http://c-mt.dk/counting/images/sccconstructionsmall.png "Notice that for any SCC with more than one brick, any additional brick has to connect to a single other brick using at least two studs")

* **Rectilinear configuration (RC)**: A configuration where all bricks are connected at right angles. Notice that an SCC is an RC, but the opposite is not necessarily true. Previous results counted all RCs.
* **Model**: A set of configurations with the property that any configuration in the set can be turned into any other configuration of the set simply by turning bricks at turn points. We typically paint the individual SCCs of a configuration different colors. Please refer to the "Examples" section below for examples.

## Findings

This is a work in progress, so the results are still incomplete. The table shows the current totals. The numbers marked with an asterix (*) are not final. 

|  Bricks    | 1 |  2 |     3 |       4 |          5 |           6 | 
|:----------:|--:|---:|------:|--------:|-----------:|------------:|
| **SCC**    | 1 | 20 | 1.004 |  58.862 |  3.817.291 | 261.534.637 |
| **RC**     | 1 | 24 | 1.560 | 119.580 | 10.166.403 | 915.103.765 |
| **Models** | 0 |  0 |     0 |  *1.100 |   *135.614 | *19.701.710 |

Please refer to [Findings](http://c-mt.dk/counting/findings.php) for a breakdown of the current findings.

## Examples

Notice that this definition causes the two configurations in the image below to represent a rectilinear configuration (left) and a model (right) even though they consist of the same SCCs.

![Two different models](http://c-mt.dk/counting/images/modelsdifferentsmall.png "These two models are not the same because the green and yellow SCC block the turn points")

Notice that for a given set of SCCs there can be many models (and many RCs). As an example. The following image shows how three specific SCCs can be combined into 14 different models.

![3 SCCs becoming 14 models](http://c-mt.dk/counting/images/variousmodelsexamplesmall.png "These three SCCs can be combined into 14 different models")
