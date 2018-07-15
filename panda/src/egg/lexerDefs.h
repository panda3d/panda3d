/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lexerDefs.h
 * @author drose
 * @date 1999-01-17
 */

#ifndef LEXER_H
#define LEXER_H

#include "pandabase.h"

#include "typedef.h"

#include <string>

void egg_init_lexer(std::istream &in, const std::string &filename);
void egg_start_group_body();
void egg_start_texture_body();
void egg_start_primitive_body();
int egg_error_count();
int egg_warning_count();

void eggyyerror(const std::string &msg);
void eggyyerror(std::ostringstream &strm);

void eggyywarning(const std::string &msg);
void eggyywarning(std::ostringstream &strm);

int eggyylex();

// always read from files
#define YY_NEVER_INTERACTIVE 1

#endif
