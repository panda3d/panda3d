/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cppArrayType.cxx
 * @author drose
 * @date 1999-10-19
 */

#include "cppArrayType.h"
#include "cppExpression.h"
#include "cppPointerType.h"

/**
 *
 */
CPPArrayType::
CPPArrayType(CPPType *element_type, CPPExpression *bounds) :
  CPPType(CPPFile()),
  _element_type(element_type),
  _bounds(bounds)
{
}

/**
 * Returns true if this declaration is an actual, factual declaration, or
 * false if some part of the declaration depends on a template parameter which
 * has not yet been instantiated.
 */
bool CPPArrayType::
is_fully_specified() const {
  return CPPType::is_fully_specified() &&
    _element_type->is_fully_specified();
}

/**
 * If this CPPType object is a forward reference or other nonspecified
 * reference to a type that might now be known a real type, returns the real
 * type.  Otherwise returns the type itself.
 */
CPPType *CPPArrayType::
resolve_type(CPPScope *current_scope, CPPScope *global_scope) {
  CPPType *ptype = _element_type->resolve_type(current_scope, global_scope);

  if (ptype != _element_type) {
    CPPArrayType *rep = new CPPArrayType(*this);
    rep->_element_type = ptype;
    return CPPType::new_type(rep);
  }
  return this;
}

/**
 * Returns true if the type, or any nested type within the type, is a
 * CPPTBDType and thus isn't fully determined right now.  In this case,
 * calling resolve_type() may or may not resolve the type.
 */
bool CPPArrayType::
is_tbd() const {
  return _element_type->is_tbd();
}

/**
 * Returns true if the type is considered a standard layout type.
 */
bool CPPArrayType::
is_standard_layout() const {
  return _element_type->is_standard_layout();
}

/**
 * Returns true if the type is considered a Plain Old Data (POD) type.
 */
bool CPPArrayType::
is_trivial() const {
  return _element_type->is_trivial();
}

/**
 * Returns true if the type can be safely copied by memcpy or memmove.
 */
bool CPPArrayType::
is_trivially_copyable() const {
  return _element_type->is_trivially_copyable();
}

/**
 * Returns true if the type is default-constructible.
 */
bool CPPArrayType::
is_default_constructible() const {
  return _bounds != nullptr && _element_type->is_default_constructible();
}

/**
 * Returns true if the type is copy-constructible.
 */
bool CPPArrayType::
is_copy_constructible() const {
  // This is technically not exactly true, but array data members do not
  // prevent C++ implicit copy constructor generation rules, so we need to
  // return true here.
  // If this is a problem, we will need to create a separate method for the
  // purpose of checking copyability as a data member.
  return true;
}

/**
 * Returns true if the type is copy-assignable.
 */
bool CPPArrayType::
is_copy_assignable() const {
  // Same story as is_copy_constructible.
  return true;
}

/**
 * This is a little more forgiving than is_equal(): it returns true if the
 * types appear to be referring to the same thing, even if they may have
 * different pointers or somewhat different definitions.  It's useful for
 * parameter matching, etc.
 */
bool CPPArrayType::
is_equivalent(const CPPType &other) const {
  const CPPArrayType *ot = ((CPPType *)&other)->as_array_type();
  if (ot == nullptr) {
    return CPPType::is_equivalent(other);
  }

  return _element_type->is_equivalent(*ot->_element_type);
}

/**
 *
 */
CPPDeclaration *CPPArrayType::
substitute_decl(CPPDeclaration::SubstDecl &subst,
                CPPScope *current_scope, CPPScope *global_scope) {
  SubstDecl::const_iterator si = subst.find(this);
  if (si != subst.end()) {
    return (*si).second;
  }

  CPPArrayType *rep = new CPPArrayType(*this);
  rep->_element_type =
    _element_type->substitute_decl(subst, current_scope, global_scope)
    ->as_type();

  if (_bounds != nullptr) {
    rep->_bounds =
      _bounds->substitute_decl(subst, current_scope, global_scope)
      ->as_expression();
  }

  if (rep->_element_type == _element_type &&
      rep->_bounds == _bounds) {
    delete rep;
    rep = this;
  }
  rep = CPPType::new_type(rep)->as_array_type();
  subst.insert(SubstDecl::value_type(this, rep));
  return rep;
}

/**
 *
 */
void CPPArrayType::
output(std::ostream &out, int indent_level, CPPScope *scope, bool complete) const {
  /*
  _element_type->output(out, indent_level, scope, complete);
  out << "[";
  if (_bounds != NULL) {
    out << *_bounds;
  }
  out << "]";
  */
  output_instance(out, indent_level, scope, complete, "", "");
}

/**
 * Formats a C++-looking line that defines an instance of the given type, with
 * the indicated name.  In most cases this will be "type name", but some types
 * have special exceptions.
 */
void CPPArrayType::
output_instance(std::ostream &out, int indent_level, CPPScope *scope,
                bool complete, const std::string &prename,
                const std::string &name) const {
  std::ostringstream brackets;
  brackets << "[";
  if (_bounds != nullptr) {
    brackets << *_bounds;
  }
  brackets << "]";

  if (!_attributes.is_empty()) {
    brackets << " " << _attributes;
  }

  std::string bracketsstr = brackets.str();

  _element_type->output_instance(out, indent_level, scope, complete,
                                 prename, name + bracketsstr);
}

/**
 *
 */
CPPDeclaration::SubType CPPArrayType::
get_subtype() const {
  return ST_array;
}

/**
 *
 */
CPPArrayType *CPPArrayType::
as_array_type() {
  return this;
}

/**
 * Called by CPPDeclaration() to determine whether this type is equivalent to
 * another type of the same type.
 */
bool CPPArrayType::
is_equal(const CPPDeclaration *other) const {
  const CPPArrayType *ot = ((CPPDeclaration *)other)->as_array_type();
  assert(ot != nullptr);

  if (_bounds != nullptr && ot->_bounds != nullptr) {
    if (*_bounds != *ot->_bounds) {
      return false;
    }
  } else if ((_bounds == nullptr) != (ot->_bounds == nullptr)) {
    return false;
  }

  return *_element_type == *ot->_element_type;
}


/**
 * Called by CPPDeclaration() to determine whether this type should be ordered
 * before another type of the same type, in an arbitrary but fixed ordering.
 */
bool CPPArrayType::
is_less(const CPPDeclaration *other) const {
  const CPPArrayType *ot = ((CPPDeclaration *)other)->as_array_type();
  assert(ot != nullptr);

  if (_bounds != nullptr && ot->_bounds != nullptr) {
    if (*_bounds != *ot->_bounds) {
      return *_bounds < *ot->_bounds;
    }
  } else if ((_bounds == nullptr) != (ot->_bounds == nullptr)) {
    return _bounds < ot->_bounds;
  }

  if (*_element_type != *ot->_element_type) {
    return *_element_type < *ot->_element_type;
  }
  return false;
}
