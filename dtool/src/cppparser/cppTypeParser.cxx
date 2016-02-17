/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cppTypeParser.cxx
 * @author drose
 * @date 1999-12-14
 */

#include "cppTypeParser.h"
#include "cppType.h"

/**
 *
 */
CPPTypeParser::
CPPTypeParser(CPPScope *current_scope, CPPScope *global_scope) :
  _current_scope(current_scope),
  _global_scope(global_scope)
{
  _type = NULL;
}

/**
 *
 */
CPPTypeParser::
~CPPTypeParser() {
}

/**
 *
 */
bool CPPTypeParser::
parse_type(const string &type) {
  if (!init_type(type)) {
    cerr << "Unable to parse type\n";
    return false;
  }

  _type = ::parse_type(this, _current_scope, _global_scope);

  return get_error_count() == 0;
}

/**
 *
 */
bool CPPTypeParser::
parse_type(const string &type, const CPPPreprocessor &filepos) {
  if (!init_type(type)) {
    cerr << "Unable to parse type\n";
    return false;
  }

  copy_filepos(filepos);

  _type = ::parse_type(this, _current_scope, _global_scope);

  return get_error_count() == 0;
}

/**
 *
 */
void CPPTypeParser::
output(ostream &out) const {
  if (_type == NULL) {
    out << "(null type)";
  } else {
    out << *_type;
  }
}
