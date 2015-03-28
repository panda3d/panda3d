// Filename: cppTypeProxy.cxx
// Created by:  drose (07Dec99)
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


#include "cppTypeProxy.h"
#include "cppFile.h"

////////////////////////////////////////////////////////////////////
//     Function: CPPTypeProxy::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPTypeProxy::
CPPTypeProxy() :
  CPPType(CPPFile())
{
  _actual_type = (CPPType *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPTypeProxy::resolve_type
//       Access: Public, Virtual
//  Description: If this CPPType object is a forward reference or
//               other nonspecified reference to a type that might now
//               be known a real type, returns the real type.
//               Otherwise returns the type itself.
////////////////////////////////////////////////////////////////////
CPPType *CPPTypeProxy::
resolve_type(CPPScope *, CPPScope *) {
  if (_actual_type == (CPPType *)NULL) {
    return this;
  }
  return _actual_type;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPTypeProxy::is_tbd
//       Access: Public, Virtual
//  Description: Returns true if the type, or any nested type within
//               the type, is a CPPTBDType and thus isn't fully
//               determined right now.  In this case, calling
//               resolve_type() may or may not resolve the type.
////////////////////////////////////////////////////////////////////
bool CPPTypeProxy::
is_tbd() const {
  if (_actual_type == (CPPType *)NULL) {
    return false;
  }
  return _actual_type->is_tbd();
}

////////////////////////////////////////////////////////////////////
//     Function: CPPTypeProxy::has_typedef_name
//       Access: Public
//  Description: Returns true if the type has even been typedef'ed and
//               therefore has a simple name available to stand for
//               it.  Extension types are all implicitly typedef'ed on
//               declaration.
////////////////////////////////////////////////////////////////////
bool CPPTypeProxy::
has_typedef_name() const {
  if (_actual_type == (CPPType *)NULL) {
    return false;
  }
  return _actual_type->has_typedef_name();
}

////////////////////////////////////////////////////////////////////
//     Function: CPPTypeProxy::get_typedef_name
//       Access: Public
//  Description: Returns a string that can be used to name the type,
//               if has_typedef_name() returned true.  This will be
//               the first typedef name applied to the type.
////////////////////////////////////////////////////////////////////
string CPPTypeProxy::
get_typedef_name(CPPScope *) const {
  if (_actual_type == (CPPType *)NULL) {
    return string();
  }
  return _actual_type->get_typedef_name();
}


////////////////////////////////////////////////////////////////////
//     Function: CPPTypeProxy::get_simple_name
//       Access: Public, Virtual
//  Description: Returns a fundametal one-word name for the type.
//               This name will not include any scoping operators or
//               template parameters, so it may not be a compilable
//               reference to the type.
////////////////////////////////////////////////////////////////////
string CPPTypeProxy::
get_simple_name() const {
  if (_actual_type == (CPPType *)NULL) {
    return "unknown";
  }
  return _actual_type->get_simple_name();
}

////////////////////////////////////////////////////////////////////
//     Function: CPPTypeProxy::get_local_name
//       Access: Public, Virtual
//  Description: Returns the compilable, correct name for this type
//               within the indicated scope.  If the scope is NULL,
//               within the scope the type is declared in.
////////////////////////////////////////////////////////////////////
string CPPTypeProxy::
get_local_name(CPPScope *scope) const {
  if (_actual_type == (CPPType *)NULL) {
    return "unknown";
  }
  return _actual_type->get_local_name(scope);
}

////////////////////////////////////////////////////////////////////
//     Function: CPPTypeProxy::get_fully_scoped_name
//       Access: Public, Virtual
//  Description: Returns the compilable, correct name for the type,
//               with completely explicit scoping.
////////////////////////////////////////////////////////////////////
string CPPTypeProxy::
get_fully_scoped_name() const {
  if (_actual_type == (CPPType *)NULL) {
    return "unknown";
  }
  return _actual_type->get_fully_scoped_name();
}

////////////////////////////////////////////////////////////////////
//     Function: CPPTypeProxy::get_preferred_name
//       Access: Public, Virtual
//  Description: Returns the best name to use for the type from a
//               programmer's point of view.  This will typically be a
//               typedef name if one is available, or the full C++
//               name if it is not.  The typedef may or may not be
//               visible within the current scope, so this type name
//               may not be compilable.
////////////////////////////////////////////////////////////////////
string CPPTypeProxy::
get_preferred_name() const {
  if (_actual_type == (CPPType *)NULL) {
    return "unknown";
  }
  return _actual_type->get_preferred_name();
}

////////////////////////////////////////////////////////////////////
//     Function: CPPTypeProxy::is_incomplete
//       Access: Public, Virtual
//  Description: Returns true if the type has not yet been fully
//               specified, false if it has.
////////////////////////////////////////////////////////////////////
bool CPPTypeProxy::
is_incomplete() const {
  if (_actual_type == (CPPType *)NULL) {
    return true;
  }
  return _actual_type->is_incomplete();
}

////////////////////////////////////////////////////////////////////
//     Function: CPPTypeProxy::output_instance
//       Access: Public, Virtual
//  Description: Formats a C++-looking line that defines an instance
//               of the given type, with the indicated name.  In most
//               cases this will be "type name", but some types have
//               special exceptions.
////////////////////////////////////////////////////////////////////
void CPPTypeProxy::
output_instance(ostream &out, int indent_level, CPPScope *scope,
                bool complete, const string &prename,
                const string &name) const {
  if (_actual_type == (CPPType *)NULL) {
    out << "unknown " << prename << name;
    return;
  }
  _actual_type->output_instance(out, indent_level, scope, complete,
                                prename, name);
}

////////////////////////////////////////////////////////////////////
//     Function: CPPTypeProxy::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CPPTypeProxy::
output(ostream &out, int indent_level, CPPScope *scope, bool complete) const {
  if (_actual_type == (CPPType *)NULL) {
    out << "unknown";
    return;
  }
  _actual_type->output(out, indent_level, scope, complete);
}


////////////////////////////////////////////////////////////////////
//     Function: CPPTypeProxy::get_subtype
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPDeclaration::SubType CPPTypeProxy::
get_subtype() const {
  return ST_type_proxy;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPTypeProxy::as_type
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPType *CPPTypeProxy::
as_type() {
  if (_actual_type == (CPPType *)NULL) {
    return this;
  }
  return _actual_type;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPTypeProxy::as_simple_type
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPSimpleType *CPPTypeProxy::
as_simple_type() {
  if (_actual_type == (CPPType *)NULL) {
    return (CPPSimpleType *)NULL;
  }
  return _actual_type->as_simple_type();
}

////////////////////////////////////////////////////////////////////
//     Function: CPPTypeProxy::as_pointer_type
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPPointerType *CPPTypeProxy::
as_pointer_type() {
  if (_actual_type == (CPPType *)NULL) {
    return (CPPPointerType *)NULL;
  }
  return _actual_type->as_pointer_type();
}

////////////////////////////////////////////////////////////////////
//     Function: CPPTypeProxy::as_reference_type
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPReferenceType *CPPTypeProxy::
as_reference_type() {
  if (_actual_type == (CPPType *)NULL) {
    return (CPPReferenceType *)NULL;
  }
  return _actual_type->as_reference_type();
}

////////////////////////////////////////////////////////////////////
//     Function: CPPTypeProxy::as_array_type
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPArrayType *CPPTypeProxy::
as_array_type() {
  if (_actual_type == (CPPType *)NULL) {
    return (CPPArrayType *)NULL;
  }
  return _actual_type->as_array_type();
}

////////////////////////////////////////////////////////////////////
//     Function: CPPTypeProxy::as_const_type
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPConstType *CPPTypeProxy::
as_const_type() {
  if (_actual_type == (CPPType *)NULL) {
    return (CPPConstType *)NULL;
  }
  return _actual_type->as_const_type();
}

////////////////////////////////////////////////////////////////////
//     Function: CPPTypeProxy::as_function_type
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPFunctionType *CPPTypeProxy::
as_function_type() {
  if (_actual_type == (CPPType *)NULL) {
    return (CPPFunctionType *)NULL;
  }
  return _actual_type->as_function_type();
}

////////////////////////////////////////////////////////////////////
//     Function: CPPTypeProxy::as_extension_type
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPExtensionType *CPPTypeProxy::
as_extension_type() {
  if (_actual_type == (CPPType *)NULL) {
    return (CPPExtensionType *)NULL;
  }
  return _actual_type->as_extension_type();
}

////////////////////////////////////////////////////////////////////
//     Function: CPPTypeProxy::as_struct_type
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPStructType *CPPTypeProxy::
as_struct_type() {
  if (_actual_type == (CPPType *)NULL) {
    return (CPPStructType *)NULL;
  }
  return _actual_type->as_struct_type();
}

////////////////////////////////////////////////////////////////////
//     Function: CPPTypeProxy::as_enum_type
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPEnumType *CPPTypeProxy::
as_enum_type() {
  if (_actual_type == (CPPType *)NULL) {
    return (CPPEnumType *)NULL;
  }
  return _actual_type->as_enum_type();
}

////////////////////////////////////////////////////////////////////
//     Function: CPPTypeProxy::as_tbd_type
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPTBDType *CPPTypeProxy::
as_tbd_type() {
  if (_actual_type == (CPPType *)NULL) {
    return (CPPTBDType *)NULL;
  }
  return _actual_type->as_tbd_type();
}

////////////////////////////////////////////////////////////////////
//     Function: CPPTypeProxy::as_typedef_type
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPTypedefType *CPPTypeProxy::
as_typedef_type() {
  if (_actual_type == (CPPType *)NULL) {
    return (CPPTypedefType *)NULL;
  }
  return _actual_type->as_typedef_type();
}

////////////////////////////////////////////////////////////////////
//     Function: CPPTypeProxy::as_type_proxy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPTypeProxy *CPPTypeProxy::
as_type_proxy() {
  return this;
}
