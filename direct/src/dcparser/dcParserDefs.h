// Filename: dcParserDefs.h
// Created by:  drose (05Oct00)
// 
////////////////////////////////////////////////////////////////////

#ifndef DCPARSERDEFS_H
#define DCPARSERDEFs_H

#include "dcbase.h"
#include "dcSubatomicType.h"

class DCFile;
class DCClass;
class DCAtomicField;

void dc_init_parser(istream &in, const string &filename, DCFile &file);
void dc_cleanup_parser();
int dcyyparse();

// This structure holds the return value for each token.
// Traditionally, this is a union, and is declared with the %union
// declaration in the parser.y file, but unions are pretty worthless
// in C++ (you can't include an object that has member functions in a
// union), so we'll use a class instead.  That means we need to
// declare it externally, here.

class DCTokenType {
public:
  union U {
    int integer;
    double real;
    DCClass *dclass;
    DCAtomicField *atomic;
    DCSubatomicType subatomic;
  } u;
  string str;
};

// The yacc-generated code expects to use the symbol 'YYSTYPE' to
// refer to the above class.
#define YYSTYPE DCTokenType

#endif
