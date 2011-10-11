// Filename: cppParser.cxx
// Created by:  drose (19Oct99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////


#include "cppParser.h"
#include "cppFile.h"
#include "cppTypeParser.h"
#include "cppBisonDefs.h"

#include <set>
#include <assert.h>

bool cppparser_output_class_keyword = false;

////////////////////////////////////////////////////////////////////
//     Function: CPPParser::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPParser::
CPPParser() : CPPScope((CPPScope *)NULL, CPPNameComponent(""), V_public) {
}

////////////////////////////////////////////////////////////////////
//     Function: CPPParser::is_fully_specified
//       Access: Public, Virtual
//  Description: Returns true if this declaration is an actual,
//               factual declaration, or false if some part of the
//               declaration depends on a template parameter which has
//               not yet been instantiated.
////////////////////////////////////////////////////////////////////
bool CPPParser::
is_fully_specified() const {
  // The global scope is always considered to be "fully specified",
  // even if it contains some template declarations.
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPParser::parse_file
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool CPPParser::
parse_file(const Filename &filename) {
  if (!init_cpp(CPPFile(filename, filename, CPPFile::S_local))) {
    cerr << "Unable to read " << filename << "\n";
    return false;
  }
  parse_cpp(this);

  return get_error_count() == 0;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPParser::parse_expr
//       Access: Public
//  Description: Given a string, expand all manifests within the
//               string and evaluate it as an expression.  Returns
//               NULL if the string is not a valid expression.
////////////////////////////////////////////////////////////////////
CPPExpression *CPPParser::
parse_expr(const string &expr) {
  return CPPPreprocessor::parse_expr(expr, this, this);
}

////////////////////////////////////////////////////////////////////
//     Function: CPPParser::parse_type
//       Access: Public
//  Description: Given a string, interpret it as a type name and
//               return the corresponding CPPType.  Returns NULL if
//               the string is not a valid type.
////////////////////////////////////////////////////////////////////
CPPType *CPPParser::
parse_type(const string &type) {
  CPPTypeParser ep(this, this);
  ep._verbose = 0;
  if (ep.parse_type(type, *this)) {
    return ep._type;
  } else {
    return (CPPType *)NULL;
  }
}
