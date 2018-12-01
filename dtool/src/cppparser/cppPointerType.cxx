/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cppPointerType.cxx
 * @author drose
 * @date 1999-10-19
 */

#include "cppPointerType.h"
#include "cppFunctionType.h"
#include "cppIdentifier.h"
#include "cppArrayType.h"
#include "cppStructType.h"
#include "cppSimpleType.h"

/**
 *
 */
CPPPointerType::
CPPPointerType(CPPType *pointing_at) :
  CPPType(CPPFile()),
  _pointing_at(pointing_at)
{
}

/**
 * Returns true if this declaration is an actual, factual declaration, or
 * false if some part of the declaration depends on a template parameter which
 * has not yet been instantiated.
 */
bool CPPPointerType::
is_fully_specified() const {
  return CPPType::is_fully_specified() &&
    _pointing_at->is_fully_specified();
}

/**
 *
 */
CPPDeclaration *CPPPointerType::
substitute_decl(CPPDeclaration::SubstDecl &subst,
                CPPScope *current_scope, CPPScope *global_scope) {
  SubstDecl::const_iterator si = subst.find(this);
  if (si != subst.end()) {
    return (*si).second;
  }

  CPPPointerType *rep = new CPPPointerType(*this);
  rep->_pointing_at =
    _pointing_at->substitute_decl(subst, current_scope, global_scope)
    ->as_type();

  if (rep->_pointing_at == _pointing_at) {
    delete rep;
    rep = this;
  }
  rep = CPPType::new_type(rep)->as_pointer_type();
  subst.insert(SubstDecl::value_type(this, rep));
  return rep;
}

/**
 * If this CPPType object is a forward reference or other nonspecified
 * reference to a type that might now be known a real type, returns the real
 * type.  Otherwise returns the type itself.
 */
CPPType *CPPPointerType::
resolve_type(CPPScope *current_scope, CPPScope *global_scope) {
  CPPType *ptype = _pointing_at->resolve_type(current_scope, global_scope);

  if (ptype != _pointing_at) {
    CPPPointerType *rep = new CPPPointerType(*this);
    rep->_pointing_at = ptype;
    return CPPType::new_type(rep);
  }
  return this;
}

/**
 * Returns true if the type, or any nested type within the type, is a
 * CPPTBDType and thus isn't fully determined right now.  In this case,
 * calling resolve_type() may or may not resolve the type.
 */
bool CPPPointerType::
is_tbd() const {
  return _pointing_at->is_tbd();
}

/**
 * Returns true if the type is considered a standard layout type.
 */
bool CPPPointerType::
is_standard_layout() const {
  return true;
}

/**
 * Returns true if the type is considered a Plain Old Data (POD) type.
 */
bool CPPPointerType::
is_trivial() const {
  return true;
}

/**
 * Returns true if the type can be constructed using the given argument.
 */
bool CPPPointerType::
is_constructible(const CPPType *given_type) const {
  given_type = ((CPPType *)given_type)->remove_reference()->remove_cv();

  // Can convert from compatible pointer or array type.
  CPPType *other_target;
  switch (given_type->get_subtype()) {
  case ST_array:
    other_target = given_type->as_array_type()->_element_type;
    break;

  case ST_pointer:
    other_target = given_type->as_pointer_type()->_pointing_at;
    break;

  case ST_simple:
    // Can initialize from nullptr.
    return given_type->as_simple_type()->_type == CPPSimpleType::T_nullptr;

  default:
    return false;
  }

  // Can't convert const to non-const pointer.
  if (other_target->is_const() && !_pointing_at->is_const()) {
    return false;
  }

  // Are we pointing to the same type?  That's always OK.
  const CPPType *a = _pointing_at->remove_cv();
  const CPPType *b = other_target->remove_cv();
  if (a == b || *a == *b) {
    return true;
  }

  // Can initialize void pointer with any pointer.
  const CPPSimpleType *simple_type = a->as_simple_type();
  if (simple_type != nullptr) {
    return simple_type->_type == CPPSimpleType::T_void;
  }

  // Can initialize from derived class pointer.
  const CPPStructType *a_struct = a->as_struct_type();
  const CPPStructType *b_struct = b->as_struct_type();
  if (a_struct != nullptr && b_struct != nullptr) {
    return a_struct->is_base_of(b_struct);
  }

  return false;
}

/**
 * Returns true if the type is default-constructible.
 */
bool CPPPointerType::
is_default_constructible() const {
  return true;
}

/**
 * Returns true if the type is copy-constructible.
 */
bool CPPPointerType::
is_copy_constructible() const {
  return true;
}

/**
 * Returns true if the type is copy-assignable.
 */
bool CPPPointerType::
is_copy_assignable() const {
  return true;
}

/**
 * This is a little more forgiving than is_equal(): it returns true if the
 * types appear to be referring to the same thing, even if they may have
 * different pointers or somewhat different definitions.  It's useful for
 * parameter matching, etc.
 */
bool CPPPointerType::
is_equivalent(const CPPType &other) const {
  const CPPPointerType *ot = ((CPPType *)&other)->as_pointer_type();
  if (ot == nullptr) {
    return CPPType::is_equivalent(other);
  }

  return _pointing_at->is_equivalent(*ot->_pointing_at);
}

/**
 *
 */
void CPPPointerType::
output(std::ostream &out, int indent_level, CPPScope *scope, bool complete) const {
  /*
  CPPFunctionType *ftype = _pointing_at->as_function_type();
  if (ftype != (CPPFunctionType *)NULL) {
    // Pointers to functions are a bit of a special case; we have to be a
    // little more careful about where the '*' goes.

    string star = "*";
    if ((ftype->_flags & CPPFunctionType::F_method_pointer) != 0) {
      // We have to output pointers-to-method with a scoping before the '*'.
      star = ftype->_class_owner->get_fully_scoped_name() + "::*";
    }

    _pointing_at->output_instance(out, indent_level, scope, complete,
                                  star, "");

  } else {
    _pointing_at->output(out, indent_level, scope, complete);
    out << " *";
  }
  */
  output_instance(out, indent_level, scope, complete, "", "");
}

/**
 * Formats a C++-looking line that defines an instance of the given type, with
 * the indicated name.  In most cases this will be "type name", but some types
 * have special exceptions.
 */
void CPPPointerType::
output_instance(std::ostream &out, int indent_level, CPPScope *scope,
                bool complete, const std::string &prename,
                const std::string &name) const {
  std::string star = "*";

  CPPFunctionType *ftype = _pointing_at->as_function_type();
  if (ftype != nullptr &&
      ((ftype->_flags & CPPFunctionType::F_method_pointer) != 0)) {
    // We have to output pointers-to-method with a scoping before the '*'.
    star = ftype->_class_owner->get_fully_scoped_name() + "::*";
  }

  _pointing_at->output_instance(out, indent_level, scope, complete,
                                star + prename, name);
}

/**
 *
 */
CPPDeclaration::SubType CPPPointerType::
get_subtype() const {
  return ST_pointer;
}

/**
 *
 */
CPPPointerType *CPPPointerType::
as_pointer_type() {
  return this;
}


/**
 * Called by CPPDeclaration() to determine whether this type is equivalent to
 * another type of the same type.
 */
bool CPPPointerType::
is_equal(const CPPDeclaration *other) const {
  const CPPPointerType *ot = ((CPPDeclaration *)other)->as_pointer_type();
  assert(ot != nullptr);

  return _pointing_at == ot->_pointing_at;
}


/**
 * Called by CPPDeclaration() to determine whether this type should be ordered
 * before another type of the same type, in an arbitrary but fixed ordering.
 */
bool CPPPointerType::
is_less(const CPPDeclaration *other) const {
  const CPPPointerType *ot = ((CPPDeclaration *)other)->as_pointer_type();
  assert(ot != nullptr);

  return _pointing_at < ot->_pointing_at;
}
