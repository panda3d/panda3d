// Filename: cppSimpleType.cxx
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

#include "cppSimpleType.h"
#include "cppGlobals.h"

////////////////////////////////////////////////////////////////////
//     Function: CPPSimpleType::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPSimpleType::
CPPSimpleType(CPPSimpleType::Type type, int flags) :
  CPPType(CPPFile()),
  _type(type), _flags(flags)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CPPSimpleType::is_tbd
//       Access: Public, Virtual
//  Description: Returns true if the type, or any nested type within
//               the type, is a CPPTBDType and thus isn't fully
//               determined right now.  In this case, calling
//               resolve_type() may or may not resolve the type.
////////////////////////////////////////////////////////////////////
bool CPPSimpleType::
is_tbd() const {
  return (_type == T_unknown);
}

////////////////////////////////////////////////////////////////////
//     Function: CPPSimpleType::is_trivial
//       Access: Public, Virtual
//  Description: Returns true if the type is considered a Plain Old
//               Data (POD) type.
////////////////////////////////////////////////////////////////////
bool CPPSimpleType::
is_trivial() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPSimpleType::is_parameter_expr
//       Access: Public, Virtual
//  Description: Returns true if the type is a special parameter
//               expression type.
//
//               This sort of type is created to handle instance
//               declarations that initially look like function
//               prototypes.
////////////////////////////////////////////////////////////////////
bool CPPSimpleType::
is_parameter_expr() const {
  return (_type == T_parameter);
}

////////////////////////////////////////////////////////////////////
//     Function: CPPSimpleType::get_preferred_name
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
string CPPSimpleType::
get_preferred_name() const {
  // Simple types always prefer to use their native types.
  return get_local_name();
}

////////////////////////////////////////////////////////////////////
//     Function: CPPSimpleType::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CPPSimpleType::
output(ostream &out, int, CPPScope *, bool) const {
  if (_flags & F_unsigned) {
    out << "unsigned ";
  }

  if (_flags & F_signed) {
    out << "signed ";
  }

  if ((_type == T_int && (_flags & F_longlong) != 0) &&
      !cpp_longlong_keyword.empty()) {
    // It's a long long, and we have a specific long long type name.
    // This is to output code for compilers that don't recognize "long
    // long int".
    out << cpp_longlong_keyword;
    return;
  }

  if (_flags & F_longlong) {
    out << "long long ";
  } else if (_flags & F_long) {
    out << "long ";
  } else if (_flags & F_short) {
    out << "short ";
  }

  switch (_type) {
  case T_unknown:
    out << "unknown";
    break;

  case T_bool:
    out << "bool";
    break;

  case T_char:
    out << "char";
    break;

  case T_wchar_t:
    out << "wchar_t";
    break;

  case T_char16_t:
    out << "char16_t";
    break;

  case T_char32_t:
    out << "char32_t";
    break;

  case T_int:
    out << "int";
    break;

  case T_float:
    out << "float";
    break;

  case T_double:
    out << "double";
    break;

  case T_void:
    out << "void";
    break;

  case T_parameter:
    out << "parameter";
    break;

  default:
    out << "***invalid type***";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CPPSimpleType::get_subtype
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPDeclaration::SubType CPPSimpleType::
get_subtype() const {
  return ST_simple;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPSimpleType::as_simple_type
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPSimpleType *CPPSimpleType::
as_simple_type() {
  return this;
}


////////////////////////////////////////////////////////////////////
//     Function: CPPSimpleType::is_equal
//       Access: Protected, Virtual
//  Description: Called by CPPDeclaration() to determine whether this type is
//               equivalent to another type of the same type.
////////////////////////////////////////////////////////////////////
bool CPPSimpleType::
is_equal(const CPPDeclaration *other) const {
  const CPPSimpleType *ot = ((CPPDeclaration *)other)->as_simple_type();
  assert(ot != NULL);

  return _type == ot->_type && _flags == ot->_flags;
}


////////////////////////////////////////////////////////////////////
//     Function: CPPSimpleType::is_less
//       Access: Protected, Virtual
//  Description: Called by CPPDeclaration() to determine whether this type
//               should be ordered before another type of the same
//               type, in an arbitrary but fixed ordering.
////////////////////////////////////////////////////////////////////
bool CPPSimpleType::
is_less(const CPPDeclaration *other) const {
  const CPPSimpleType *ot = ((CPPDeclaration *)other)->as_simple_type();
  assert(ot != NULL);

  if (_type != ot->_type) {
    return _type < ot->_type;
  }
  return _flags < ot->_flags;
}
