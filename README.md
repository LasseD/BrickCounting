# BrickCounting
Count all combinations of six 2x4 LEGO bricks

See paper on-line at at http://c-mt.dk/counting

RectilinearBrick: x,y,lv,isHorizontal
^
StronglyConnectedConfiguration: parts, partsSizes
^
RectilinearConfiguration: otherBricks
 
TODO:
1: Construct all possible SCCs (StronglyConnectedConfigurationBuilder)
2: Construct all possible Rectilinear configurations (Super set)
3: Construct all possible configurations using SCCs.
 3a: First for connections, no circles, simple check "isRelializable"
 3b: Add different models with same circles (motion map)
 3c: Add circles (near angle search)

Detailed TODO:
1:
 1a: Construct SCC blowup alorithm + detect duplicates.
  brick: get possible strongly connected
  RC: 
   Run ^^ for all bricks, 
   remove duplicates/impossible
   return all
  StronglyConnectedConfigurationBuilder::buildForSize(size):
   if size==1 - return easy.
   For all one smaller: expand while checking for duplicates
 1b: Serialize SCC: Easy to read, space efficient, indexed.
  Folder structure: /scc/<size>.bin 
  File content for <size>:
   Actual content: {<bricks sorted z,x,y>}
   Content is sorted! => Easy read + lookup for duplicates.
  TODO!
