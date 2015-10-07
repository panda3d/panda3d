// Filename: cppExtensionType.cxx
// Created by:  drose (21Oct99)
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


#include "cppExtensionType.h"
#include "cppTypedefType.h"
#include "cppIdentifier.h"
#include "cppParser.h"
#include "indent.h"

////////////////////////////////////////////////////////////////////
//     Function: CPPExtensionType::Conextensionor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPExtensionType::
CPPExtensionType(CPPExtensionType::Type type,
                 CPPIdentifier *ident, CPPScope *current_scope,
                 const CPPFile &file) :
  CPPType(file),
  _type(type), _ident(ident)
{
  if (_ident != NULL) {
    _ident->_native_scope = current_scope;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CPPExtensionType::get_simple_name
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
string CPPExtensionType::
get_simple_name() const {
  if (_ident == NULL) {
    return "";
  }
  return _ident->get_simple_name();
}

////////////////////////////////////////////////////////////////////
//     Function: CPPExtensionType::get_local_name
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
string CPPExtensionType::
get_local_name(CPPScope *scope) const {
  if (_ident == NULL) {
    return "";
  }
  return _ident->get_local_name(scope);
}

////////////////////////////////////////////////////////////////////
//     Function: CPPExtensionType::get_fully_scoped_name
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
string CPPExtensionType::
get_fully_scoped_name() const {
  if (_ident == NULL) {
    return "";
  }
  return _ident->get_fully_scoped_name();
}

////////////////////////////////////////////////////////////////////
//     Function: CPPExtensionType::is_incomplete
//       Access: Public, Virtual
//  Description: Returns true if the type has not yet been fully
//               specified, false if it has.
////////////////////////////////////////////////////////////////////
bool CPPExtensionType::
is_incomplete() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPExtensionType::is_tbd
//       Access: Public, Virtual
//  Description: Returns true if the type, or any nested type within
//               the type, is a CPPTBDType and thus isn't fully
//               determined right now.  In this case, calling
//               resolve_type() may or may not resolve the type.
////////////////////////////////////////////////////////////////////
bool CPPExtensionType::
is_tbd() const {
  if (_ident != (CPPIdentifier *)NULL) {
    return _ident->is_tbd();
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPExtensionType::is_trivial
//       Access: Public, Virtual
//  Description: Returns true if the type is considered a Plain Old
//               Data (POD) type.
////////////////////////////////////////////////////////////////////
bool CPPExtensionType::
is_trivial() const {
  return (_type == T_enum);
}

////////////////////////////////////////////////////////////////////
//     Function: CPPExtensionType::substitute_decl
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPDeclaration *CPPExtensionType::
substitute_decl(CPPDeclaration::SubstDecl &subst,
                CPPScope *current_scope, CPPScope *global_scope) {
  SubstDecl::const_iterator si = subst.find(this);
  if (si != subst.end()) {
    return (*si).second;
  }

  CPPExtensionType *rep = new CPPExtensionType(*this);
  if (_ident != NULL) {
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

////////////////////////////////////////////////////////////////////
//     Function: CPPExtensionType::resolve_type
//       Access: Public, Virtual
//  Description: If this CPPType object is a forward reference or
//               other nonspecified reference to a type that might now
//               be known a real type, returns the real type.
//               Otherwise returns the type itself.
////////////////////////////////////////////////////////////////////
CPPType *CPPExtensionType::
resolve_type(CPPScope *current_scope, CPPScope *global_scope) {
  if (_ident == NULL) {
    // We can't resolve anonymous types.  But that's OK, since they
    // can't be forward declared anyway.
    return this;
  }

  // Maybe it has been defined by now.
  CPPType *type = _ident->find_type(current_scope, global_scope);
  if (type != NULL) {
    return type;
  }
  return this;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPExtensionType::is_equivalent_type
//       Access: Public, Virtual
//  Description: This is a little more forgiving than is_equal(): it
//               returns true if the types appear to be referring to
//               the same thing, even if they may have different
//               pointers or somewhat different definitions.  It's
//               useful for parameter matching, etc.
////////////////////////////////////////////////////////////////////
bool CPPExtensionType::
is_equivalent(const CPPType &other) const {
  const CPPExtensionType *ot = ((CPPType *)&other)->as_extension_type();
  if (ot == (CPPExtensionType *)NULL) {
    return CPPType::is_equivalent(other);
  }

  // We consider two different extension types to be equivalent if
  // they have the same name.

  return *_ident == *ot->_ident;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPExtensionType::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CPPExtensionType::
output(ostream &out, int, CPPScope *scope, bool) const {
  if (_ident != NULL) {
    // If we have a name, use it.
    if (cppparser_output_class_keyword) {
      out << _type << " ";
    }
    out << _ident->get_local_name(scope);

  } else if (!_typedefs.empty()) {
    // If we have a typedef name, use it.
    out << _typedefs.front()->get_local_name(scope);

  } else {
    out << "(**unknown forward-reference type**)";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CPPExtensionType::get_subtype
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPDeclaration::SubType CPPExtensionType::
get_subtype() const {
  return ST_extension;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPExtensionType::as_extension_type
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPExtensionType *CPPExtensionType::
as_extension_type() {
  return this;
}

ostream &
operator << (ostream &out, CPPExtensionType::Type type) {
  switch (type) {
  case CPPExtensionType::T_enum:
    return out << "enum";

  case CPPExtensionType::T_class:
    return out << "class";

  case CPPExtensionType::T_struct:
    return out << "struct";

  case CPPExtensionType::T_union:
    return out << "union";

  default:
    return out << "***invalid extension type***";
  }
}
