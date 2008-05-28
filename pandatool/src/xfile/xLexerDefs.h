// Filename: xLexerDefs.h
// Created by:  drose (03Oct04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef XLEXERDEFS_H
#define XLEXERDEFS_H

#include "pandatoolbase.h"

void x_init_lexer(istream &in, const string &filename);
int x_error_count();
int x_warning_count();

void xyyerror(const string &msg);
void xyyerror(const string &msg, int line_number, int col_number, 
              const string &current_line);
void xyywarning(const string &msg);

int xyylex();

extern int x_line_number;
extern int x_col_number;
extern char x_current_line[];

#endif
