// Filename: cppNamespace.cxx
// Created by:  drose (16Nov99)
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


#include "cppNamespace.h"
#include "cppIdentifier.h"
#include "cppScope.h"
#include "indent.h"

////////////////////////////////////////////////////////////////////
//     Function: CPPNamespace::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPNamespace::
CPPNamespace(CPPIdentifier *ident, CPPScope *scope, const CPPFile &file) :
  CPPDeclaration(file),
  _ident(ident), _scope(scope)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CPPNamespace::get_simple_name
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
string CPPNamespace::
get_simple_name() const {
  if (_ident == NULL) {
    return "";
  }
  return _ident->get_simple_name();
}

////////////////////////////////////////////////////////////////////
//     Function: CPPNamespace::get_local_name
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
string CPPNamespace::
get_local_name(CPPScope *scope) const {
  if (_ident == NULL) {
    return "";
  }
  return _ident->get_local_name(scope);
}

////////////////////////////////////////////////////////////////////
//     Function: CPPNamespace::get_fully_scoped_name
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
string CPPNamespace::
get_fully_scoped_name() const {
  if (_ident == NULL) {
    return "";
  }
  return _ident->get_fully_scoped_name();
}

////////////////////////////////////////////////////////////////////
//     Function: CPPNamespace::get_scope
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPScope *CPPNamespace::
get_scope() const {
  return _scope;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPNamespace::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CPPNamespace::
output(ostream &out, int indent_level, CPPScope *scope, bool complete) const {
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

////////////////////////////////////////////////////////////////////
//     Function: CPPNamespace::get_subtype
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPDeclaration::SubType CPPNamespace::
get_subtype() const {
  return ST_namespace;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPNamespace::as_namespace
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPNamespace *CPPNamespace::
as_namespace() {
  return this;
}
