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
CPPNamespace(CPPIdentifier *ident, CPPScope *scope, const CPPFile &file,
             CPPAttributeList attr) :
  CPPDeclaration(file, std::move(attr)),
  _is_inline(false),
  _ident(ident),
  _scope(scope)
{
}

/**
 *
 */
std::string CPPNamespace::
get_simple_name() const {
  if (_ident == nullptr) {
    return "";
  }
  return _ident->get_simple_name();
}

/**
 *
 */
std::string CPPNamespace::
get_local_name(CPPScope *scope) const {
  if (_ident == nullptr) {
    return "";
  }
  return _ident->get_local_name(scope);
}

/**
 *
 */
std::string CPPNamespace::
get_fully_scoped_name() const {
  if (_ident == nullptr) {
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
output(std::ostream &out, int indent_level, CPPScope *scope, bool complete) const {
  if (_is_inline) {
    out << "inline ";
  }
  out << "namespace ";

  if (!complete && _ident != nullptr) {
    // If we have a name, use it.
    out << "namespace " << _ident->get_local_name(scope);
  }
  else {
    if (!_attributes.is_empty()) {
      out << _attributes << " ";
    }
    if (_ident != nullptr) {
      out << _ident->get_local_name(scope) << " {\n";
    } else {
      out << "{\n";
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
