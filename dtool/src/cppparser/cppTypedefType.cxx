/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cppTypedefType.cxx
 * @author rdb
 * @date 2014-08-01
 */

#include "cppTypedefType.h"
#include "cppIdentifier.h"
#include "cppInstanceIdentifier.h"
#include "cppTemplateScope.h"
#include "indent.h"

using std::string;

/**
 *
 */
CPPTypedefType::
CPPTypedefType(CPPType *type, const string &name, CPPScope *current_scope) :
  CPPType(CPPFile()),
  _type(type),
  _ident(new CPPIdentifier(name)),
  _using(false)
{
  if (_ident != nullptr) {
    _ident->_native_scope = current_scope;
  }

  _subst_decl_recursive_protect = false;

  // assert(_type != NULL); if (global) { _type->_typedefs.push_back(this);
  // CPPType::record_alt_name_for(_type, inst->get_local_name()); }
}

/**
 *
 */
CPPTypedefType::
CPPTypedefType(CPPType *type, CPPIdentifier *ident, CPPScope *current_scope,
               CPPAttributeList attr) :
  CPPType(CPPFile()),
  _type(type),
  _ident(ident),
  _using(false)
{
  if (_ident != nullptr) {
    _ident->_native_scope = current_scope;
  }
  _subst_decl_recursive_protect = false;

  _attributes = std::move(attr);
}

/**
 * Constructs a new CPPTypedefType object that defines a typedef to the
 * indicated type according to the type and the InstanceIdentifier.  The
 * InstanceIdentifier pointer is deallocated.
 */
CPPTypedefType::
CPPTypedefType(CPPType *type, CPPInstanceIdentifier *ii,
               CPPScope *current_scope, const CPPFile &file) :
  CPPType(file),
  _using(false)
{
  assert(ii != nullptr);
  _type = ii->unroll_type(type);
  _ident = ii->_ident;
  _attributes = std::move(ii->_attributes);
  ii->_ident = nullptr;
  delete ii;

  if (_ident != nullptr) {
    _ident->_native_scope = current_scope;
  }

  _subst_decl_recursive_protect = false;
}

/**
 *
 */
bool CPPTypedefType::
is_scoped() const {
  if (_ident == nullptr) {
    return false;
  } else {
    return _ident->is_scoped();
  }
}

/**
 *
 */
CPPScope *CPPTypedefType::
get_scope(CPPScope *current_scope, CPPScope *global_scope,
          CPPPreprocessor *error_sink) const {
  if (_ident == nullptr) {
    return current_scope;
  } else {
    return _ident->get_scope(current_scope, global_scope, error_sink);
  }
}

/**
 *
 */
string CPPTypedefType::
get_simple_name() const {
  if (_ident == nullptr) {
    return "";
  }
  return _ident->get_simple_name();
}

/**
 *
 */
string CPPTypedefType::
get_local_name(CPPScope *scope) const {
  if (_ident == nullptr) {
    return "";
  }
  return _ident->get_local_name(scope);
}

/**
 *
 */
string CPPTypedefType::
get_fully_scoped_name() const {
  if (_ident == nullptr) {
    return "";
  }
  return _ident->get_fully_scoped_name();
}

/**
 * Returns true if the type has not yet been fully specified, false if it has.
 */
bool CPPTypedefType::
is_incomplete() const {
  return false;
  // return _type->is_incomplete();
}

/**
 * Returns true if the type, or any nested type within the type, is a
 * CPPTBDType and thus isn't fully determined right now.  In this case,
 * calling resolve_type() may or may not resolve the type.
 */
bool CPPTypedefType::
is_tbd() const {
  if (_ident != nullptr && _ident->is_tbd()) {
    return true;
  }
  return _type->is_tbd();
}

/**
 * Returns true if the type is considered a fundamental type.
 */
bool CPPTypedefType::
is_fundamental() const {
  return _type->is_fundamental();
}

/**
 * Returns true if the type is considered a standard layout type.
 */
bool CPPTypedefType::
is_standard_layout() const {
  return _type->is_standard_layout();
}

/**
 * Returns true if the type is considered a Plain Old Data (POD) type.
 */
bool CPPTypedefType::
is_trivial() const {
  return _type->is_trivial();
}

/**
 * Returns true if the type can be safely copied by memcpy or memmove.
 */
bool CPPTypedefType::
is_trivially_copyable() const {
  return _type->is_trivially_copyable();
}

/**
 * Returns true if the type can be constructed using the given argument.
 */
bool CPPTypedefType::
is_constructible(const CPPType *given_type) const {
  return _type->is_constructible(given_type);
}

/**
 * Returns true if the type is default-constructible.
 */
bool CPPTypedefType::
is_default_constructible() const {
  return _type->is_default_constructible();
}

/**
 * Returns true if the type is copy-constructible.
 */
bool CPPTypedefType::
is_copy_constructible() const {
  return _type->is_copy_constructible();
}

/**
 * Returns true if the type is copy-assignable.
 */
bool CPPTypedefType::
is_copy_assignable() const {
  return _type->is_copy_assignable();
}

/**
 * Returns true if the type is destructible.
 */
bool CPPTypedefType::
is_destructible() const {
  return _type->is_destructible();
}

/**
 * Returns true if this declaration is an actual, factual declaration, or
 * false if some part of the declaration depends on a template parameter which
 * has not yet been instantiated.
 */
bool CPPTypedefType::
is_fully_specified() const {
  if (_ident != nullptr && !_ident->is_fully_specified()) {
    return false;
  }
  return CPPDeclaration::is_fully_specified() &&
    _type->is_fully_specified();
}

/**
 *
 */
CPPDeclaration *CPPTypedefType::
instantiate(const CPPTemplateParameterList *actual_params,
            CPPScope *current_scope, CPPScope *global_scope,
            CPPPreprocessor *error_sink) const {

  return _type->instantiate(actual_params, current_scope, global_scope, error_sink);
}

/**
 *
 */
CPPDeclaration *CPPTypedefType::
substitute_decl(CPPDeclaration::SubstDecl &subst,
                CPPScope *current_scope, CPPScope *global_scope) {

  if (_ident != nullptr && _ident->get_scope(current_scope, global_scope) == global_scope) {
    // Hack... I know that size_t etc is supposed to work fine, so preserve
    // these top-level typedefs.
    CPPDeclaration *top =
      CPPType::substitute_decl(subst, current_scope, global_scope);
    if (top != this) {
      return top;
    }
    top = new CPPTypedefType(*this);
    subst.insert(SubstDecl::value_type(this, top));
    return top;
  }

  return _type->substitute_decl(subst, current_scope, global_scope);

  // Bah, this doesn't seem to work, and I can't figure out why.  Well, for
  // now, let's just substitute it with the type we're pointing to.  This is
  // not a huge deal for now, until we find that we need to preserve these
  // typedefs.

  /*
  CPPDeclaration *top =
    CPPType::substitute_decl(subst, current_scope, global_scope);
  if (top != this) {
    return top;
  }

  if (_subst_decl_recursive_protect) {
    // We're already executing this block; we'll have to return a proxy to the
    // type which we'll define later.
    CPPTypeProxy *proxy = new CPPTypeProxy;
    _proxies.push_back(proxy);
    assert(proxy != NULL);
    return proxy;
  }
  _subst_decl_recursive_protect = true;

  CPPTypedefType *rep = new CPPTypedefType(*this);
  CPPDeclaration *new_type =
    _type->substitute_decl(subst, current_scope, global_scope);
  rep->_type = new_type->as_type();

  if (rep->_type == NULL) {
    rep->_type = _type;
  }

  if (_ident != NULL) {
    rep->_ident =
      _ident->substitute_decl(subst, current_scope, global_scope);
  }

  if (rep->_type == _type && rep->_ident == _ident) {
    delete rep;
    rep = this;
  }

  rep = CPPType::new_type(rep)->as_typedef_type();
  subst.insert(SubstDecl::value_type(this, rep));

  _subst_decl_recursive_protect = false;
  // Now fill in all the proxies we created for our recursive references.
  Proxies::iterator pi;
  for (pi = _proxies.begin(); pi != _proxies.end(); ++pi) {
    (*pi)->_actual_type = rep;
  }

  return rep; */
}

/**
 * If this CPPType object is a forward reference or other nonspecified
 * reference to a type that might now be known a real type, returns the real
 * type.  Otherwise returns the type itself.
 */
CPPType *CPPTypedefType::
resolve_type(CPPScope *current_scope, CPPScope *global_scope) {
  CPPType *ptype = _type->resolve_type(current_scope, global_scope);

  if (ptype != _type) {
    CPPTypedefType *rep = new CPPTypedefType(*this);
    rep->_type = ptype;
    return CPPType::new_type(rep);
  }

  return this;
}

/**
 * Returns true if variables of this type may be implicitly converted to
 * the other type.
 */
bool CPPTypedefType::
is_convertible_to(const CPPType *other) const {
  return _type->is_convertible_to(other);
}

/**
 * This is a little more forgiving than is_equal(): it returns true if the
 * types appear to be referring to the same thing, even if they may have
 * different pointers or somewhat different definitions.  It's useful for
 * parameter matching, etc.
 */
bool CPPTypedefType::
is_equivalent(const CPPType &other) const {
  CPPType *ot = (CPPType *)&other;

  // Unwrap all the typedefs to get to where it is pointing.
  while (ot->get_subtype() == ST_typedef) {
    ot = ot->as_typedef_type()->_type;
  }

  // Compare the unwrapped type to what we are pointing to.  If we are
  // pointing to a typedef ourselves, then this will automatically recurse.
  return _type->is_equivalent(*ot);
}

/**
 *
 */
void CPPTypedefType::
output(std::ostream &out, int indent_level, CPPScope *scope, bool complete) const {
  string name;
  if (_ident != nullptr) {
    name = _ident->get_local_name(scope);
  }

  if (complete) {
    if (_using) {
      // It was declared using the "using" keyword.
      if (is_template()) {
        get_template_scope()->_parameters.write_formal(out, scope);
        indent(out, indent_level);
      }
      out << "using " << name;
      if (!_attributes.is_empty()) {
        out << " " << _attributes;
      }
      out << " = ";
      _type->output(out, 0, scope, false);
    } else {
      if (!_attributes.is_empty()) {
        out << _attributes << " ";
      }
      out << "typedef ";
      _type->output_instance(out, indent_level, scope, false, "", name);
    }
  } else {
    out << name;
  }
}

/**
 *
 */
CPPDeclaration::SubType CPPTypedefType::
get_subtype() const {
  return ST_typedef;
}

/**
 *
 */
CPPTypedefType *CPPTypedefType::
as_typedef_type() {
  return this;
}

/**
 * Called by CPPDeclaration() to determine whether this type is equivalent to
 * another type of the same type.
 */
bool CPPTypedefType::
is_equal(const CPPDeclaration *other) const {
  const CPPTypedefType *ot = ((CPPDeclaration *)other)->as_typedef_type();
  assert(ot != nullptr);

  return (*_type == *ot->_type) && (*_ident == *ot->_ident) && (_using == ot->_using);
}

/**
 * Called by CPPDeclaration() to determine whether this type should be ordered
 * before another type of the same type, in an arbitrary but fixed ordering.
 */
bool CPPTypedefType::
is_less(const CPPDeclaration *other) const {
  return CPPDeclaration::is_less(other);

  // The below code causes a crash for unknown reasons.
  /*
  const CPPTypedefType *ot = ((CPPDeclaration *)other)->as_typedef_type();
  assert(ot != NULL);

  if (_type != ot->_type) {
    return _type < ot->_type;
  }

  if (*_ident != *ot->_ident) {
    return *_ident < *ot->_ident;
  }

  return false; */
}
