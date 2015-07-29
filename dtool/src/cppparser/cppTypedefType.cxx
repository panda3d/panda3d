// Filename: cppTypedefType.cxx
// Created by:  rdb (01Aug14)
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

#include "cppTypedefType.h"
#include "cppIdentifier.h"
#include "cppInstanceIdentifier.h"

////////////////////////////////////////////////////////////////////
//     Function: CPPTypedefType::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPTypedefType::
CPPTypedefType(CPPType *type, const string &name, CPPScope *current_scope) :
  CPPType(CPPFile()),
  _type(type),
  _ident(new CPPIdentifier(name))
{
  if (_ident != NULL) {
    _ident->_native_scope = current_scope;
  }

  _subst_decl_recursive_protect = false;

  //assert(_type != NULL);
  //if (global) {
  //  _type->_typedefs.push_back(this);
  //  CPPType::record_alt_name_for(_type, inst->get_local_name());
  //}
}

////////////////////////////////////////////////////////////////////
//     Function: CPPTypedefType::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPTypedefType::
CPPTypedefType(CPPType *type, CPPIdentifier *ident, CPPScope *current_scope) :
  CPPType(CPPFile()),
  _type(type),
  _ident(ident)
{
  if (_ident != NULL) {
    _ident->_native_scope = current_scope;
  }
  _subst_decl_recursive_protect = false;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPTypedefType::Constructor
//       Access: Public
//  Description: Constructs a new CPPTypedefType object that defines a
//               typedef to the indicated type according to the type
//               and the InstanceIdentifier.  The InstanceIdentifier
//               pointer is deallocated.
////////////////////////////////////////////////////////////////////
CPPTypedefType::
CPPTypedefType(CPPType *type, CPPInstanceIdentifier *ii,
               CPPScope *current_scope, const CPPFile &file) :
  CPPType(file)
{
  _type = ii->unroll_type(type);
  _ident = ii->_ident;
  ii->_ident = NULL;
  delete ii;

  if (_ident != NULL) {
    _ident->_native_scope = current_scope;
  }

  _subst_decl_recursive_protect = false;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPTypedefType::is_scoped
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool CPPTypedefType::
is_scoped() const {
  if (_ident == NULL) {
    return false;
  } else {
    return _ident->is_scoped();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CPPTypedefType::get_scope
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPScope *CPPTypedefType::
get_scope(CPPScope *current_scope, CPPScope *global_scope,
          CPPPreprocessor *error_sink) const {
  if (_ident == NULL) {
    return current_scope;
  } else {
    return _ident->get_scope(current_scope, global_scope, error_sink);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CPPTypedefType::get_simple_name
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
string CPPTypedefType::
get_simple_name() const {
  if (_ident == NULL) {
    return "";
  }
  return _ident->get_simple_name();
}

////////////////////////////////////////////////////////////////////
//     Function: CPPTypedefType::get_local_name
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
string CPPTypedefType::
get_local_name(CPPScope *scope) const {
  if (_ident == NULL) {
    return "";
  }
  return _ident->get_local_name(scope);
}

////////////////////////////////////////////////////////////////////
//     Function: CPPTypedefType::get_fully_scoped_name
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
string CPPTypedefType::
get_fully_scoped_name() const {
  if (_ident == NULL) {
    return "";
  }
  return _ident->get_fully_scoped_name();
}

////////////////////////////////////////////////////////////////////
//     Function: CPPTypedefType::is_incomplete
//       Access: Public, Virtual
//  Description: Returns true if the type has not yet been fully
//               specified, false if it has.
////////////////////////////////////////////////////////////////////
bool CPPTypedefType::
is_incomplete() const {
  return false;
  //return _type->is_incomplete();
}

////////////////////////////////////////////////////////////////////
//     Function: CPPTypedefType::is_tbd
//       Access: Public, Virtual
//  Description: Returns true if the type, or any nested type within
//               the type, is a CPPTBDType and thus isn't fully
//               determined right now.  In this case, calling
//               resolve_type() may or may not resolve the type.
////////////////////////////////////////////////////////////////////
bool CPPTypedefType::
is_tbd() const {
  if (_ident != NULL && _ident->is_tbd()) {
    return true;
  }
  return _type->is_tbd();
}

////////////////////////////////////////////////////////////////////
//     Function: CPPTypedefType::is_trivial
//       Access: Public, Virtual
//  Description: Returns true if the type is considered a Plain Old
//               Data (POD) type.
////////////////////////////////////////////////////////////////////
bool CPPTypedefType::
is_trivial() const {
  return _type->is_trivial();
}

////////////////////////////////////////////////////////////////////
//     Function: CPPTypedefType::is_fully_specified
//       Access: Public, Virtual
//  Description: Returns true if this declaration is an actual,
//               factual declaration, or false if some part of the
//               declaration depends on a template parameter which has
//               not yet been instantiated.
////////////////////////////////////////////////////////////////////
bool CPPTypedefType::
is_fully_specified() const {
  if (_ident != NULL && !_ident->is_fully_specified()) {
    return false;
  }
  return CPPDeclaration::is_fully_specified() &&
    _type->is_fully_specified();
}

////////////////////////////////////////////////////////////////////
//     Function: CPPTypedefType::substitute_decl
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPDeclaration *CPPTypedefType::
substitute_decl(CPPDeclaration::SubstDecl &subst,
                CPPScope *current_scope, CPPScope *global_scope) {

  if (_ident != NULL && _ident->get_scope(current_scope, global_scope) == global_scope) {
    // Hack... I know that size_t etc is supposed to work fine, so
    // preserve these top-level typedefs.
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

  // Bah, this doesn't seem to work, and I can't figure out why.
  // Well, for now, let's just substitute it with the type we're
  // pointing to.  This is not a huge deal for now, until we find
  // that we need to preserve these typedefs.

  /*
  CPPDeclaration *top =
    CPPType::substitute_decl(subst, current_scope, global_scope);
  if (top != this) {
    return top;
  }

  if (_subst_decl_recursive_protect) {
    // We're already executing this block; we'll have to return a
    // proxy to the type which we'll define later.
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
  // Now fill in all the proxies we created for our recursive
  // references.
  Proxies::iterator pi;
  for (pi = _proxies.begin(); pi != _proxies.end(); ++pi) {
    (*pi)->_actual_type = rep;
  }

  return rep; */
}

////////////////////////////////////////////////////////////////////
//     Function: CPPFunctionType::resolve_type
//       Access: Public, Virtual
//  Description: If this CPPType object is a forward reference or
//               other nonspecified reference to a type that might now
//               be known a real type, returns the real type.
//               Otherwise returns the type itself.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: CPPTypedefType::is_equivalent_type
//       Access: Public, Virtual
//  Description: This is a little more forgiving than is_equal(): it
//               returns true if the types appear to be referring to
//               the same thing, even if they may have different
//               pointers or somewhat different definitions.  It's
//               useful for parameter matching, etc.
////////////////////////////////////////////////////////////////////
bool CPPTypedefType::
is_equivalent(const CPPType &other) const {
  CPPType *ot = (CPPType *)&other;

  // Unwrap all the typedefs to get to where it is pointing.
  while (ot->get_subtype() == ST_typedef) {
    ot = ot->as_typedef_type()->_type;
  }

  // Compare the unwrapped type to what we are pointing to.
  // If we are pointing to a typedef ourselves, then this will
  // automatically recurse.
  return _type->is_equivalent(*ot);
}

////////////////////////////////////////////////////////////////////
//     Function: CPPTypedefType::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CPPTypedefType::
output(ostream &out, int indent_level, CPPScope *scope, bool complete) const {
  string name;
  if (_ident != NULL) {
    name = _ident->get_local_name(scope);
  }

  if (complete) {
    out << "typedef ";
    _type->output_instance(out, indent_level, scope, false, "", name);
  } else {
    out << name;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CPPTypedefType::get_subtype
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPDeclaration::SubType CPPTypedefType::
get_subtype() const {
  return ST_typedef;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPTypedefType::as_typedef_type
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPTypedefType *CPPTypedefType::
as_typedef_type() {
  return this;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPTypedefType::is_equal
//       Access: Protected, Virtual
//  Description: Called by CPPDeclaration() to determine whether this type is
//               equivalent to another type of the same type.
////////////////////////////////////////////////////////////////////
bool CPPTypedefType::
is_equal(const CPPDeclaration *other) const {
  const CPPTypedefType *ot = ((CPPDeclaration *)other)->as_typedef_type();
  assert(ot != NULL);

  return (*_type == *ot->_type) && (*_ident == *ot->_ident);
}

////////////////////////////////////////////////////////////////////
//     Function: CPPTypedefType::is_less
//       Access: Protected, Virtual
//  Description: Called by CPPDeclaration() to determine whether this type
//               should be ordered before another type of the same
//               type, in an arbitrary but fixed ordering.
////////////////////////////////////////////////////////////////////
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
