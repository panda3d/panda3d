/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cppReferenceType.cxx
 * @author drose
 * @date 1999-10-19
 */

#include "cppReferenceType.h"
#include "cppTypedefType.h"
#include "cppStructType.h"

/**
 *
 */
CPPReferenceType::
CPPReferenceType(CPPType *pointing_at, ValueCategory vcat) :
  CPPType(CPPFile()),
  _pointing_at(pointing_at),
  _value_category(vcat)
{
}

/**
 * Returns true if this declaration is an actual, factual declaration, or
 * false if some part of the declaration depends on a template parameter which
 * has not yet been instantiated.
 */
bool CPPReferenceType::
is_fully_specified() const {
  return CPPType::is_fully_specified() &&
    _pointing_at->is_fully_specified();
}

/**
 *
 */
CPPDeclaration *CPPReferenceType::
substitute_decl(CPPDeclaration::SubstDecl &subst,
                CPPScope *current_scope, CPPScope *global_scope) {
  SubstDecl::const_iterator si = subst.find(this);
  if (si != subst.end()) {
    return (*si).second;
  }

  CPPReferenceType *rep = new CPPReferenceType(*this);
  rep->_pointing_at =
    _pointing_at->substitute_decl(subst, current_scope, global_scope)
    ->as_type();

  if (rep->_pointing_at == _pointing_at) {
    delete rep;
    rep = this;
  }
  rep = CPPType::new_type(rep)->as_reference_type();
  subst.insert(SubstDecl::value_type(this, rep));
  return rep;
}

/**
 * If this CPPType object is a forward reference or other nonspecified
 * reference to a type that might now be known a real type, returns the real
 * type.  Otherwise returns the type itself.
 */
CPPType *CPPReferenceType::
resolve_type(CPPScope *current_scope, CPPScope *global_scope) {
  CPPType *ptype = _pointing_at->resolve_type(current_scope, global_scope);

  if (ptype != _pointing_at) {
    CPPReferenceType *rep = new CPPReferenceType(*this);
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
bool CPPReferenceType::
is_tbd() const {
  return _pointing_at->is_tbd();
}

/**
 * Returns true if the type is considered a standard layout type.
 */
bool CPPReferenceType::
is_standard_layout() const {
  return false;
}

/**
 * Returns true if the type is considered a Plain Old Data (POD) type.
 */
bool CPPReferenceType::
is_trivial() const {
  return false;
}

/**
 * Returns true if the type can be constructed using the given argument.
 */
bool CPPReferenceType::
is_constructible(const CPPType *given_type) const {
  const CPPType *a;
  const CPPType *b;

  CPPReferenceType *ref_type = ((CPPType *)given_type)->as_reference_type();
  if (ref_type != nullptr) {
    if (ref_type->_value_category == VC_rvalue) {
      return is_constructible(ref_type->_pointing_at);
    }

    if (_value_category == VC_rvalue) {
      // Can never initialize an rvalue ref from an lvalue ref.
      return false;
    }

    if (!_pointing_at->is_const()) {
      // Cannot initialize a non-const reference using a const one.
      if (ref_type->_pointing_at->is_const()) {
        return false;
      }
    }

    a = _pointing_at->remove_cv();
    b = ref_type->_pointing_at->remove_cv();

  } else {
    // Initializing using an rvalue.
    if (!_pointing_at->is_const()) {
      // Cannot initialize a non-const reference using a const one.
      if (given_type->is_const()) {
        return false;
      }

      // Cannot initalise a non-const lvalue reference with an rvalue ref.
      if (_value_category == VC_lvalue) {
        return false;
      }
    }

    a = _pointing_at->remove_cv();
    b = ((CPPType *)given_type)->remove_cv();
  }

  if (a == b || *a == *b) {
    return true;
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
bool CPPReferenceType::
is_default_constructible() const {
  return false;
}

/**
 * Returns true if the type is copy-constructible.
 */
bool CPPReferenceType::
is_copy_constructible() const {
  return (_value_category == VC_lvalue);
}

/**
 * Returns true if the type is destructible.
 */
bool CPPReferenceType::
is_destructible() const {
  return false;
}

/**
 * This is a little more forgiving than is_equal(): it returns true if the
 * types appear to be referring to the same thing, even if they may have
 * different pointers or somewhat different definitions.  It's useful for
 * parameter matching, etc.
 */
bool CPPReferenceType::
is_equivalent(const CPPType &other) const {
  const CPPReferenceType *ot = ((CPPType *)&other)->as_reference_type();
  if (ot == nullptr) {
    return CPPType::is_equivalent(other);
  }

  return _pointing_at->is_equivalent(*ot->_pointing_at);
}

/**
 *
 */
void CPPReferenceType::
output(std::ostream &out, int indent_level, CPPScope *scope, bool complete) const {
  /*
  _pointing_at->output(out, indent_level, scope, complete);
  out << " &";
  */
  output_instance(out, indent_level, scope, complete, "", "");
}

/**
 * Formats a C++-looking line that defines an instance of the given type, with
 * the indicated name.  In most cases this will be "type name", but some types
 * have special exceptions.
 */
void CPPReferenceType::
output_instance(std::ostream &out, int indent_level, CPPScope *scope,
                bool complete, const std::string &prename,
                const std::string &name) const {

  if (_value_category == VC_rvalue) {
    _pointing_at->output_instance(out, indent_level, scope, complete,
                                  "&&" + prename, name);
  } else {
    _pointing_at->output_instance(out, indent_level, scope, complete,
                                  "&" + prename, name);
  }
}

/**
 *
 */
CPPDeclaration::SubType CPPReferenceType::
get_subtype() const {
  return ST_reference;
}

/**
 *
 */
CPPReferenceType *CPPReferenceType::
as_reference_type() {
  return this;
}


/**
 * Called by CPPDeclaration() to determine whether this type is equivalent to
 * another type of the same type.
 */
bool CPPReferenceType::
is_equal(const CPPDeclaration *other) const {
  const CPPReferenceType *ot = ((CPPDeclaration *)other)->as_reference_type();
  assert(ot != nullptr);

  return (_pointing_at == ot->_pointing_at) &&
         (_value_category == ot->_value_category);
}


/**
 * Called by CPPDeclaration() to determine whether this type should be ordered
 * before another type of the same type, in an arbitrary but fixed ordering.
 */
bool CPPReferenceType::
is_less(const CPPDeclaration *other) const {
  const CPPReferenceType *ot = ((CPPDeclaration *)other)->as_reference_type();
  assert(ot != nullptr);

  if (_value_category != ot->_value_category) {
    return (_value_category < ot->_value_category);
  }

  return _pointing_at < ot->_pointing_at;
}
