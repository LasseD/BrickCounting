// -*- mode: c++; tab-width: 2; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=2 sts=2 sw=2 noet :

#include <iostream>
#include <stdlib.h>
#include "rectilinear.h"

/*
Rectilinear:

Design:
- File layout:
-- folder: count, file: lvls
-- example: folder 2, file 11.brx

- file format:
-- Rectilinear combination bit usage (11 bit total):
 First bit: 0 for vertical, 1 for horizontal
 x index: 0 to 31: -15 to 15 (5 bit)
 y index: 0 to 31: -15 to 15 (5 bit)
 Restrictions:
  - first brick must be a 000 config
  - lower lv. 0 must be minimal (lex on bricks)
  - bricks must be x,y lex sorted on layers

- Program design:
-- main.cpp: entry point.
-- file_management.h and .cpp: 
--- class RectilinearCombinations
---- read
---- contains
---- verify
---- add
---- save

-  Usage: params: count (must be at most +1 of largest done)

Single Circle (no non-rect):
- one 3 point circle:
 TODO
- two 3 point circles:
 -- middle has to be 2-brick, then 3 point circles from ^^
- one 4 point circle:
 -- one 3-brick, 3 x single brick.
 -- one 2-brick, 3 x single brick
 -- 4 x one brick
 -- two 2-brick, 2 x single brick
- one 5 point circle:
 -- one 2-brick, 4 x single brick
 -- 5 x one brick
- one 6 point circle:
 -- 6 x single brick

no circle, non-rect:
- TODO

Both circle and rect:
- one 4 point circle:
 -- two single brick unrect
 -- one single brick unrect
 -- one two brick unrect
 -- two-snake unrect
- one 3 point circle (uses 4 bricks):
 -- two single brick unrect
 -- one single brick unrect
 -- one two brick unrect
 -- two-snake unrect

 */
int main(int argc, char** argv) {
  rectilinear::build_all_combinations<1>();
  rectilinear::build_all_combinations<2>();
  rectilinear::build_all_combinations<3>();
  rectilinear::build_all_combinations<4>();
  rectilinear::build_all_combinations<5>();
  rectilinear::build_all_combinations<6>();
  return 0;
}

