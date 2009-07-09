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

// The following functions are C-style wrappers around the below
// P3DObject virtual methods; they are defined to allow us to create
// the C-style P3D_class_definition method table to store in the
// P3D_object structure.
static void
object_finish(P3D_object *object) {
  delete (P3DObject *)object;
}

static P3D_object_type 
object_get_type(P3D_object *object) {
  return ((P3DObject *)object)->get_type();
}

static bool 
object_get_bool(P3D_object *object) {
  return ((P3DObject *)object)->get_bool();
}

static int
object_get_int(P3D_object *object) {
  return ((P3DObject *)object)->get_int();
}

static double 
object_get_float(P3D_object *object) {
  return ((P3DObject *)object)->get_float();
}

static int 
object_get_string(P3D_object *object, char *buffer, int buffer_length) {
  return ((P3DObject *)object)->get_string(buffer, buffer_length);
}

static int 
object_get_repr(P3D_object *object, char *buffer, int buffer_length) {
  return ((P3DObject *)object)->get_repr(buffer, buffer_length);
}

static P3D_object *
object_get_property(P3D_object *object, const char *property) {
  return ((P3DObject *)object)->get_property(property);
}

static bool
object_set_property(P3D_object *object, const char *property,
                    P3D_object *value) {
  return ((P3DObject *)object)->set_property(property, value);
}

static bool
object_has_method(P3D_object *object, const char *method_name) {
  return ((P3DObject *)object)->has_method(method_name);
}

static P3D_object *
object_call(P3D_object *object, const char *method_name,
            P3D_object *params[], int num_params) {
  if (method_name == NULL) {
    method_name = "";
  }
  return ((P3DObject *)object)->call(method_name, params, num_params);
}

static P3D_object *
object_eval(P3D_object *object, const char *expression) {
  return ((P3DObject *)object)->eval(expression);
}

P3D_class_definition P3DObject::_object_class = {
  &object_finish,
  &object_get_type,
  &object_get_bool,
  &object_get_int,
  &object_get_float,
  &object_get_string,
  &object_get_repr,
  &object_get_property,
  &object_set_property,
  &object_has_method,
  &object_call,
  &object_eval,
};

// The next functions are used to construct the generic
// P3D_class_definition class returned by P3D_make_class_definition().
// These are pointers to no-op functions, which the host may or may
// not choose to override.
static void
generic_finish(P3D_object *object) {
  // You must override finish(), though, otherwise it's a leak.  The
  // core API has no idea how to delete your object.
  nout << "Warning!  default object_finish() method does nothing; object will leak.\n" << flush;
}

static P3D_object_type 
generic_get_type(P3D_object *object) {
  // We assume anyone going through the trouble of subclassing this
  // will want to return an object, not one of the other fundamental
  // types.
  return P3D_OT_object;
}

static bool 
generic_get_bool(P3D_object *object) {
  return false;
}

static int
generic_get_int(P3D_object *object) {
  return 0;
}

static double 
generic_get_float(P3D_object *object) {
  return 0.0;
}

static int 
generic_get_string(P3D_object *object, char *buffer, int buffer_length) {
  return 0;
}

static int 
generic_get_repr(P3D_object *object, char *buffer, int buffer_length) {
  return 0;
}

static P3D_object *
generic_get_property(P3D_object *object, const char *property) {
  return NULL;
}

static bool
generic_set_property(P3D_object *object, const char *property,
                     P3D_object *value) {
  return false;
}

static bool
generic_has_method(P3D_object *object, const char *method_name) {
  return false;
}

static P3D_object *
generic_call(P3D_object *object, const char *method_name,
            P3D_object *params[], int num_params) {
  return NULL;
}

static P3D_object *
generic_eval(P3D_object *object, const char *expression) {
  return NULL;
}

P3D_class_definition P3DObject::_generic_class = {
  &generic_finish,
  &generic_get_type,
  &generic_get_bool,
  &generic_get_int,
  &generic_get_float,
  &generic_get_string,
  &generic_get_repr,
  &generic_get_property,
  &generic_set_property,
  &generic_has_method,
  &generic_call,
  &generic_eval,
};

////////////////////////////////////////////////////////////////////
//     Function: P3DObject::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
P3DObject::
~P3DObject() {
  assert(_ref_count == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DObject::get_int
//       Access: Public, Virtual
//  Description: Returns the object value coerced to an integer, if
//               possible.
////////////////////////////////////////////////////////////////////
int P3DObject::
get_int() {
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DObject::get_float
//       Access: Public, Virtual
//  Description: Returns the object value coerced to a floating-point
//               value, if possible.
////////////////////////////////////////////////////////////////////
double P3DObject::
get_float() {
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
get_string(char *buffer, int buffer_length) {
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
get_repr(char *buffer, int buffer_length) {
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
get_property(const string &property) {
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
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DObject::has_method
//       Access: Public, Virtual
//  Description: Returns true if the named method exists on this
//               object, false otherwise.
////////////////////////////////////////////////////////////////////
bool P3DObject::
has_method(const string &method_name) {
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
call(const string &method_name, P3D_object *params[], int num_params) {
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DObject::eval
//       Access: Public, Virtual
//  Description: Evaluates an arbitrary JavaScript expression.  None
//               of the P3DObject classes implement this.
////////////////////////////////////////////////////////////////////
P3D_object *P3DObject::
eval(const string &expression) {
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
output(ostream &out) {
  string value;
  make_string(value);
  out << value;
}

