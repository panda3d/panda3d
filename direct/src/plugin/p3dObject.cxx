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
#include "p3dBoolObject.h"
#include "p3dIntObject.h"
#include "p3dFloatObject.h"
#include "p3dStringObject.h"
#include "p3dInstanceManager.h"
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
                    bool needs_response, P3D_object *value) {
  return ((P3DObject *)object)->set_property(property, needs_response, value);
}

static bool
object_has_method(P3D_object *object, const char *method_name) {
  return ((P3DObject *)object)->has_method(method_name);
}

static P3D_object *
object_call(P3D_object *object, const char *method_name,
            bool needs_response,
            P3D_object *params[], int num_params) {
  if (method_name == NULL) {
    method_name = "";
  }
  return ((P3DObject *)object)->call(method_name, needs_response, params, num_params);
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
  nout << "Warning!  default object_finish() method does nothing; object will leak.\n";
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
                     bool needs_response, P3D_object *value) {
  return false;
}

static bool
generic_has_method(P3D_object *object, const char *method_name) {
  return false;
}

static P3D_object *
generic_call(P3D_object *object, const char *method_name,
             bool needs_response, P3D_object *params[], int num_params) {
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
//               return value is a new-reference P3D_object, or NULL
//               on error.
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
set_property(const string &property, bool needs_response, P3D_object *value) {
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
//               invokes the object itself.
//
//               If needs_response is true, the return value is a
//               new-reference P3D_object on success, or NULL on
//               failure.  If needs_response is false, the return
//               value is always NULL, and there is no way to
//               determine success or failure.
////////////////////////////////////////////////////////////////////
P3D_object *P3DObject::
call(const string &method_name, bool needs_response,
     P3D_object *params[], int num_params) {
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

////////////////////////////////////////////////////////////////////
//     Function: P3DObject::fill_xml
//       Access: Public, Virtual
//  Description: If this object has a valid XML representation for the
//               indicated session (that hasn't already been
//               implemented by the generic code in P3DSession), this
//               method will apply it to the indicated "value" element
//               and return true.  Otherwise, this method will leave
//               the element unchanged and return false.
////////////////////////////////////////////////////////////////////
bool P3DObject::
fill_xml(TiXmlElement *xvalue, P3DSession *session) {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DObject::get_object_array
//       Access: Public, Virtual
//  Description: Returns a pointer to the array of objects represented
//               by this object, if any, or NULL if the object does
//               not represent an array of objects.  This may also
//               return NULL for a zero-length array; use
//               get_object_array_size() to differentiate.
////////////////////////////////////////////////////////////////////
P3D_object **P3DObject::
get_object_array() {
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DObject::get_object_array_size
//       Access: Public, Virtual
//  Description: Returns the number of elements in the array returned
//               by get_object_array(), or -1 if this object does not
//               representan array of objects.
////////////////////////////////////////////////////////////////////
int P3DObject::
get_object_array_size() {
  return -1;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DObject::as_python_object
//       Access: Public, Virtual
//  Description: Returns this object, downcast to a P3DPythonObject,
//               if it is in fact an object of that type; or NULL if
//               it is not.
////////////////////////////////////////////////////////////////////
P3DPythonObject *P3DObject::
as_python_object() {
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DObject::get_bool_property
//       Access: Public
//  Description: Returns the value of the named property, as a
//               boolean.  Returns 0 if the property does not exist.
////////////////////////////////////////////////////////////////////
bool P3DObject::
get_bool_property(const string &property) {
  P3D_object *result = get_property(property);
  if (result == NULL) {
    return 0;
  }
  bool bresult = P3D_OBJECT_GET_BOOL(result);
  P3D_OBJECT_DECREF(result);
  return bresult;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DObject::set_bool_property
//       Access: Public
//  Description: Changes the value of the named property to the
//               indicated boolean value.
////////////////////////////////////////////////////////////////////
void P3DObject::
set_bool_property(const string &property, bool value) {
  P3D_object *bvalue = new P3DBoolObject(value);
  set_property(property, false, bvalue);
  P3D_OBJECT_DECREF(bvalue);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DObject::get_int_property
//       Access: Public
//  Description: Returns the value of the named property, as an
//               integer.  Returns 0 if the property does not exist.
////////////////////////////////////////////////////////////////////
int P3DObject::
get_int_property(const string &property) {
  P3D_object *result = get_property(property);
  if (result == NULL) {
    return 0;
  }
  int iresult = P3D_OBJECT_GET_INT(result);
  P3D_OBJECT_DECREF(result);
  return iresult;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DObject::set_int_property
//       Access: Public
//  Description: Changes the value of the named property to the
//               indicated integer value.
////////////////////////////////////////////////////////////////////
void P3DObject::
set_int_property(const string &property, int value) {
  P3D_object *ivalue = new P3DIntObject(value);
  set_property(property, false, ivalue);
  P3D_OBJECT_DECREF(ivalue);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DObject::get_float_property
//       Access: Public
//  Description: Returns the value of the named property, as a
//               floating-point number.  Returns 0.0 if the property
//               does not exist.
////////////////////////////////////////////////////////////////////
double P3DObject::
get_float_property(const string &property) {
  P3D_object *result = get_property(property);
  if (result == NULL) {
    return 0.0;
  }
  double fresult = P3D_OBJECT_GET_FLOAT(result);
  P3D_OBJECT_DECREF(result);
  return fresult;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DObject::set_float_property
//       Access: Public
//  Description: Changes the value of the named property to the
//               indicated floating-point value.
////////////////////////////////////////////////////////////////////
void P3DObject::
set_float_property(const string &property, double value) {
  P3D_object *fvalue = new P3DFloatObject(value);
  set_property(property, false, fvalue);
  P3D_OBJECT_DECREF(fvalue);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DObject::get_string_property
//       Access: Public
//  Description: Returns the value of the named property, as a
//               string.  Returns empty string if the property does
//               not exist.
////////////////////////////////////////////////////////////////////
string P3DObject::
get_string_property(const string &property) {
  P3D_object *result = get_property(property);
  if (result == NULL) {
    return string();
  }

  int size = P3D_OBJECT_GET_STRING(result, NULL, 0);
  char *buffer = new char[size];
  P3D_OBJECT_GET_STRING(result, buffer, size);
  string sresult(buffer, size);
  delete[] buffer;

  P3D_OBJECT_DECREF(result);
  return sresult;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DObject::set_string_property
//       Access: Public
//  Description: Changes the value of the named property to the
//               indicated string value.
////////////////////////////////////////////////////////////////////
void P3DObject::
set_string_property(const string &property, const string &value) {
  P3D_object *svalue = new P3DStringObject(value);
  set_property(property, false, svalue);
  P3D_OBJECT_DECREF(svalue);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DObject::set_undefined_property
//       Access: Public
//  Description: Changes the value of the named property to the
//               undefined value.
////////////////////////////////////////////////////////////////////
void P3DObject::
set_undefined_property(const string &property) {
  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  P3D_object *uvalue = inst_mgr->new_undefined_object();
  set_property(property, false, uvalue);
  P3D_OBJECT_DECREF(uvalue);
}
