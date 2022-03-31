/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dcLexerDefs.h
 * @author drose
 * @date 2000-10-05
 */

#ifndef DCLEXERDEFS_H
#define DCLEXERDEFS_H

#include "dcbase.h"

void dc_init_lexer(std::istream &in, const std::string &filename);
void dc_start_parameter_value();
void dc_start_parameter_description();
int dc_error_count();
int dc_warning_count();

void dcyyerror(const std::string &msg);
void dcyywarning(const std::string &msg);

int dcyylex();

// we always read files
#define YY_NEVER_INTERACTIVE 1
#endif
