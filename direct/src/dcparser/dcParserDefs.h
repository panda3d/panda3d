/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dcParserDefs.h
 * @author drose
 * @date 2000-10-05
 */

#ifndef DCPARSERDEFS_H
#define DCPARSERDEFS_H

#include "dcbase.h"
#include "dcSubatomicType.h"
#include "vector_uchar.h"

class DCFile;
class DCClass;
class DCSwitch;
class DCField;
class DCAtomicField;
class DCParameter;
class DCKeyword;
class DCPacker;

void dc_init_parser(std::istream &in, const std::string &filename, DCFile &file);
void dc_init_parser_parameter_value(std::istream &in, const std::string &filename,
                                    DCPacker &packer);
void dc_init_parser_parameter_description(std::istream &in, const std::string &filename,
                                          DCFile *file);
DCField *dc_get_parameter_description();
void dc_cleanup_parser();
int dcyyparse();

extern DCFile *dc_file;

// This structure holds the return value for each token.  Traditionally, this
// is a union, and is declared with the %union declaration in the parser.y
// file, but unions are pretty worthless in C++ (you can't include an object
// that has member functions in a union), so we'll use a class instead.  That
// means we need to declare it externally, here.

class EXPCL_DIRECT_DCPARSER DCTokenType {
public:
  union U {
    int s_int;
    unsigned int s_uint;
    int64_t int64;
    uint64_t uint64;
    double real;
    bool flag;
    DCClass *dclass;
    DCSwitch *dswitch;
    DCField *field;
    DCAtomicField *atomic;
    DCSubatomicType subatomic;
    DCParameter *parameter;
    const DCKeyword *keyword;
  } u;
  std::string str;
  vector_uchar bytes;
};

// The yacc-generated code expects to use the symbol 'YYSTYPE' to refer to the
// above class.
#define YYSTYPE DCTokenType

#endif
