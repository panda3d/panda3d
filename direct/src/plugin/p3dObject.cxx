// Filename: p3dObject.cxx
// Created by:  drose (30Jun09)
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

#include "p3dObject.h"
#include <string.h>  // strncpy

// The following functions are C-style wrappers around the above
// P3DObject virtual methods; they are defined to allow us to create
// the C-style P3D_class_definition method table to store in the
// P3D_object structure.
static void
object_finish(P3D_object *object) {
  P3DObject::unref_delete(((P3DObject *)object));
}

static P3D_object *
object_copy(const P3D_object *object) {
  return ((const P3DObject *)object)->make_copy();
}

static P3D_object_type 
object_get_type(const P3D_object *object) {
  return ((const P3DObject *)object)->get_type();
}

static bool 
object_get_bool(const P3D_object *object) {
  return ((const P3DObject *)object)->get_bool();
}

static int
object_get_int(const P3D_object *object) {
  return ((const P3DObject *)object)->get_int();
}

static double 
object_get_float(const P3D_object *object) {
  return ((const P3DObject *)object)->get_float();
}

static int 
object_get_string(const P3D_object *object, char *buffer, int buffer_length) {
  return ((const P3DObject *)object)->get_string(buffer, buffer_length);
}

static int 
object_get_repr(const P3D_object *object, char *buffer, int buffer_length) {
  return ((const P3DObject *)object)->get_repr(buffer, buffer_length);
}

static P3D_object *
object_get_property(const P3D_object *object, const char *property) {
  return ((const P3DObject *)object)->get_property(property);
}

static bool
object_set_property(P3D_object *object, const char *property,
                    P3D_object *value) {
  return ((P3DObject *)object)->set_property(property, value);
}

static P3D_object *
object_get_element(const P3D_object *object, int n) {
  return ((const P3DObject *)object)->get_element(n);
}

static bool
object_set_element(P3D_object *object, int n, P3D_object *value) {
  return ((P3DObject *)object)->set_element(n, value);
}

static int object_get_list_length(const P3D_object *object) {
  return ((const P3DObject *)object)->get_list_length();
}

static P3D_object *
object_call(const P3D_object *object, const char *method_name,
            P3D_object *params) {
  if (method_name == NULL) {
    method_name = "";
  }
  return ((const P3DObject *)object)->call(method_name, params);
}

P3D_class_definition P3DObject::_object_class = {
  &object_finish,
  &object_copy,
  &object_get_type,
  &object_get_bool,
  &object_get_int,
  &object_get_float,
  &object_get_string,
  &object_get_repr,
  &object_get_property,
  &object_set_property,
  &object_get_element,
  &object_set_element,
  &object_get_list_length,
  &object_call,
};

////////////////////////////////////////////////////////////////////
//     Function: P3DObject::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
P3DObject::
~P3DObject() {
}

////////////////////////////////////////////////////////////////////
//     Function: P3DObject::make_copy
//       Access: Public, Virtual
//  Description: Returns a new copy of the object, if necessary.  If
//               the object type is static and all instances are
//               identical, this actually simply ups the reference
//               count and returns the same object.
////////////////////////////////////////////////////////////////////
P3DObject *P3DObject::
make_copy() const {
  ref();
  return (P3DObject *)this;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DObject::get_int
//       Access: Public, Virtual
//  Description: Returns the object value coerced to an integer, if
//               possible.
////////////////////////////////////////////////////////////////////
int P3DObject::
get_int() const {
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DObject::get_float
//       Access: Public, Virtual
//  Description: Returns the object value coerced to a floating-point
//               value, if possible.
////////////////////////////////////////////////////////////////////
double P3DObject::
get_float() const {
  return get_int();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DObject::get_string
//       Access: Public
//  Description: Stores a string that represents the object value in
//               the indicated buffer; a null character is included if
//               there is space.  Returns the number of characters
//               needed in the output (which might be more than the
//               actual number of characters stored if buffer_length
//               was too small).
////////////////////////////////////////////////////////////////////
int P3DObject::
get_string(char *buffer, int buffer_length) const {
  string result;
  make_string(result);
  strncpy(buffer, result.c_str(), buffer_length);
  return (int)result.size();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DObject::get_repr
//       Access: Public
//  Description: Returns a user-friendly representation of the object,
//               similar to get_string(), above.
////////////////////////////////////////////////////////////////////
int P3DObject::
get_repr(char *buffer, int buffer_length) const {
  ostringstream strm;
  output(strm);
  string result = strm.str();
  strncpy(buffer, result.c_str(), buffer_length);
  return (int)result.size();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DObject::get_property
//       Access: Public, Virtual
//  Description: Returns the named property element in the object.  The
//               return value is a freshly-allocated P3DObject object
//               that must be deleted by the caller, or NULL on error.
////////////////////////////////////////////////////////////////////
P3D_object *P3DObject::
get_property(const string &property) const {
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DObject::set_property
//       Access: Public, Virtual
//  Description: Modifies (or deletes, if value is NULL) the named
//               property element in the object.  Returns true on
//               success, false on failure.
////////////////////////////////////////////////////////////////////
bool P3DObject::
set_property(const string &property, P3D_object *value) {
  if (value != NULL) {
    P3D_OBJECT_FINISH(value);
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DObject::get_list_length
//       Access: Public, Virtual
//  Description: Returns the length of the object as a list.
////////////////////////////////////////////////////////////////////
int P3DObject::
get_list_length() const {
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DObject::get_element
//       Access: Public, Virtual
//  Description: Returns the nth item in the value as a list.  The
//               return value is a freshly-allocated P3DObject object
//               that must be deleted by the caller, or NULL on error.
////////////////////////////////////////////////////////////////////
P3D_object *P3DObject::
get_element(int n) const {
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DObject::set_element
//       Access: Public, Virtual
//  Description: Modifies (or deletes, if value is NULL) the nth item
//               in the value as a list.  Returns true on success,
//               false on failure.
////////////////////////////////////////////////////////////////////
bool P3DObject::
set_element(int n, P3D_object *value) {
  if (value != NULL) {
    P3D_OBJECT_FINISH(value);
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DObject::call
//       Access: Public, Virtual
//  Description: Invokes the named method on the object, passing the
//               indicated parameters.  If the method name is empty,
//               invokes the object itself.  Returns the return value
//               on success, NULL on error.
////////////////////////////////////////////////////////////////////
P3D_object *P3DObject::
call(const string &method_name, P3D_object *params) const {
  if (params != NULL) {
    P3D_OBJECT_FINISH(params);
  }
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DObject::output
//       Access: Public, Virtual
//  Description: Writes a formatted representation of the value to the
//               indicated string.  This is intended for developer
//               assistance.
////////////////////////////////////////////////////////////////////
void P3DObject::
output(ostream &out) const {
  string value;
  make_string(value);
  out << value;
}

