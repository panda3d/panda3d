// Filename: stitchLexerDefs.h
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

#ifndef STITCHLEXERDEFS_H
#define STITCHLEXERDEFS_H

#include <pandatoolbase.h>

void stitch_init_lexer(istream &in, const string &filename);
int stitch_error_count();
int stitch_warning_count();

void stitchyyerror(const string &msg);
void stitchyyerror(ostringstream &strm);

void stitchyywarning(const string &msg);
void stitchyywarning(ostringstream &strm);

int stitchyylex();

#endif
