/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file xLexerDefs.h
 * @author drose
 * @date 2004-10-03
 */

#ifndef XLEXERDEFS_H
#define XLEXERDEFS_H

#include "pandatoolbase.h"

void x_init_lexer(std::istream &in, const std::string &filename);
int x_error_count();
int x_warning_count();

void xyyerror(const std::string &msg);
void xyyerror(const std::string &msg, int line_number, int col_number,
              const std::string &current_line);
void xyywarning(const std::string &msg);

int xyylex();

extern int x_line_number;
extern int x_col_number;
extern char x_current_line[];

#endif
