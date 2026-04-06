/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vrmlLexerDefs.h
 * @author drose
 * @date 2004-09-30
 */

#ifndef VRMLLEXERDEFS_H
#define VRMLLEXERDEFS_H

#include "pandatoolbase.h"

void vrml_init_lexer(std::istream &in, const std::string &filename);
int vrml_error_count();
int vrml_warning_count();

void vrmlyyerror(const std::string &msg);
void vrmlyywarning(const std::string &msg);

int vrmlyylex();

#endif
