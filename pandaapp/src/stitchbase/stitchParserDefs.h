// Filename: stitchParserDefs.h
// Created by:  drose (08Nov99)
// 
////////////////////////////////////////////////////////////////////

#ifndef STITCHPARSERDEFS_H
#define STITCHPARSERDEFS_H

#include <pandatoolbase.h>

#include <luse.h>

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

class YYSTYPE {
public:
  double number;
  string str;
  StitchCommand *command;
  LVecBase4d vec;
  int num_components;
};

#endif
