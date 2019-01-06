/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cppTBDType.cxx
 * @author drose
 * @date 1999-11-05
 */

#include "cppTBDType.h"
#include "cppIdentifier.h"

#include "cppSimpleType.h"

/**
 *
 */
CPPTBDType::
CPPTBDType(CPPIdentifier *ident) :
  CPPType(CPPFile()),
  _ident(ident)
{
  _subst_decl_recursive_protect = false;
}

/**
 * If this CPPType object is a forward reference or other nonspecified
 * reference to a type that might now be known a real type, returns the real
 * type.  Otherwise returns the type itself.
 */
CPPType *CPPTBDType::
resolve_type(CPPScope *current_scope, CPPScope *global_scope) {
  CPPType *type = _ident->find_type(current_scope, global_scope);
  if (type != nullptr) {
    return type;
  }
  return this;
}

/**
 * Returns true if the type, or any nested type within the type, is a
 * CPPTBDType and thus isn't fully determined right now.  In this case,
 * calling resolve_type() may or may not resolve the type.
 */
bool CPPTBDType::
is_tbd() const {
  return true;
}

/**
 * Returns a fundametal one-word name for the type.  This name will not
 * include any scoping operators or template parameters, so it may not be a
 * compilable reference to the type.
 */
std::string CPPTBDType::
get_simple_name() const {
  return _ident->get_simple_name();
}

/**
 * Returns the compilable, correct name for this type within the indicated
 * scope.  If the scope is NULL, within the scope the type is declared in.
 */
std::string CPPTBDType::
get_local_name(CPPScope *scope) const {
  return _ident->get_local_name(scope);
}

/**
 * Returns the compilable, correct name for the type, with completely explicit
 * scoping.
 */
std::string CPPTBDType::
get_fully_scoped_name() const {
  return _ident->get_fully_scoped_name();
}

/**
 *
 */
CPPDeclaration *CPPTBDType::
substitute_decl(CPPDeclaration::SubstDecl &subst,
                CPPScope *current_scope, CPPScope *global_scope) {
  CPPDeclaration *top =
    CPPDeclaration::substitute_decl(subst, current_scope, global_scope);
  if (top != this) {
    return top;
  }

  // Protect against recursive entry into this function block.  I know it's
  // ugly--have you got any better suggestions?
  if (_subst_decl_recursive_protect) {
    // We're already executing this block.
    return this;
  }
  _subst_decl_recursive_protect = true;

  CPPTBDType *rep = new CPPTBDType(*this);
  rep->_ident = _ident->substitute_decl(subst, current_scope, global_scope);

  if (rep->_ident == _ident) {
    delete rep;
    rep = this;
  }

  rep = CPPType::new_type(rep)->as_tbd_type();
  assert(rep != nullptr);

  CPPType *result = rep;

  // Can we now define it as a real type?
  CPPType *type = rep->_ident->find_type(current_scope, global_scope, subst);
  if (type != nullptr) {
    result = type;
  }

  subst.insert(SubstDecl::value_type(this, result));

  _subst_decl_recursive_protect = false;
  return result;
}

/**
 *
 */
void CPPTBDType::
output(std::ostream &out, int, CPPScope *, bool) const {
  out /* << "typename " */ << *_ident;
}

/**
 *
 */
CPPDeclaration::SubType CPPTBDType::
get_subtype() const {
  return ST_tbd;
}

/**
 *
 */
CPPTBDType *CPPTBDType::
as_tbd_type() {
  return this;
}


/**
 * Called by CPPDeclaration() to determine whether this type is equivalent to
 * another type of the same type.
 */
bool CPPTBDType::
is_equal(const CPPDeclaration *other) const {
  const CPPTBDType *ot = ((CPPDeclaration *)other)->as_tbd_type();
  assert(ot != nullptr);

  return (*_ident) == (*ot->_ident);
}


/**
 * Called by CPPDeclaration() to determine whether this type should be ordered
 * before another type of the same type, in an arbitrary but fixed ordering.
 */
bool CPPTBDType::
is_less(const CPPDeclaration *other) const {
  const CPPTBDType *ot = ((CPPDeclaration *)other)->as_tbd_type();
  assert(ot != nullptr);

  return (*_ident) < (*ot->_ident);
}
