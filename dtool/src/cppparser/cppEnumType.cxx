// Filename: cppEnumType.cxx
// Created by:  drose (25Oct99)
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


#include "cppEnumType.h"
#include "cppTypedef.h"
#include "cppExpression.h"
#include "cppSimpleType.h"
#include "cppScope.h"
#include "cppParser.h"
#include "indent.h"

////////////////////////////////////////////////////////////////////
//     Function: CPPEnumType::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPEnumType::
CPPEnumType(CPPIdentifier *ident, CPPScope *current_scope,
            const CPPFile &file) :
  CPPExtensionType(T_enum, ident, current_scope, file)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CPPEnumType::add_element
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPInstance *CPPEnumType::
add_element(const string &name, CPPExpression *value) {
  CPPType *type =
    CPPType::new_type(new CPPSimpleType(CPPSimpleType::T_int,
                                        CPPSimpleType::F_unsigned));
  CPPIdentifier *ident = new CPPIdentifier(name);
  CPPInstance *inst = new CPPInstance(type, ident);
  inst->_initializer = value;
  _elements.push_back(inst);
  return inst;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPEnumType::is_incomplete
//       Access: Public, Virtual
//  Description: Returns true if the type has not yet been fully
//               specified, false if it has.
////////////////////////////////////////////////////////////////////
bool CPPEnumType::
is_incomplete() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPEnumType::substitute_decl
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPDeclaration *CPPEnumType::
substitute_decl(CPPDeclaration::SubstDecl &subst,
                CPPScope *current_scope, CPPScope *global_scope) {
  SubstDecl::const_iterator si = subst.find(this);
  if (si != subst.end()) {
    return (*si).second;
  }

  CPPEnumType *rep = new CPPEnumType(*this);
  if (_ident != NULL) {
    rep->_ident =
      _ident->substitute_decl(subst, current_scope, global_scope);
  }

  if (rep->_ident == _ident) {
    delete rep;
    rep = this;
  }
  rep = CPPType::new_type(rep)->as_enum_type();
  subst.insert(SubstDecl::value_type(this, rep));

  return rep;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPEnumType::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CPPEnumType::
output(ostream &out, int indent_level, CPPScope *scope, bool complete) const {
  if (!complete && _ident != NULL) {
    // If we have a name, use it.
    if (cppparser_output_class_keyword) {
      out << _type << " ";
    }
    out << _ident->get_local_name(scope);

  } else if (!complete && !_typedefs.empty()) {
    // If we have a typedef name, use it.
    out << _typedefs.front()->get_local_name(scope);

  } else {
    out << _type;
    if (_ident != NULL) {
      out << " " << _ident->get_local_name(scope);
    }

    out << " {\n";
    Elements::const_iterator ei;
    for (ei = _elements.begin(); ei != _elements.end(); ++ei) {
      indent(out, indent_level + 2) << (*ei)->get_local_name();
      if ((*ei)->_initializer != NULL) {
        out << " = " << *(*ei)->_initializer;
      }
      out << ",\n";
    }
    indent(out, indent_level) << "}";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CPPEnumType::get_subtype
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPDeclaration::SubType CPPEnumType::
get_subtype() const {
  return ST_enum;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPEnumType::as_enum_type
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPEnumType *CPPEnumType::
as_enum_type() {
  return this;
}
