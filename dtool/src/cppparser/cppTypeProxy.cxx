/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cppTypeProxy.cxx
 * @author drose
 * @date 1999-12-07
 */

#include "cppTypeProxy.h"
#include "cppFile.h"

using std::string;

/**
 *
 */
CPPTypeProxy::
CPPTypeProxy() :
  CPPType(CPPFile())
{
  _actual_type = nullptr;
}

/**
 * If this CPPType object is a forward reference or other nonspecified
 * reference to a type that might now be known a real type, returns the real
 * type.  Otherwise returns the type itself.
 */
CPPType *CPPTypeProxy::
resolve_type(CPPScope *, CPPScope *) {
  if (_actual_type == nullptr) {
    return this;
  }
  return _actual_type;
}

/**
 * Returns true if the type, or any nested type within the type, is a
 * CPPTBDType and thus isn't fully determined right now.  In this case,
 * calling resolve_type() may or may not resolve the type.
 */
bool CPPTypeProxy::
is_tbd() const {
  if (_actual_type == nullptr) {
    return false;
  }
  return _actual_type->is_tbd();
}

/**
 * Returns true if the type has even been typedef'ed and therefore has a
 * simple name available to stand for it.  Extension types are all implicitly
 * typedef'ed on declaration.
 */
bool CPPTypeProxy::
has_typedef_name() const {
  if (_actual_type == nullptr) {
    return false;
  }
  return _actual_type->has_typedef_name();
}

/**
 * Returns a string that can be used to name the type, if has_typedef_name()
 * returned true.  This will be the first typedef name applied to the type.
 */
string CPPTypeProxy::
get_typedef_name(CPPScope *) const {
  if (_actual_type == nullptr) {
    return string();
  }
  return _actual_type->get_typedef_name();
}


/**
 * Returns a fundametal one-word name for the type.  This name will not
 * include any scoping operators or template parameters, so it may not be a
 * compilable reference to the type.
 */
string CPPTypeProxy::
get_simple_name() const {
  if (_actual_type == nullptr) {
    return "unknown";
  }
  return _actual_type->get_simple_name();
}

/**
 * Returns the compilable, correct name for this type within the indicated
 * scope.  If the scope is NULL, within the scope the type is declared in.
 */
string CPPTypeProxy::
get_local_name(CPPScope *scope) const {
  if (_actual_type == nullptr) {
    return "unknown";
  }
  return _actual_type->get_local_name(scope);
}

/**
 * Returns the compilable, correct name for the type, with completely explicit
 * scoping.
 */
string CPPTypeProxy::
get_fully_scoped_name() const {
  if (_actual_type == nullptr) {
    return "unknown";
  }
  return _actual_type->get_fully_scoped_name();
}

/**
 * Returns the best name to use for the type from a programmer's point of
 * view.  This will typically be a typedef name if one is available, or the
 * full C++ name if it is not.  The typedef may or may not be visible within
 * the current scope, so this type name may not be compilable.
 */
string CPPTypeProxy::
get_preferred_name() const {
  if (_actual_type == nullptr) {
    return "unknown";
  }
  return _actual_type->get_preferred_name();
}

/**
 * Returns true if the type has not yet been fully specified, false if it has.
 */
bool CPPTypeProxy::
is_incomplete() const {
  if (_actual_type == nullptr) {
    return true;
  }
  return _actual_type->is_incomplete();
}

/**
 * Formats a C++-looking line that defines an instance of the given type, with
 * the indicated name.  In most cases this will be "type name", but some types
 * have special exceptions.
 */
void CPPTypeProxy::
output_instance(std::ostream &out, int indent_level, CPPScope *scope,
                bool complete, const string &prename,
                const string &name) const {
  if (_actual_type == nullptr) {
    out << "unknown " << prename << name;
    return;
  }
  _actual_type->output_instance(out, indent_level, scope, complete,
                                prename, name);
}

/**
 *
 */
void CPPTypeProxy::
output(std::ostream &out, int indent_level, CPPScope *scope, bool complete) const {
  if (_actual_type == nullptr) {
    out << "unknown";
    return;
  }
  _actual_type->output(out, indent_level, scope, complete);
}


/**
 *
 */
CPPDeclaration::SubType CPPTypeProxy::
get_subtype() const {
  return ST_type_proxy;
}

/**
 *
 */
CPPType *CPPTypeProxy::
as_type() {
  if (_actual_type == nullptr) {
    return this;
  }
  return _actual_type;
}

/**
 *
 */
CPPSimpleType *CPPTypeProxy::
as_simple_type() {
  if (_actual_type == nullptr) {
    return nullptr;
  }
  return _actual_type->as_simple_type();
}

/**
 *
 */
CPPPointerType *CPPTypeProxy::
as_pointer_type() {
  if (_actual_type == nullptr) {
    return nullptr;
  }
  return _actual_type->as_pointer_type();
}

/**
 *
 */
CPPReferenceType *CPPTypeProxy::
as_reference_type() {
  if (_actual_type == nullptr) {
    return nullptr;
  }
  return _actual_type->as_reference_type();
}

/**
 *
 */
CPPArrayType *CPPTypeProxy::
as_array_type() {
  if (_actual_type == nullptr) {
    return nullptr;
  }
  return _actual_type->as_array_type();
}

/**
 *
 */
CPPConstType *CPPTypeProxy::
as_const_type() {
  if (_actual_type == nullptr) {
    return nullptr;
  }
  return _actual_type->as_const_type();
}

/**
 *
 */
CPPFunctionType *CPPTypeProxy::
as_function_type() {
  if (_actual_type == nullptr) {
    return nullptr;
  }
  return _actual_type->as_function_type();
}

/**
 *
 */
CPPExtensionType *CPPTypeProxy::
as_extension_type() {
  if (_actual_type == nullptr) {
    return nullptr;
  }
  return _actual_type->as_extension_type();
}

/**
 *
 */
CPPStructType *CPPTypeProxy::
as_struct_type() {
  if (_actual_type == nullptr) {
    return nullptr;
  }
  return _actual_type->as_struct_type();
}

/**
 *
 */
CPPEnumType *CPPTypeProxy::
as_enum_type() {
  if (_actual_type == nullptr) {
    return nullptr;
  }
  return _actual_type->as_enum_type();
}

/**
 *
 */
CPPTBDType *CPPTypeProxy::
as_tbd_type() {
  if (_actual_type == nullptr) {
    return nullptr;
  }
  return _actual_type->as_tbd_type();
}

/**
 *
 */
CPPTypedefType *CPPTypeProxy::
as_typedef_type() {
  if (_actual_type == nullptr) {
    return nullptr;
  }
  return _actual_type->as_typedef_type();
}

/**
 *
 */
CPPTypeProxy *CPPTypeProxy::
as_type_proxy() {
  return this;
}
