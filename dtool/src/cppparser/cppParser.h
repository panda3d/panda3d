/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cppParser.h
 * @author drose
 * @date 1999-10-19
 */

#ifndef CPPPARSER_H
#define CPPPARSER_H

#include "dtoolbase.h"

#include "cppScope.h"
#include "cppPreprocessor.h"
#include "filename.h"

#include <set>

/**
 *
 */
class CPPParser : public CPPScope, public CPPPreprocessor {
public:
  CPPParser();

  virtual bool is_fully_specified() const;

  bool parse_file(const Filename &filename);

  CPPExpression *parse_expr(const std::string &expr);
  CPPType *parse_type(const std::string &type);
};

/*
 * Normally, this variable should be left true, especially while parsing.
 * However, after parsing has finished, and you want to output the results of
 * parsing in a way that can be successfully compiled by VC++, you may need to
 * set this variable to false.  It controls the way typenames are written.
 * When true, class names are written 'class X', which is the way the parser
 * expects things to come, and which compiles successfully under every
 * compiler except VC++.  When false, class names are written simply 'X',
 * which is the only way they'll compile under VC++.
 */
extern bool cppparser_output_class_keyword;

#endif
