// Filename: ppBrowserObject.cxx
// Created by:  drose (05Jul09)
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

#include "ppBrowserObject.h"
#include "ppInstance.h"
#include <sstream>
#include <string.h>  // strncpy

// The following functions are C-style wrappers around the above
// PPBrowserObject methods; they are defined to allow us to create the
// C-style P3D_class_definition method table to store in the
// P3D_object structure.
static void
object_finish(P3D_object *object) {
  delete ((PPBrowserObject *)object);
}

static P3D_object *
object_copy(const P3D_object *object) {
  return new PPBrowserObject(*(const PPBrowserObject *)object);
}

static int 
object_get_repr(const P3D_object *object, char *buffer, int buffer_length) {
  return ((const PPBrowserObject *)object)->get_repr(buffer, buffer_length);
}

static P3D_object *
object_get_property(const P3D_object *object, const char *property) {
  return ((const PPBrowserObject *)object)->get_property(property);
}

static bool
object_set_property(P3D_object *object, const char *property,
                    P3D_object *value) {
  return ((PPBrowserObject *)object)->set_property(property, value);
}

static P3D_object *
object_call(const P3D_object *object, const char *method_name,
            P3D_object *params[], int num_params) {
  if (method_name == NULL) {
    method_name = "";
  }
  return ((const PPBrowserObject *)object)->call(method_name, params, num_params);
}

P3D_class_definition *PPBrowserObject::_browser_object_class;

////////////////////////////////////////////////////////////////////
//     Function: PPBrowserObject::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PPBrowserObject::
PPBrowserObject(PPInstance *inst, NPObject *npobj) :
  _instance(inst),
  _npobj(npobj)
{
  _class = get_class_definition();
  browser->retainobject(_npobj);
}

////////////////////////////////////////////////////////////////////
//     Function: PPBrowserObject::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PPBrowserObject::
PPBrowserObject(const PPBrowserObject &copy) :
  _instance(copy._instance),
  _npobj(copy._npobj)
{
  _class = get_class_definition();
  browser->retainobject(_npobj);
}

////////////////////////////////////////////////////////////////////
//     Function: PPBrowserObject::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PPBrowserObject::
~PPBrowserObject() {
  browser->releaseobject(_npobj);
}

////////////////////////////////////////////////////////////////////
//     Function: PPBrowserObject::get_repr
//       Access: Public
//  Description: Returns a user-friendly representation of the object,
//               similar to get_string(), above.
////////////////////////////////////////////////////////////////////
int PPBrowserObject::
get_repr(char *buffer, int buffer_length) const {
  ostringstream strm;
  strm << "NPObject " << _npobj;
  string result = strm.str();
  strncpy(buffer, result.c_str(), buffer_length);
  return (int)result.size();
}

////////////////////////////////////////////////////////////////////
//     Function: PPBrowserObject::get_property
//       Access: Public
//  Description: Returns the named property element in the object.  The
//               return value is a freshly-allocated PPBrowserObject object
//               that must be deleted by the caller, or NULL on error.
////////////////////////////////////////////////////////////////////
P3D_object *PPBrowserObject::
get_property(const string &property) const {
  NPIdentifier property_name = browser->getstringidentifier(property.c_str());
  NPVariant result;
  if (!browser->getproperty(_instance->get_npp_instance(), _npobj,
                            property_name, &result)) {
    // Failed to retrieve property.
    return NULL;
  }

  P3D_object *object = _instance->variant_to_p3dobj(&result);
  browser->releasevariantvalue(&result);
  return object;
}

////////////////////////////////////////////////////////////////////
//     Function: PPBrowserObject::set_property
//       Access: Public
//  Description: Modifies (or deletes, if value is NULL) the named
//               property element in the object.  Returns true on
//               success, false on failure.
////////////////////////////////////////////////////////////////////
bool PPBrowserObject::
set_property(const string &property, P3D_object *value) {
  if (value != NULL) {
    P3D_OBJECT_FINISH(value);
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PPBrowserObject::call
//       Access: Public
//  Description: Invokes the named method on the object, passing the
//               indicated parameters.  If the method name is empty,
//               invokes the object itself.  Returns the return value
//               on success, NULL on error.
////////////////////////////////////////////////////////////////////
P3D_object *PPBrowserObject::
call(const string &method_name, P3D_object *params[], int num_params) const {
  for (int i = 0; i < num_params; ++i) {
    P3D_OBJECT_FINISH(params[i]);
  }
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: PPBrowserObject::get_class_definition
//       Access: Private, Static
//  Description: Returns a pointer to the P3D_class_definition object
//               that lists all of the C-style method pointers for
//               this class object.
////////////////////////////////////////////////////////////////////
P3D_class_definition *PPBrowserObject::
get_class_definition() {
  if (_browser_object_class == NULL) {
    // Create a default class_definition object, and fill in the
    // appropriate pointers.
    _browser_object_class = P3D_make_class_definition();
    _browser_object_class->_finish = &object_finish;
    _browser_object_class->_copy = &object_copy;

    _browser_object_class->_get_repr = &object_get_repr;
    _browser_object_class->_get_property = &object_get_property;
    _browser_object_class->_set_property = &object_set_property;
    _browser_object_class->_call = &object_call;
  }

  return _browser_object_class;
}
