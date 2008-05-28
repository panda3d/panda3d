// Filename: lexerDefs.h
// Created by:  drose (17Jan99)
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

#ifndef LEXER_H
#define LEXER_H

#include "pandabase.h"

#include "typedef.h"

#include <string>

void egg_init_lexer(istream &in, const string &filename);
void egg_start_group_body();
void egg_start_texture_body();
void egg_start_primitive_body();
int egg_error_count();
int egg_warning_count();

void eggyyerror(const string &msg);
void eggyyerror(ostringstream &strm);

void eggyywarning(const string &msg);
void eggyywarning(ostringstream &strm);

int eggyylex();

// always read from files
#define YY_NEVER_INTERACTIVE 1

#endif
