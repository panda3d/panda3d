// Filename: p3dPythonObject.cxx
// Created by:  drose (03Jul09)
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

#include "p3dPythonObject.h"

////////////////////////////////////////////////////////////////////
//     Function: P3DPythonObject::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DPythonObject::
P3DPythonObject(P3DSession *session, int object_id) :
  _session(session),
  _object_id(object_id)
{
  _session->ref();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPythonObject::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
P3DPythonObject::
~P3DPythonObject() {
  // When the P3DPythonObject wrapper goes away, we have to inform the
  // child process that we no longer need the corresponding PyObject
  // to be kept around.
  _session->drop_pyobj(_object_id);
  p3d_unref_delete(_session);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPythonObject::get_type
//       Access: Public, Virtual
//  Description: Returns the fundamental type of this kind of object.
////////////////////////////////////////////////////////////////////
P3D_object_type P3DPythonObject::
get_type() {
  return P3D_OT_object;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPythonObject::get_bool
//       Access: Public, Virtual
//  Description: Returns the object value coerced to a boolean, if
//               possible.
////////////////////////////////////////////////////////////////////
bool P3DPythonObject::
get_bool() {
  bool bresult = 0;

  P3D_object *result = call("__bool__", true, NULL, 0);
  if (result != NULL) {
    bresult = P3D_OBJECT_GET_BOOL(result);
    P3D_OBJECT_DECREF(result);
  }    

  return bresult;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPythonObject::get_int
//       Access: Public, Virtual
//  Description: Returns the object value coerced to an integer, if
//               possible.
////////////////////////////////////////////////////////////////////
int P3DPythonObject::
get_int() {
  int iresult = 0;

  P3D_object *result = call("__int__", true, NULL, 0);
  if (result != NULL) {
    iresult = P3D_OBJECT_GET_INT(result);
    P3D_OBJECT_DECREF(result);
  }    

  return iresult;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPythonObject::get_float
//       Access: Public, Virtual
//  Description: Returns the object value coerced to a floating-point
//               value, if possible.
////////////////////////////////////////////////////////////////////
double P3DPythonObject::
get_float() {
  double fresult = 0.0;

  P3D_object *result = call("__float__", true, NULL, 0);
  if (result != NULL) {
    fresult = P3D_OBJECT_GET_FLOAT(result);
    P3D_OBJECT_DECREF(result);
  }    

  return fresult;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPythonObject::make_string
//       Access: Public, Virtual
//  Description: Fills the indicated C++ string object with the value
//               of this object coerced to a string.
////////////////////////////////////////////////////////////////////
void P3DPythonObject::
make_string(string &value) {
  P3D_object *result = call("__str__", true, NULL, 0);
  if (result != NULL) {
    int size = P3D_OBJECT_GET_STRING(result, NULL, 0);
    char *buffer = new char[size];
    P3D_OBJECT_GET_STRING(result, buffer, size);
    value = string(buffer, size);
    delete[] buffer;

    P3D_OBJECT_DECREF(result);
  }    
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPythonObject::get_property
//       Access: Public, Virtual
//  Description: Returns the named property element in the object.  The
//               return value is a new-reference P3D_object, or NULL
//               on error.
////////////////////////////////////////////////////////////////////
P3D_object *P3DPythonObject::
get_property(const string &property) {
  P3D_object *params[1];
  params[0] = new P3DStringObject(property);

  P3D_object *result = call("__get_property__", true, params, 1);
  P3D_OBJECT_DECREF(params[0]);
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPythonObject::set_property
//       Access: Public, Virtual
//  Description: Modifies (or deletes, if value is NULL) the named
//               property element in the object.  Returns true on
//               success, false on failure.
////////////////////////////////////////////////////////////////////
bool P3DPythonObject::
set_property(const string &property, bool needs_response, P3D_object *value) {
  if (!_session->get_matches_script_origin()) {
    // If you can't be scripting us, you can't be setting properties either.
    return false;
  }

  return set_property_insecure(property, needs_response, value);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPythonObject::set_property_insecure
//       Access: Public
//  Description: Works as set_property(), but does not check the
//               matches_script_origin flag.  Intended to be called
//               internally only, never to be called from Javascript.
////////////////////////////////////////////////////////////////////
bool P3DPythonObject::
set_property_insecure(const string &property, bool needs_response, 
                      P3D_object *value) {
  bool bresult = !needs_response;

  P3D_object *params[2];
  params[0] = new P3DStringObject(property);

  P3D_object *result = NULL;

  if (value == NULL) {
    // Delete an attribute.
    result = call_insecure("__del_property__", needs_response, params, 1);

  } else {
    // Set a new attribute.
    params[1] = value;
    result = call_insecure("__set_property__", needs_response, params, 2);
  }

  P3D_OBJECT_DECREF(params[0]);

  if (result != NULL) {
    bresult = P3D_OBJECT_GET_BOOL(result);
    P3D_OBJECT_DECREF(result);
  }

  return bresult;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPythonObject::has_method
//       Access: Public, Virtual
//  Description: Returns true if the named method exists on this
//               object, false otherwise.
////////////////////////////////////////////////////////////////////
bool P3DPythonObject::
has_method(const string &method_name) {
  // First, check the cache.
  pair<HasMethod::iterator, bool> cresult = _has_method.insert(HasMethod::value_type(method_name, false));
  HasMethod::iterator hi = cresult.first;
  if (!cresult.second) {
    // Already cached.
    return (*hi).second;
  }

  bool bresult = false;

  P3D_object *params[1];
  params[0] = new P3DStringObject(method_name);

  P3D_object *result = call("__has_method__", true, params, 1);
  P3D_OBJECT_DECREF(params[0]);

  if (result != NULL) {
    bresult = P3D_OBJECT_GET_BOOL(result);
    P3D_OBJECT_DECREF(result);
  }

  // Save the cached result, so we don't have to keep asking this
  // question.  We assume that the set of methods on an object don't
  // change substantially, so we can get away with keeping this cache.
  (*hi).second = bresult;

  return bresult;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPythonObject::call
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
P3D_object *P3DPythonObject::
call(const string &method_name, bool needs_response,
     P3D_object *params[], int num_params) {
  if (!_session->get_matches_script_origin()) {
    // If you can't be scripting us, you can't be calling methods.
    return NULL;
  }

  return call_insecure(method_name, needs_response, params, num_params);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPythonObject::call_insecure
//       Access: Public
//  Description: Works as call(), but does not check the
//               matches_script_origin flag.  Intended to be called
//               internally only, never to be called from Javascript.
////////////////////////////////////////////////////////////////////
P3D_object *P3DPythonObject::
call_insecure(const string &method_name, bool needs_response,
              P3D_object *params[], int num_params) {
  TiXmlDocument *doc = new TiXmlDocument;
  TiXmlElement *xcommand = new TiXmlElement("command");
  xcommand->SetAttribute("cmd", "pyobj");
  xcommand->SetAttribute("op", "call");
  if (!method_name.empty()) {
    xcommand->SetAttribute("method_name", method_name);
  }

  TiXmlElement *xobject = _session->p3dobj_to_xml(this);
  xobject->SetValue("object");
  xcommand->LinkEndChild(xobject);

  for (int i = 0; i < num_params; ++i) {
    TiXmlElement *xparams = _session->p3dobj_to_xml(params[i]);
    xcommand->LinkEndChild(xparams);
  }

  doc->LinkEndChild(xcommand);

  // If no response is requested, send the command out in a vacuum,
  // and return NULL.
  if (!needs_response) {
    _session->send_command(doc);
    return NULL;
  }

  // If a response is requested, we have to send the command and wait
  // for it.
  TiXmlDocument *response = _session->command_and_response(doc);

  P3D_object *result = NULL;
  if (response != NULL) {
    TiXmlElement *xresponse = response->FirstChildElement("response");
    if (xresponse != NULL) {
      TiXmlElement *xvalue = xresponse->FirstChildElement("value");
      if (xvalue != NULL) {
        result = _session->xml_to_p3dobj(xvalue);
      }
    }
    delete response;
  }

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPythonObject::output
//       Access: Public, Virtual
//  Description: Writes a formatted representation of the value to the
//               indicated string.  This is intended for developer
//               assistance.
////////////////////////////////////////////////////////////////////
void P3DPythonObject::
output(ostream &out) {
  P3D_object *result = call("__repr__", true, NULL, 0);
  out << "Python " << _object_id;
  if (result != NULL) {
    out << ": " << *result;
    P3D_OBJECT_DECREF(result);
  }    
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPythonObject::fill_xml
//       Access: Public, Virtual
//  Description: If this object has a valid XML representation for the
//               indicated session (that hasn't already been
//               implemented by the generic code in P3DSession), this
//               method will apply it to the indicated "value" element
//               and return true.  Otherwise, this method will leave
//               the element unchanged and return false.
////////////////////////////////////////////////////////////////////
bool P3DPythonObject::
fill_xml(TiXmlElement *xvalue, P3DSession *session) {
  if (session == _session) {
    // If it's a P3DPythonObject from the same session, just send
    // the object_id down, since the actual implementation of this
    // object exists (as a Python object) in the sub-process space.
    xvalue->SetAttribute("type", "python");
    xvalue->SetAttribute("object_id", _object_id);
    return true;
  }

  // Otherwise, we have to send it as a browser object.
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPythonObject::as_python_object
//       Access: Public, Virtual
//  Description: Returns this object, downcast to a P3DPythonObject,
//               if it is in fact an object of that type; or NULL if
//               it is not.
////////////////////////////////////////////////////////////////////
P3DPythonObject *P3DPythonObject::
as_python_object() {
  return this;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPythonObject::get_session
//       Access: Public
//  Description: Returns the session that this object is identified
//               with.
////////////////////////////////////////////////////////////////////
P3DSession *P3DPythonObject::
get_session() {
  return _session;
}


////////////////////////////////////////////////////////////////////
//     Function: P3DPythonObject::get_object_id
//       Access: Public
//  Description: Returns the object_id number that is used to uniquely
//               identify this object in the XML stream.
////////////////////////////////////////////////////////////////////
int P3DPythonObject::
get_object_id() {
  return _object_id;
}

