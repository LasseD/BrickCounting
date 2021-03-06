# All Combinations of Six 2x4 LEGO Bricks

[Main project page](http://c-mt.dk/counting) | [Findings](http://c-mt.dk/counting/findings.php) | **Full paper** | [Models](http://c-mt.dk/counting/models.php) | [Project on GitHub](https://github.com/LasseD/BrickCounting) | [Updates on Eurobricks](http://www.eurobricks.com/forum/index.php?showtopic=71971)

By Lasse Deleuran, <last_updated>

## Abstract

The answer to the question "Take six eight-stud LEGO bricks (2x4) - how many ways can they be combined?" is currently accepted as being 915.103.765. This is how many distinct configurations of bricks there are when we restrict the bricks to be placed as if on a grid where the studs of the bricks are located on integer coordinates. This limitation is extremely useful when using software to find an answer to the question as it limits all but the most basic of geometrical considerations.

![Bricks connected at right angles](http://c-mt.dk/counting/images/rectilinearintrosmall.png "There are 915.103.765 distinct configurations of six bricks when the bricks restricted to integer coordinates for the studs.")

In this paper we seek to find the answer without imposing the limitation on the placement of the bricks.

![Bricks connected at odd angles](http://c-mt.dk/counting/images/modelsintrosmall.png "It is currently unknown how many ways six bricks can be combined.")

We present a simple framework for identifying and classifying models of bricks. Using our framework we are able to identify all models of 4 bricks, and provide lower bounds for the number of models with 5 and 6 bricks. The use of lower bounds is currently caused by software limitations.

|  Number of bricks                 | 1 |  2 |     3 |       4 |          5 |           6 | 
|:---------------------------------:|--:|---:|------:|--------:|-----------:|------------:|
| **Previous results**              | 1 | 24 | 1.560 | 119.580 | 10.166.403 | 915.103.765 |
| **Models (our main results)**     | 0 |  0 |     0 |   1.144 |   209.603* | 26.417.316* |

* The best known lower bounds

## Introduction

This paper is contructed as follows: 

### Previous Results

Previous results, for counting how many ways 2x4 (two-by-four) LEGO bricks can be assembled, count distinct fonfigurations where the bricks are placed as if on a grid where the studs of the bricks have integer coordinates. This limitation on the placement of bricks makes for a simple and clean model of computation.

 where the placement of a brick can be described by three integers (x, y, z) and a flag (IsHorizontal). Verification for , and verifying if a configuration is valid.

TODO

### Overview of our results

TODO

This limitation makes computation of the number of combinations easy. In this paper we wish to include all combinations where bricks are in general position, that is, are allowed to be connected at non-rectilinear angles.

<TODO: Insert previous results here>

Care has to be taken when deciding if two models are "the same". If we are simply interested in identifying the number of different models, the result is simply "One for a single brick and infinity for any number of bricks larger than one". This is realized by connecting two bricks at a corner and observing that the angle they are connected at is freely adjustable between the extremes.

We use the concept of homotopic equivalence: Two models are homotopically equivalent (or simply "homotopic") if one can be transformed into the other simply by rotating any free angles. If two models are not homotopic, they are homotopically distinct. We are interested in finding the number of homotopically distinct models. Notice that this definition extends to the previous results. These results count the number of homotopically distinct models where each model is homotopic to a model where all bricks are at rectilinear positions.

In the following section we present strict defitions for the concepts used in the remainder of the paper. Using strict definitions makes it easier to understand exactly what is referred to, and removes any ambiguity. Once the definitions are set, we dedicate a section to the geometry of the problem at hand, followed by a section with discussions of practical considerations when computing the results. The results and conclusion are presented in the last two sections.

## Framework for classifying models

### Definitions

In this section we present the definitions to be used for the remainder of the paper.

#### Bricks

We only consider the standard "2 x 4" LEGO Brick, or simply _brick_. A brick consist of a _block_ and a eight _studs_ on top of the block. The exact geometry of the brick is presented in a later section.

#### Connections

The block of a brick is open underneath allowing a stud from another brick to be placed inside the block. There are eight such places in a brick (each corresponding to a stud above). Two bricks are _connected_ when one of them has at least one stud inside the block of the other. With bricks being connected like this, we can limit ourselves to consider bricks as always having the studs pointing up, that is, with the block placed on the X/Y plane and the studs pointing along the Z-axis in the positive direction. See the following section for the geometry of bricks. 
Two bricks are said to be _connected by a corner_ if their shared connection consists of a single stud and this stud is at one of the four corner locations of its brick.
Two bricks are _strongly connected_ if their shared connection consists of two or more studs. Notice that the angle of two strongly connected bricks can not be changed.

#### Levels

The way bricks are connected imposes a natural _level_ on their position along the Z-axis. Consider a set of connected bricks. We say that the bricks with lowest Z-position are at _level 0_. The bricks directly above are at _level 1_ and so forth. When two bricks are at _level n_ and _level (n+1)_ we say that they are at _neighboring levels_.

#### Configuration

A _configuration_ is a set of connected bricks. We only consider physically realizable configurations. This means that any stud inside a block of a brick must be at one of the eight allowed locations, and that two blocks may not intersect.
A configuration can only be moved by rotation and translation in the X/Y-plane. We say two configurations are the same (or identical) if one can become the other by simply moving it.
Notice that already for two bricks there are infinitely many different configurations as the bricks can be connected by a corner and the angle they are connected at can be turned freely between the two extremes. 
A configuration in which all bricks are connected at straight angles is called _rectilinear_.
Given a brick _a_ in a configuration, the _strongly connected configuration_ of _a_ is the rectilinear configuration obtained by greedily adding strongly connected bricks to the set of bricks initially containing _a_.
Notice that the division of a configuration into strongly connected configurations is unique.

#### Homotopy

A configuration can be canges using a using a _simple transformation_. A simple transformation is performed by rotating one or more pairs of bricks that are connected by their corners.
Two configurations are _homotopically equivalent_, or _homotopic_ if one can be transformed into the other by a set of simple transformations. 

#### Model 

Two configurations are in the same _homotopy class_ iff they are homotopic. We use the term _model_ to denote a homotopy class and say the model is _rectilinear_ if it contains a rectilinear configuration.

#### Turn points 

We define the _turn points_ of a model to be those connections for which there exist at least two configurations in the model where the angle between the bricks of the connection are different. Notice that two bricks connected by the corner are not necessarily contributing a turn point. 
A turn point is _free_ (as opposed to _restricted_) if it can be turned as if there were only the two bricks of the connection in the model.
A turn point is _non rectilinearly locked_ (or simply _locked_) if it is _restricted_ and the angle of which is can turn does not contain a straight angle. 
Notice that two models can be different simply because of a turn point being locked at two different intervals of angles. <TODO - example>
Notice also that the angle of one turn point can affect whether other turn points are locked. There are even non-rectilinear models where for each turn point there exists a configurations where the turn point isn't locked. <TODO - example>

#### Circles

A _circle_ is a set of three or more turn points in a model with the property that for each turn point of the set, each rectilinear configuration of the turn point is connected to exactly two of the turn points of the set, and that the turn points of the set form a connected component when considering the connections of the rectilinear configurations.
A circle is locked if at least one of its turn points is locked.

## Geometry

<TODO: Find size of block, diameter of stud and compute angle>


## Counting All Models

Previous results have computed all rectilinear models. 
We are interested in finding all non-rectilinear models. In order to find non-rectilinear models, all physically realizable configurations are considered. Turn points are identified in order to identify homotopy classes.
We divide the discussion of how to find non-rectilinear models into sections based on the number of bricks.


### The Trivial Case: 1 brick

This case is trivial as there is only one model when using a single brick. This model is rectilinear and there are thus no non-rectilinear models.


### The easy case: 2 bricks

From previous results [] it is given that there are 24 models with two bricks. Four of these models have a turn point, but these turn points are obviously free, so all 24 models are rectilinear.


### Still no non-rectilinear models: 3 bricks

There are 1560 rectilinear models []. 
In order to explore potential non-rectilinear models we consider models with either one or two turn points. 

#### Models with one turn point

A configuration of a model with one turn point consist of a rectilinear configuration _a_ with two bricks (without turn points) and a single brick _b_ connected to this configuration at a corner (the turn point). 
From the previous section we know there are 20 (24-4) ways _a_ can be constructed. To connect _b_ to _a_ at a corner can't cause the turn point to be locked. With the low number of combinations this can even be realized quickly by trying to construct them all.

#### Models with two turn points

Consider a model with 3 bricks and 2 turn points. One turn point can't cause the other to be locked, so all models of this kind are rectilinear.


### The first interesting case: 4 bricks

For 4 bricks there are both models with locked cycles and locked turn points.

In order to identify all non-rectilinear models without circles we have constructed a program that attempts to connect models with three or less bricks at corners of bricks and compute the intervals of possible angles of the turn points. <TODO - result> 

For models with circles there are at most one circle in a model and it has either 3 or 4 turn points. 
If a circle has 4 turn points then it is also rectilinear as there is a configuration where the turn points form a square.
If a circle has 3 turn points then there are so few ways in which the turn points can be constructed that the non-rectilinear models can be found simply by investigating all such possible models. Notice! There are models with where the circle isn't locked, so these models need not to be counted. <TODO - result>


<TODO - perform calculations with 4 bricks first!>


## References

- [http://www.math.ku.dk/~eilers/lego.html](http://www.math.ku.dk/~eilers/lego.html)
