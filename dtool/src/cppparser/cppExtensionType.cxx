/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cppExtensionType.cxx
 * @author drose
 * @date 1999-10-21
 */

#include "cppExtensionType.h"
#include "cppTypedefType.h"
#include "cppIdentifier.h"
#include "cppParser.h"
#include "indent.h"

/**
 *
 */
CPPExtensionType::
CPPExtensionType(CPPExtensionType::Type type,
                 CPPIdentifier *ident, CPPScope *current_scope,
                 const CPPFile &file, CPPAttributeList attr) :
  CPPType(file),
  _type(type), _ident(ident),
  _alignment(nullptr)
{
  if (_ident != nullptr) {
    _ident->_native_scope = current_scope;
  }
  _attributes = std::move(attr);
}

/**
 *
 */
std::string CPPExtensionType::
get_simple_name() const {
  if (_ident == nullptr) {
    return "";
  }
  return _ident->get_simple_name();
}

/**
 *
 */
std::string CPPExtensionType::
get_local_name(CPPScope *scope) const {
  if (_ident == nullptr) {
    return "";
  }
  return _ident->get_local_name(scope);
}

/**
 *
 */
std::string CPPExtensionType::
get_fully_scoped_name() const {
  if (_ident == nullptr) {
    return "";
  }
  return _ident->get_fully_scoped_name();
}

/**
 * Returns true if the type has not yet been fully specified, false if it has.
 */
bool CPPExtensionType::
is_incomplete() const {
  return true;
}

/**
 * Returns true if the type, or any nested type within the type, is a
 * CPPTBDType and thus isn't fully determined right now.  In this case,
 * calling resolve_type() may or may not resolve the type.
 */
bool CPPExtensionType::
is_tbd() const {
  if (_ident != nullptr) {
    return _ident->is_tbd();
  }
  return false;
}

/**
 * Returns true if the type is considered a standard layout type.
 */
bool CPPExtensionType::
is_standard_layout() const {
  return (_type == T_enum || _type == T_enum_class || _type == T_enum_struct);
}

/**
 * Returns true if the type is considered a Plain Old Data (POD) type.
 */
bool CPPExtensionType::
is_trivial() const {
  return (_type == T_enum || _type == T_enum_class || _type == T_enum_struct);
}

/**
 * Returns true if the type can be safely copied by memcpy or memmove.
 */
bool CPPExtensionType::
is_trivially_copyable() const {
  return (_type == T_enum || _type == T_enum_class || _type == T_enum_struct);
}

/**
 * Returns true if the type can be constructed using the given argument.
 */
bool CPPExtensionType::
is_constructible(const CPPType *given_type) const {
  if (_type == T_enum || _type == T_enum_class || _type == T_enum_struct) {
    const CPPExtensionType *other = ((CPPType *)given_type)->remove_reference()->remove_const()->as_extension_type();
    return other != nullptr && is_equal(other);
  }
  return false;
}

/**
 * Returns true if the type is default-constructible.
 */
bool CPPExtensionType::
is_default_constructible() const {
  return (_type == T_enum || _type == T_enum_class || _type == T_enum_struct);
}

/**
 * Returns true if the type is copy-constructible.
 */
bool CPPExtensionType::
is_copy_constructible() const {
  return (_type == T_enum || _type == T_enum_class || _type == T_enum_struct);
}

/**
 * Returns true if the type is copy-assignable.
 */
bool CPPExtensionType::
is_copy_assignable() const {
  return (_type == T_enum || _type == T_enum_class || _type == T_enum_struct);
}

/**
 *
 */
CPPDeclaration *CPPExtensionType::
substitute_decl(CPPDeclaration::SubstDecl &subst,
                CPPScope *current_scope, CPPScope *global_scope) {
  SubstDecl::const_iterator si = subst.find(this);
  if (si != subst.end()) {
    return (*si).second;
  }

  CPPExtensionType *rep = new CPPExtensionType(*this);
  if (_ident != nullptr) {
    rep->_ident =
      _ident->substitute_decl(subst, current_scope, global_scope);
  }

  if (rep->_ident == _ident) {
    delete rep;
    rep = this;
  }
  rep = CPPType::new_type(rep)->as_extension_type();
  subst.insert(SubstDecl::value_type(this, rep));
  return rep;
}

/**
 * If this CPPType object is a forward reference or other nonspecified
 * reference to a type that might now be known a real type, returns the real
 * type.  Otherwise returns the type itself.
 */
CPPType *CPPExtensionType::
resolve_type(CPPScope *current_scope, CPPScope *global_scope) {
  if (_ident == nullptr) {
    // We can't resolve anonymous types.  But that's OK, since they can't be
    // forward declared anyway.
    return this;
  }

  // Maybe it has been defined by now.
  CPPType *type = _ident->find_type(current_scope, global_scope);
  if (type != nullptr) {
    return type;
  }
  return this;
}

/**
 * This is a little more forgiving than is_equal(): it returns true if the
 * types appear to be referring to the same thing, even if they may have
 * different pointers or somewhat different definitions.  It's useful for
 * parameter matching, etc.
 */
bool CPPExtensionType::
is_equivalent(const CPPType &other) const {
  const CPPExtensionType *ot = ((CPPType *)&other)->as_extension_type();
  if (ot == nullptr) {
    return CPPType::is_equivalent(other);
  }

  // We consider two different extension types to be equivalent if they have
  // the same name.

  return _ident != nullptr && ot->_ident != nullptr && *_ident == *ot->_ident;
}

/**
 *
 */
void CPPExtensionType::
output(std::ostream &out, int, CPPScope *scope, bool complete) const {
  if (_ident != nullptr) {
    // If we have a name, use it.
    if (complete || cppparser_output_class_keyword) {
      out << _type << " ";
    }
    if (complete && !_attributes.is_empty()) {
      out << _attributes << " ";
    }
    out << _ident->get_local_name(scope);

  } else if (!_typedefs.empty()) {
    // If we have a typedef name, use it.
    out << _typedefs.front()->get_local_name(scope);

  } else {
    out << "(**unknown forward-reference type**)";
  }
}

/**
 *
 */
CPPDeclaration::SubType CPPExtensionType::
get_subtype() const {
  return ST_extension;
}

/**
 *
 */
CPPExtensionType *CPPExtensionType::
as_extension_type() {
  return this;
}

std::ostream &
operator << (std::ostream &out, CPPExtensionType::Type type) {
  switch (type) {
  case CPPExtensionType::T_enum:
    return out << "enum";

  case CPPExtensionType::T_class:
    return out << "class";

  case CPPExtensionType::T_struct:
    return out << "struct";

  case CPPExtensionType::T_union:
    return out << "union";

  case CPPExtensionType::T_enum_class:
    return out << "enum class";

  case CPPExtensionType::T_enum_struct:
    return out << "enum struct";

  default:
    return out << "***invalid extension type***";
  }
}
