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
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPythonObject::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
P3DPythonObject::
~P3DPythonObject() {
  // TODO.
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPythonObject::get_type
//       Access: Public, Virtual
//  Description: Returns the fundamental type of this kind of object.
////////////////////////////////////////////////////////////////////
P3D_object_type P3DPythonObject::
get_type() const {
  return P3D_OT_object;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPythonObject::get_bool
//       Access: Public, Virtual
//  Description: Returns the object value coerced to a boolean, if
//               possible.
////////////////////////////////////////////////////////////////////
bool P3DPythonObject::
get_bool() const {
  bool bresult = 0;

  P3D_object *result = call("__bool__", NULL);
  if (result != NULL) {
    bresult = P3D_OBJECT_GET_BOOL(result);
    P3D_OBJECT_FINISH(result);
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
get_int() const {
  int iresult = 0;

  P3D_object *result = call("__int__", NULL);
  if (result != NULL) {
    iresult = P3D_OBJECT_GET_INT(result);
    P3D_OBJECT_FINISH(result);
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
get_float() const {
  double fresult = 0.0;

  P3D_object *result = call("__float__", NULL);
  if (result != NULL) {
    fresult = P3D_OBJECT_GET_FLOAT(result);
    P3D_OBJECT_FINISH(result);
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
make_string(string &value) const {
  P3D_object *result = call("__str__", NULL);
  if (result != NULL) {
    int size = P3D_OBJECT_GET_STRING(result, NULL, 0);
    char *buffer = new char[size];
    P3D_OBJECT_GET_STRING(result, buffer, size);
    value = string(buffer, size);
    delete[] buffer;

    P3D_OBJECT_FINISH(result);
  }    
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPythonObject::get_property
//       Access: Public, Virtual
//  Description: Returns the named property element in the object.  The
//               return value is a freshly-allocated P3DPythonObject object
//               that must be deleted by the caller, or NULL on error.
////////////////////////////////////////////////////////////////////
P3D_object *P3DPythonObject::
get_property(const string &property) const {
  P3DListObject *params = new P3DListObject;
  params->append(new P3DStringObject(property));

  P3D_object *result = call("__getattr__", params);
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

  P3DListObject *params = new P3DListObject;
  params->append(new P3DStringObject(property));

  P3D_object *result = NULL;

  if (value == NULL) {
    // Delete an attribute.
    result = call("__delattr__", params);

  } else {
    // Set a new attribute.
    params->append(value);

    result = call("__setattr__", params);
  }

  if (result != NULL) {
    bresult = P3D_OBJECT_GET_BOOL(result);
    P3D_OBJECT_FINISH(result);
  }

  return bresult;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPythonObject::get_list_length
//       Access: Public, Virtual
//  Description: Returns the length of the object as a list.
////////////////////////////////////////////////////////////////////
int P3DPythonObject::
get_list_length() const {
  // TODO.
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPythonObject::get_element
//       Access: Public, Virtual
//  Description: Returns the nth item in the value as a list.  The
//               return value is a freshly-allocated P3DPythonObject object
//               that must be deleted by the caller, or NULL on error.
////////////////////////////////////////////////////////////////////
P3D_object *P3DPythonObject::
get_element(int n) const {
  // TODO.
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPythonObject::set_element
//       Access: Public, Virtual
//  Description: Modifies (or deletes, if value is NULL) the nth item
//               in the value as a list.  Returns true on success,
//               false on failure.
////////////////////////////////////////////////////////////////////
bool P3DPythonObject::
set_element(int n, P3D_object *value) {
  // TODO.
  if (value != NULL) {
    P3D_OBJECT_FINISH(value);
  }
  return false;
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
call(const string &method_name, P3D_object *params) const {
  TiXmlDocument *doc = new TiXmlDocument;
  TiXmlDeclaration *decl = new TiXmlDeclaration("1.0", "utf-8", "");
  TiXmlElement *xcommand = new TiXmlElement("command");
  xcommand->SetAttribute("cmd", "pyobj");
  xcommand->SetAttribute("op", "call");
  xcommand->SetAttribute("object_id", _object_id);
  if (!method_name.empty()) {
    xcommand->SetAttribute("method_name", method_name);
  }

  if (params != NULL) {
    assert(params->_class == &P3DObject::_object_class);
    TiXmlElement *xparams = ((P3DObject *)params)->make_xml();
    xcommand->LinkEndChild(xparams);

    // Now we're done with the params object passed in, we can delete
    // it as promised.
    P3D_OBJECT_FINISH(params);
  }

  doc->LinkEndChild(decl);
  doc->LinkEndChild(xcommand);
  TiXmlDocument *response = _session->command_and_response(doc);
  nout << "call response pointer: " << response << "\n" << flush;

  P3DObject *result = NULL;
  if (response != NULL) {
    TiXmlElement *xresponse = response->FirstChildElement("response");
    if (xresponse != NULL) {
      TiXmlElement *xvalue = xresponse->FirstChildElement("value");
      if (xvalue != NULL) {
        result = _session->xml_to_object(xvalue);
      }
    }
    delete response;
  }

  nout << "call result = " << result << "\n" << flush;
  if (result != NULL) {
    nout << "  result = " << *result << "\n" << flush;
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
output(ostream &out) const {
  P3D_object *result = call("__repr__", NULL);
  out << "Python " << _object_id;
  if (result != NULL) {
    out << ": " << *result;
    P3D_OBJECT_FINISH(result);
  }    
}


////////////////////////////////////////////////////////////////////
//     Function: P3DPythonObject::make_xml
//       Access: Public, Virtual
//  Description: Allocates and returns a new XML structure
//               corresponding to this value.
////////////////////////////////////////////////////////////////////
TiXmlElement *P3DPythonObject::
make_xml() const {
  TiXmlElement *xvalue = new TiXmlElement("value");
  xvalue->SetAttribute("type", "python");
  xvalue->SetAttribute("object_id", _object_id);

  return xvalue;
}
