// Filename: dcLexerDefs.h
// Created by:  drose (05Oct00)
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

#endif
