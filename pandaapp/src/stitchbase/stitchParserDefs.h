// Filename: stitchParserDefs.h
// Created by:  drose (08Nov99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef STITCHPARSERDEFS_H
#define STITCHPARSERDEFS_H

#include "pandaappbase.h"

#include "luse.h"

class StitchCommand;

int stitchyyparse();

void stitch_init_parser(istream &in, const string &filename,
                        StitchCommand *tos);

// This structure holds the return value for each token.
// Traditionally, this is a union, and is declared with the %union
// declaration in the parser.y file, but unions are pretty worthless
// in C++ (you can't include an object that has member functions in a
// union), so we'll use a class instead.  That means we need to
// declare it externally, here.

class yystype {
public:
  double number;
  string str;
  StitchCommand *command;
  LVecBase4d vec;
  int num_components;
};
#define YYSTYPE yystype

#endif
