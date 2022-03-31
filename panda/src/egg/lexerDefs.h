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
#include "parserDefs.h"

#include <string>

typedef void *yyscan_t;
struct EggLexerState;
struct EggParserState;
struct EggTokenType;
struct EggLocType;

void egg_init_lexer_state(EggLexerState &state, std::istream &in, const std::string &filename);
void egg_cleanup_lexer_state(EggLexerState &state);

void egg_start_group_body(EggLexerState &state);
void egg_start_texture_body(EggLexerState &state);
void egg_start_primitive_body(EggLexerState &state);

// These functions are declared by flex.
int eggyylex_init_extra(EggLexerState *state, yyscan_t *scanner);
int eggyylex_destroy(yyscan_t scanner);

void eggyyerror(EggLocType *loc, yyscan_t scanner, const std::string &msg);
void eggyywarning(EggLocType *loc, yyscan_t scanner, const std::string &msg);

int eggyylex(EggTokenType *yylval_param, EggLocType *yylloc_param, yyscan_t yyscanner);

static const size_t egg_max_error_width = 1024;

struct EggLexerState {
  // current_line holds as much of the current line as will fit.  Its
  // only purpose is for printing it out to report an error to the user.
  char _current_line[egg_max_error_width + 1];

  int _error_count = 0;
  int _warning_count = 0;

  // This is the pointer to the current input stream.
  std::istream *_input_p = nullptr;

  // This is the name of the egg file we're parsing.  We keep it so we
  // can print it out for error messages.
  std::string _egg_filename;

  // This is the initial token state returned by the lexer.  It allows
  // the yacc grammar to start from initial points.
  int _initial_token = 0;
};

#define YY_EXTRA_TYPE EggLexerState *

// always read from files
#define YY_NEVER_INTERACTIVE 1

#endif
