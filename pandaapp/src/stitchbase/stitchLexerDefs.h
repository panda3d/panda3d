// Filename: stitchLexerDefs.h
// Created by:  drose (08Nov99)
// 
////////////////////////////////////////////////////////////////////

#ifndef STITCHLEXERDEFS_H
#define STITCHLEXERDEFS_H

#include <pandatoolbase.h>

void stitch_init_lexer(istream &in, const string &filename);
int stitch_error_count();
int stitch_warning_count();

void stitchyyerror(const string &msg);
void stitchyyerror(ostringstream &strm);

void stitchyywarning(const string &msg);
void stitchyywarning(ostringstream &strm);

int stitchyylex();

#endif
