// Filename: cppConstType.cxx
// Created by:  drose (28Oct99)
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


#include "cppConstType.h"

////////////////////////////////////////////////////////////////////
//     Function: CPPConstType::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPConstType::
CPPConstType(CPPType *wrapped_around) :
  CPPType(CPPFile()),
  _wrapped_around(wrapped_around)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CPPConstType::is_fully_specified
//       Access: Public, Virtual
//  Description: Returns true if this declaration is an actual,
//               factual declaration, or false if some part of the
//               declaration depends on a template parameter which has
//               not yet been instantiated.
////////////////////////////////////////////////////////////////////
bool CPPConstType::
is_fully_specified() const {
  return CPPType::is_fully_specified() &&
    _wrapped_around->is_fully_specified();
}

////////////////////////////////////////////////////////////////////
//     Function: CPPConstType::substitute_decl
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPDeclaration *CPPConstType::
substitute_decl(CPPDeclaration::SubstDecl &subst,
                CPPScope *current_scope, CPPScope *global_scope) {
  SubstDecl::const_iterator si = subst.find(this);
  if (si != subst.end()) {
    return (*si).second;
  }

  CPPConstType *rep = new CPPConstType(*this);
  rep->_wrapped_around =
    _wrapped_around->substitute_decl(subst, current_scope, global_scope)
    ->as_type();

  if (rep->_wrapped_around == _wrapped_around) {
    delete rep;
    rep = this;
  }
  rep = CPPType::new_type(rep)->as_const_type();
  subst.insert(SubstDecl::value_type(this, rep));
  return rep;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPConstType::resolve_type
//       Access: Public, Virtual
//  Description: If this CPPType object is a forward reference or
//               other nonspecified reference to a type that might now
//               be known a real type, returns the real type.
//               Otherwise returns the type itself.
////////////////////////////////////////////////////////////////////
CPPType *CPPConstType::
resolve_type(CPPScope *current_scope, CPPScope *global_scope) {
  CPPType *ptype = _wrapped_around->resolve_type(current_scope, global_scope);

  if (ptype != _wrapped_around) {
    CPPConstType *rep = new CPPConstType(*this);
    rep->_wrapped_around = ptype;
    return CPPType::new_type(rep);
  }
  return this;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPConstType::is_tbd
//       Access: Public, Virtual
//  Description: Returns true if the type, or any nested type within
//               the type, is a CPPTBDType and thus isn't fully
//               determined right now.  In this case, calling
//               resolve_type() may or may not resolve the type.
////////////////////////////////////////////////////////////////////
bool CPPConstType::
is_tbd() const {
  return _wrapped_around->is_tbd();
}

////////////////////////////////////////////////////////////////////
//     Function: CPPConstType::is_trivial
//       Access: Public, Virtual
//  Description: Returns true if the type is considered a Plain Old
//               Data (POD) type.
////////////////////////////////////////////////////////////////////
bool CPPConstType::
is_trivial() const {
  return _wrapped_around->is_trivial();
}

////////////////////////////////////////////////////////////////////
//     Function: CPPConstType::is_equivalent
//       Access: Public, Virtual
//  Description: This is a little more forgiving than is_equal(): it
//               returns true if the types appear to be referring to
//               the same thing, even if they may have different
//               pointers or somewhat different definitions.  It's
//               useful for parameter matching, etc.
////////////////////////////////////////////////////////////////////
bool CPPConstType::
is_equivalent(const CPPType &other) const {
  const CPPConstType *ot = ((CPPType *)&other)->as_const_type();
  if (ot == (CPPConstType *)NULL) {
    return CPPType::is_equivalent(other);
  }

  return _wrapped_around->is_equivalent(*ot->_wrapped_around);
}

////////////////////////////////////////////////////////////////////
//     Function: CPPConstType::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CPPConstType::
output(ostream &out, int indent_level, CPPScope *scope, bool complete) const {
  _wrapped_around->output(out, indent_level, scope, complete);
  out << " const";
}

////////////////////////////////////////////////////////////////////
//     Function: CPPConstType::output_instance
//       Access: Public, Virtual
//  Description: Formats a C++-looking line that defines an instance
//               of the given type, with the indicated name.  In most
//               cases this will be "type name", but some types have
//               special exceptions.
////////////////////////////////////////////////////////////////////
void CPPConstType::
output_instance(ostream &out, int indent_level, CPPScope *scope,
                bool complete, const string &prename,
                const string &name) const {
  _wrapped_around->output_instance(out, indent_level, scope, complete,
                                   "const " + prename, name);
}

////////////////////////////////////////////////////////////////////
//     Function: CPPConstType::get_subtype
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPDeclaration::SubType CPPConstType::
get_subtype() const {
  return ST_const;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPConstType::as_const_type
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPConstType *CPPConstType::
as_const_type() {
  return this;
}


////////////////////////////////////////////////////////////////////
//     Function: CPPConstType::is_equal
//       Access: Protected, Virtual
//  Description: Called by CPPDeclaration() to determine whether this type is
//               equivalent to another type of the same type.
////////////////////////////////////////////////////////////////////
bool CPPConstType::
is_equal(const CPPDeclaration *other) const {
  const CPPConstType *ot = ((CPPDeclaration *)other)->as_const_type();
  assert(ot != NULL);

  return _wrapped_around == ot->_wrapped_around;
}


////////////////////////////////////////////////////////////////////
//     Function: CPPConstType::is_less
//       Access: Protected, Virtual
//  Description: Called by CPPDeclaration() to determine whether this type
//               should be ordered before another type of the same
//               type, in an arbitrary but fixed ordering.
////////////////////////////////////////////////////////////////////
bool CPPConstType::
is_less(const CPPDeclaration *other) const {
  const CPPConstType *ot = ((CPPDeclaration *)other)->as_const_type();
  assert(ot != NULL);

  return _wrapped_around < ot->_wrapped_around;
}
