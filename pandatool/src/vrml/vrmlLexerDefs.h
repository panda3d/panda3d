// Filename: vrmlLexerDefs.h
// Created by:  drose (30Sep04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef VRMLLEXERDEFS_H
#define VRMLLEXERDEFS_H

#include "pandatoolbase.h"

void vrml_init_lexer(istream &in, const string &filename);
int vrml_error_count();
int vrml_warning_count();

void vrmlyyerror(const string &msg);
void vrmlyywarning(const string &msg);

int vrmlyylex();

#endif
