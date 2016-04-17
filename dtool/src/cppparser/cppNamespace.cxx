/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cppNamespace.cxx
 * @author drose
 * @date 1999-11-16
 */

#include "cppNamespace.h"
#include "cppIdentifier.h"
#include "cppScope.h"
#include "indent.h"

/**
 *
 */
CPPNamespace::
CPPNamespace(CPPIdentifier *ident, CPPScope *scope, const CPPFile &file) :
  CPPDeclaration(file),
  _ident(ident),
  _scope(scope),
  _is_inline(false)
{
}

/**
 *
 */
string CPPNamespace::
get_simple_name() const {
  if (_ident == NULL) {
    return "";
  }
  return _ident->get_simple_name();
}

/**
 *
 */
string CPPNamespace::
get_local_name(CPPScope *scope) const {
  if (_ident == NULL) {
    return "";
  }
  return _ident->get_local_name(scope);
}

/**
 *
 */
string CPPNamespace::
get_fully_scoped_name() const {
  if (_ident == NULL) {
    return "";
  }
  return _ident->get_fully_scoped_name();
}

/**
 *
 */
CPPScope *CPPNamespace::
get_scope() const {
  return _scope;
}

/**
 *
 */
void CPPNamespace::
output(ostream &out, int indent_level, CPPScope *scope, bool complete) const {
  if (_is_inline) {
    out << "inline ";
  }
  if (!complete && _ident != NULL) {
    // If we have a name, use it.
    out << "namespace " << _ident->get_local_name(scope);

  } else {
    if (_ident != NULL) {
      out << "namespace " << _ident->get_local_name(scope) << " {\n";
    } else {
      out << "namespace {\n";
    }

    _scope->write(out, indent_level + 2, _scope);
    indent(out, indent_level) << "}";
  }
}

/**
 *
 */
CPPDeclaration::SubType CPPNamespace::
get_subtype() const {
  return ST_namespace;
}

/**
 *
 */
CPPNamespace *CPPNamespace::
as_namespace() {
  return this;
}
