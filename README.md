# BrickCounting
Count all combinations of six 2x4 LEGO bricks

See paper on-line at at http://c-mt.dk/counting

RectilinearBrick: x,y,lv,isHorizontal

^

StronglyConnectedConfiguration: parts, partsSizes

^

RectilinearConfiguration: otherBricks


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
 Combine all combinations of SCCs at corners to form all RCs.