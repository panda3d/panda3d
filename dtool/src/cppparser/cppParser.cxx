/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cppParser.cxx
 * @author drose
 * @date 1999-10-19
 */

#include "cppParser.h"
#include "cppFile.h"
#include "cppTypeParser.h"
#include "cppBisonDefs.h"

#include <set>
#include <assert.h>

bool cppparser_output_class_keyword = false;

/**
 *
 */
CPPParser::
CPPParser() : CPPScope(nullptr, CPPNameComponent(""), V_public) {
}

/**
 * Returns true if this declaration is an actual, factual declaration, or
 * false if some part of the declaration depends on a template parameter which
 * has not yet been instantiated.
 */
bool CPPParser::
is_fully_specified() const {
  // The global scope is always considered to be "fully specified", even if it
  // contains some template declarations.
  return true;
}

/**
 *
 */
bool CPPParser::
parse_file(const Filename &filename) {
  Filename canonical(filename);
  canonical.make_canonical();

  CPPFile file(canonical, filename, CPPFile::S_local);

  // Don't read it if we included it before and it had #pragma once.
  ParsedFiles::iterator it = _parsed_files.find(file);
  if (it != _parsed_files.end() && it->_pragma_once) {
    // But mark it as local.
    it->_source = CPPFile::S_local;
    return true;
  }

  if (!init_cpp(file)) {
    std::cerr << "Unable to read " << filename << "\n";
    return false;
  }
  parse_cpp(this);

  return get_error_count() == 0;
}

/**
 * Given a string, expand all manifests within the string and evaluate it as
 * an expression.  Returns NULL if the string is not a valid expression.
 */
CPPExpression *CPPParser::
parse_expr(const std::string &expr) {
  YYLTYPE loc = {};
  return CPPPreprocessor::parse_expr(expr, this, this, loc);
}

/**
 * Given a string, interpret it as a type name and return the corresponding
 * CPPType.  Returns NULL if the string is not a valid type.
 */
CPPType *CPPParser::
parse_type(const std::string &type) {
  CPPTypeParser ep(this, this);
  ep._verbose = 0;
  if (ep.parse_type(type, *this)) {
    return ep._type;
  } else {
    return nullptr;
  }
}
