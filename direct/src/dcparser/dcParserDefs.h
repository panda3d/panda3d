// Filename: dcParserDefs.h
// Created by:  drose (05Oct00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef DCPARSERDEFS_H
#define DCPARSERDEFS_H

#include "dcbase.h"
#include "dcSubatomicType.h"

class DCFile;
class DCClass;
class DCField;
class DCAtomicField;
class DCParameter;
class DCPacker;

void dc_init_parser(istream &in, const string &filename, DCFile &file);
void dc_init_parser_parameter_value(istream &in, const string &filename, 
                                    DCPacker &packer);
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
    PN_int64 integer;
    double real;
    bool flag;
    DCClass *dclass;
    DCField *field;
    DCAtomicField *atomic;
    DCSubatomicType subatomic;
    DCParameter *parameter;
  } u;
  string str;
};

// The yacc-generated code expects to use the symbol 'YYSTYPE' to
// refer to the above class.
#define YYSTYPE DCTokenType

#endif
