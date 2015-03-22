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
#include "cppTypedefType.h"
#include "cppExpression.h"
#include "cppSimpleType.h"
#include "cppConstType.h"
#include "cppScope.h"
#include "cppParser.h"
#include "indent.h"

////////////////////////////////////////////////////////////////////
//     Function: CPPEnumType::Constructor
//       Access: Public
//  Description: Creates an untyped, unscoped enum.
////////////////////////////////////////////////////////////////////
CPPEnumType::
CPPEnumType(CPPIdentifier *ident, CPPScope *current_scope,
            const CPPFile &file) :
  CPPExtensionType(T_enum, ident, current_scope, file),
  _parent_scope(current_scope),
  _element_type(NULL),
  _last_value(NULL)
{
  if (ident != NULL) {
    ident->_native_scope = current_scope;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CPPEnumType::Constructor
//       Access: Public
//  Description: Creates a typed but unscoped enum.
////////////////////////////////////////////////////////////////////
CPPEnumType::
CPPEnumType(CPPIdentifier *ident, CPPType *element_type,
            CPPScope *current_scope, const CPPFile &file) :
  CPPExtensionType(T_enum, ident, current_scope, file),
  _parent_scope(current_scope),
  _element_type(element_type),
  _last_value(NULL)
{
  if (ident != NULL) {
    ident->_native_scope = current_scope;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CPPEnumType::add_element
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPInstance *CPPEnumType::
add_element(const string &name, CPPExpression *value) {
  CPPIdentifier *ident = new CPPIdentifier(name);
  ident->_native_scope = _parent_scope;
  CPPInstance *inst;

  static CPPType *default_element_type = NULL;
  if (_element_type == NULL) {
    // This enum is untyped.  Use a suitable default, ie. 'int'.
    if (default_element_type == NULL) {
      default_element_type =
        CPPType::new_type(new CPPConstType(new CPPSimpleType(CPPSimpleType::T_int, 0)));
    }

    inst = new CPPInstance(default_element_type, ident);
  } else {
    // This enum has an explicit type, so use that.
    inst = new CPPInstance(CPPType::new_type(new CPPConstType(_element_type)), ident);
  }

  _elements.push_back(inst);

  if (value == (CPPExpression *)NULL) {
    if (_last_value == (CPPExpression *)NULL) {
      // This is the first value, and should therefore be 0.
      static CPPExpression *const zero = new CPPExpression(0);
      value = zero;

    } else if (_last_value->_type == CPPExpression::T_integer) {
      value = new CPPExpression(_last_value->_u._integer + 1);

    } else {
      // We may not be able to determine the value just yet.  No
      // problem; we'll just define it as another expression.
      static CPPExpression *const one = new CPPExpression(1);
      value = new CPPExpression('+', _last_value, one);
    }
  }
  inst->_initializer = value;
  _last_value = value;
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
//     Function: CPPEnumType::is_fully_specified
//       Access: Public, Virtual
//  Description: Returns true if this declaration is an actual,
//               factual declaration, or false if some part of the
//               declaration depends on a template parameter which has
//               not yet been instantiated.
////////////////////////////////////////////////////////////////////
bool CPPEnumType::
is_fully_specified() const {
  if (!CPPDeclaration::is_fully_specified()) {
    return false;
  }

  if (_ident != NULL && !_ident->is_fully_specified()) {
    return false;
  }

  if (_element_type != NULL && !_element_type->is_fully_specified()) {
    return false;
  }

  Elements::const_iterator ei;
  for (ei = _elements.begin(); ei != _elements.end(); ++ei) {
    if (!(*ei)->is_fully_specified()) {
      return false;
    }
  }

  return true;
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

  if (_element_type != NULL) {
    rep->_element_type =
      _element_type->substitute_decl(subst, current_scope, global_scope)
      ->as_type();
  }

  bool any_changed = false;

  for (int i = 0; i < _elements.size(); ++i) {
    CPPInstance *elem_rep =
      _elements[i]->substitute_decl(subst, current_scope, global_scope)
      ->as_instance();

    if (elem_rep != _elements[i]) {
      rep->_elements[i] = elem_rep;
      any_changed = true;
    }
  }

  if (rep->_ident == _ident &&
      rep->_element_type == _element_type &&
      !any_changed) {
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

  } else {
    out << _type;
    if (_ident != NULL) {
      out << " " << _ident->get_local_name(scope);
    }
    if (_element_type != NULL) {
      out << " : " << _element_type->get_local_name(scope);
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
