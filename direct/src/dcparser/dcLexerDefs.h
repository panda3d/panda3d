// Filename: dcLexerDefs.h
// Created by:  drose (05Oct00)
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

#ifndef DCLEXERDEFS_H
#define DCLEXERDEFS_H

#include "dcbase.h"

void dc_init_lexer(istream &in, const string &filename);
int dc_error_count();
int dc_warning_count();

void dcyyerror(const string &msg);
void dcyywarning(const string &msg);

int dcyylex();

// we always read files
#define YY_NEVER_INTERACTIVE 1
#endif
