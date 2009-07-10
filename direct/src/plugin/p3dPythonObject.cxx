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
  nout << "new P3DPythonObject " << this << " : " << object_id << "\n" << flush;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPythonObject::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
P3DPythonObject::
~P3DPythonObject() {
  nout << "del P3DPythonObject " << this << " : " << _object_id << "\n";
  // When the P3DPythonObject wrapper goes away, we have to inform the
  // child process that we no longer need the corresponding PyObject
  // to be kept around.
  _session->drop_pyobj(_object_id);
  unref_delete(_session);
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

  P3D_object *result = call("__bool__", NULL, 0);
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

  P3D_object *result = call("__int__", NULL, 0);
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

  P3D_object *result = call("__float__", NULL, 0);
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
  P3D_object *result = call("__str__", NULL, 0);
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

  P3D_object *result = call("__getattr__", params, 1);
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
set_property(const string &property, P3D_object *value) {
  bool bresult = false;

  P3D_object *params[2];
  params[0] = new P3DStringObject(property);

  P3D_object *result = NULL;

  if (value == NULL) {
    // Delete an attribute.
    result = call("__delattr__", params, 1);

  } else {
    // Set a new attribute.
    params[1] = value;
    result = call("__setattr__", params, 2);
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
  bool bresult = false;

  P3D_object *params[1];
  params[0] = new P3DStringObject(method_name);

  P3D_object *result = call("__has_method__", params, 1);
  P3D_OBJECT_DECREF(params[0]);

  if (result != NULL) {
    bresult = P3D_OBJECT_GET_BOOL(result);
    P3D_OBJECT_DECREF(result);
  }

  return bresult;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPythonObject::call
//       Access: Public, Virtual
//  Description: Invokes the named method on the object, passing the
//               indicated parameters.  If the method name is empty,
//               invokes the object itself.  Returns the return value
//               on success, NULL on error.
////////////////////////////////////////////////////////////////////
P3D_object *P3DPythonObject::
call(const string &method_name, P3D_object *params[], int num_params) {
  TiXmlDocument *doc = new TiXmlDocument;
  TiXmlDeclaration *decl = new TiXmlDeclaration("1.0", "utf-8", "");
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

  doc->LinkEndChild(decl);
  doc->LinkEndChild(xcommand);
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
  P3D_object *result = call("__repr__", NULL, 0);
  out << "Python " << _object_id;
  if (result != NULL) {
    out << ": " << *result;
    P3D_OBJECT_DECREF(result);
  }    
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

