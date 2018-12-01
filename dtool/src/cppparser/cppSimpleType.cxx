/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cppSimpleType.cxx
 * @author drose
 * @date 1999-10-19
 */

#include "cppSimpleType.h"
#include "cppGlobals.h"

/**
 *
 */
CPPSimpleType::
CPPSimpleType(CPPSimpleType::Type type, int flags) :
  CPPType(CPPFile()),
  _type(type), _flags(flags)
{
}

/**
 * Returns true if the type, or any nested type within the type, is a
 * CPPTBDType and thus isn't fully determined right now.  In this case,
 * calling resolve_type() may or may not resolve the type.
 */
bool CPPSimpleType::
is_tbd() const {
  return (_type == T_unknown);
}

/**
 * Returns true if the type is a boolean, floating point or integral type.
 */
bool CPPSimpleType::
is_arithmetic() const {
  return (_type > T_unknown && _type < T_void);
}

/**
 * Returns true if the type is considered a fundamental type.
 */
bool CPPSimpleType::
is_fundamental() const {
  return (_type != T_unknown && _type != T_parameter && _type != T_auto);
}

/**
 * Returns true if the type is considered a standard layout type.
 */
bool CPPSimpleType::
is_standard_layout() const {
  return (_type != T_unknown && _type != T_parameter && _type != T_auto);
}

/**
 * Returns true if the type is considered a Plain Old Data (POD) type.
 */
bool CPPSimpleType::
is_trivial() const {
  return true;
}

/**
 * Returns true if the type can be constructed using the given argument.
 */
bool CPPSimpleType::
is_constructible(const CPPType *given_type) const {
  given_type = ((CPPType *)given_type)->remove_reference()->remove_cv();

  const CPPSimpleType *simple_type = given_type->as_simple_type();
  if (simple_type == nullptr) {
    return given_type->is_enum() && is_arithmetic();
  } else if (_type == T_nullptr) {
    return simple_type->_type == T_nullptr;
  } else if (_type == T_bool) {
    return simple_type->is_arithmetic() || simple_type->_type == T_nullptr;
  } else if (is_arithmetic()) {
    return simple_type->is_arithmetic();
  } else {
    return false;
  }
}

/**
 * Returns true if the type is default-constructible.
 */
bool CPPSimpleType::
is_default_constructible() const {
  return (_type != T_void);
}

/**
 * Returns true if the type is copy-constructible.
 */
bool CPPSimpleType::
is_copy_constructible() const {
  return (_type != T_void);
}

/**
 * Returns true if the type is copy-assignable.
 */
bool CPPSimpleType::
is_copy_assignable() const {
  return (_type != T_void);
}

/**
 * Returns true if the type is destructible.
 */
bool CPPSimpleType::
is_destructible() const {
  return (_type != T_void);
}

/**
 * Returns true if the type is a special parameter expression type.
 *
 * This sort of type is created to handle instance declarations that initially
 * look like function prototypes.
 */
bool CPPSimpleType::
is_parameter_expr() const {
  return (_type == T_parameter);
}

/**
 *
 */
std::string CPPSimpleType::
get_preferred_name() const {
  // Simple types always prefer to use their native types.
  return get_local_name();
}

/**
 *
 */
void CPPSimpleType::
output(std::ostream &out, int, CPPScope *, bool) const {
  if (_flags & F_unsigned) {
    out << "unsigned ";
  }

  if (_flags & F_signed) {
    out << "signed ";
  }

  if ((_type == T_int && (_flags & F_longlong) != 0) &&
      !cpp_longlong_keyword.empty()) {
    // It's a long long, and we have a specific long long type name.  This is
    // to output code for compilers that don't recognize "long long int".
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

  case T_nullptr:
    out << "decltype(nullptr)";
    break;

  case T_parameter:
    out << "parameter";
    break;

  case T_auto:
    out << "auto";
    break;

  default:
    out << "***invalid type***";
  }
}

/**
 *
 */
CPPDeclaration::SubType CPPSimpleType::
get_subtype() const {
  return ST_simple;
}

/**
 *
 */
CPPSimpleType *CPPSimpleType::
as_simple_type() {
  return this;
}


/**
 * Called by CPPDeclaration() to determine whether this type is equivalent to
 * another type of the same type.
 */
bool CPPSimpleType::
is_equal(const CPPDeclaration *other) const {
  const CPPSimpleType *ot = ((CPPDeclaration *)other)->as_simple_type();
  assert(ot != nullptr);

  return _type == ot->_type && _flags == ot->_flags;
}


/**
 * Called by CPPDeclaration() to determine whether this type should be ordered
 * before another type of the same type, in an arbitrary but fixed ordering.
 */
bool CPPSimpleType::
is_less(const CPPDeclaration *other) const {
  const CPPSimpleType *ot = ((CPPDeclaration *)other)->as_simple_type();
  assert(ot != nullptr);

  if (_type != ot->_type) {
    return _type < ot->_type;
  }
  return _flags < ot->_flags;
}
