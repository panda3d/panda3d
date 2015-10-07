// Filename: cppPointerType.cxx
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


#include "cppPointerType.h"
#include "cppFunctionType.h"
#include "cppIdentifier.h"

////////////////////////////////////////////////////////////////////
//     Function: CPPPointerType::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPPointerType::
CPPPointerType(CPPType *pointing_at) :
  CPPType(CPPFile()),
  _pointing_at(pointing_at)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPointerType::is_fully_specified
//       Access: Public, Virtual
//  Description: Returns true if this declaration is an actual,
//               factual declaration, or false if some part of the
//               declaration depends on a template parameter which has
//               not yet been instantiated.
////////////////////////////////////////////////////////////////////
bool CPPPointerType::
is_fully_specified() const {
  return CPPType::is_fully_specified() &&
    _pointing_at->is_fully_specified();
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPointerType::substitute_decl
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: CPPPointerType::resolve_type
//       Access: Public, Virtual
//  Description: If this CPPType object is a forward reference or
//               other nonspecified reference to a type that might now
//               be known a real type, returns the real type.
//               Otherwise returns the type itself.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: CPPPointerType::is_tbd
//       Access: Public, Virtual
//  Description: Returns true if the type, or any nested type within
//               the type, is a CPPTBDType and thus isn't fully
//               determined right now.  In this case, calling
//               resolve_type() may or may not resolve the type.
////////////////////////////////////////////////////////////////////
bool CPPPointerType::
is_tbd() const {
  return _pointing_at->is_tbd();
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPointerType::is_trivial
//       Access: Public, Virtual
//  Description: Returns true if the type is considered a Plain Old
//               Data (POD) type.
////////////////////////////////////////////////////////////////////
bool CPPPointerType::
is_trivial() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPointerType::is_equivalent
//       Access: Public, Virtual
//  Description: This is a little more forgiving than is_equal(): it
//               returns true if the types appear to be referring to
//               the same thing, even if they may have different
//               pointers or somewhat different definitions.  It's
//               useful for parameter matching, etc.
////////////////////////////////////////////////////////////////////
bool CPPPointerType::
is_equivalent(const CPPType &other) const {
  const CPPPointerType *ot = ((CPPType *)&other)->as_pointer_type();
  if (ot == (CPPPointerType *)NULL) {
    return CPPType::is_equivalent(other);
  }

  return _pointing_at->is_equivalent(*ot->_pointing_at);
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPointerType::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CPPPointerType::
output(ostream &out, int indent_level, CPPScope *scope, bool complete) const {
  /*
  CPPFunctionType *ftype = _pointing_at->as_function_type();
  if (ftype != (CPPFunctionType *)NULL) {
    // Pointers to functions are a bit of a special case; we have to
    // be a little more careful about where the '*' goes.

    string star = "*";
    if ((ftype->_flags & CPPFunctionType::F_method_pointer) != 0) {
      // We have to output pointers-to-method with a scoping before the
      // '*'.
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

////////////////////////////////////////////////////////////////////
//     Function: CPPPointerType::output_instance
//       Access: Public, Virtual
//  Description: Formats a C++-looking line that defines an instance
//               of the given type, with the indicated name.  In most
//               cases this will be "type name", but some types have
//               special exceptions.
////////////////////////////////////////////////////////////////////
void CPPPointerType::
output_instance(ostream &out, int indent_level, CPPScope *scope,
                bool complete, const string &prename,
                const string &name) const {
  string star = "*";

  CPPFunctionType *ftype = _pointing_at->as_function_type();
  if (ftype != NULL &&
      ((ftype->_flags & CPPFunctionType::F_method_pointer) != 0)) {
    // We have to output pointers-to-method with a scoping before the
    // '*'.
    star = ftype->_class_owner->get_fully_scoped_name() + "::*";
  }

  _pointing_at->output_instance(out, indent_level, scope, complete,
                                star + prename, name);
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPointerType::get_subtype
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPDeclaration::SubType CPPPointerType::
get_subtype() const {
  return ST_pointer;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPPointerType::as_pointer_type
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPPointerType *CPPPointerType::
as_pointer_type() {
  return this;
}


////////////////////////////////////////////////////////////////////
//     Function: CPPPointerType::is_equal
//       Access: Protected, Virtual
//  Description: Called by CPPDeclaration() to determine whether this type is
//               equivalent to another type of the same type.
////////////////////////////////////////////////////////////////////
bool CPPPointerType::
is_equal(const CPPDeclaration *other) const {
  const CPPPointerType *ot = ((CPPDeclaration *)other)->as_pointer_type();
  assert(ot != NULL);

  return _pointing_at == ot->_pointing_at;
}


////////////////////////////////////////////////////////////////////
//     Function: CPPPointerType::is_less
//       Access: Protected, Virtual
//  Description: Called by CPPDeclaration() to determine whether this type
//               should be ordered before another type of the same
//               type, in an arbitrary but fixed ordering.
////////////////////////////////////////////////////////////////////
bool CPPPointerType::
is_less(const CPPDeclaration *other) const {
  const CPPPointerType *ot = ((CPPDeclaration *)other)->as_pointer_type();
  assert(ot != NULL);

  return _pointing_at < ot->_pointing_at;
}
