// Filename: lexerDefs.h
// Created by:  drose (17Jan99)
//
////////////////////////////////////////////////////////////////////

#ifndef LEXER_H
#define LEXER_H

#include <pandabase.h>

#include <typedef.h>

#include <string>

void egg_init_lexer(istream &in, const string &filename);
void egg_start_group_body();
int egg_error_count();
int egg_warning_count();

void eggyyerror(const string &msg);
void eggyyerror(ostringstream &strm);

void eggyywarning(const string &msg);
void eggyywarning(ostringstream &strm);

int eggyylex();

#endif
