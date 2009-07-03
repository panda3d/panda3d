// Filename: p3dListObject.cxx
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

#include "p3dListObject.h"


////////////////////////////////////////////////////////////////////
//     Function: P3DListObject::Default Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DListObject::
P3DListObject() { 
}

////////////////////////////////////////////////////////////////////
//     Function: P3DListObject::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DListObject::
P3DListObject(const P3DListObject &copy) :
  P3DObject(copy)
{
  _elements.reserve(copy._elements.size());
  Elements::const_iterator ei;
  for (ei = copy._elements.begin(); ei != copy._elements.end(); ++ei) {
    _elements.push_back(P3D_OBJECT_COPY(*ei));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DListObject::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DListObject::
~P3DListObject() {
  Elements::iterator ei;
  for (ei = _elements.begin(); ei != _elements.end(); ++ei) {
    P3D_OBJECT_FINISH(*ei);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DListObject::make_copy
//       Access: Public, Virtual
//  Description: Returns a new copy of the object, if necessary.  If
//               the object type is static and all instances are
//               identical, this actually simply ups the reference
//               count and returns the same object.
////////////////////////////////////////////////////////////////////
P3DObject *P3DListObject::
make_copy() const {
  return new P3DListObject(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DListObject::get_type
//       Access: Public, Virtual
//  Description: Returns the fundamental type of this kind of object.
////////////////////////////////////////////////////////////////////
P3D_object_type P3DListObject::
get_type() const {
  return P3D_OT_list;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DListObject::get_bool
//       Access: Public, Virtual
//  Description: Returns the object value coerced to a boolean, if
//               possible.
////////////////////////////////////////////////////////////////////
bool P3DListObject::
get_bool() const {
  return !_elements.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DListObject::make_string
//       Access: Public, Virtual
//  Description: Fills the indicated C++ string object with the value
//               of this object coerced to a string.
////////////////////////////////////////////////////////////////////
void P3DListObject::
make_string(string &value) const {
  ostringstream strm;
  strm << "[";
  if (!_elements.empty()) {
    strm << *_elements[0];
    for (size_t i = 1; i < _elements.size(); ++i) {
      strm << ", " << *_elements[i];
    }
  }
  strm << "]";

  value = strm.str();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DListObject::get_list_length
//       Access: Public, Virtual
//  Description: Returns the length of the object as a list.
////////////////////////////////////////////////////////////////////
int P3DListObject::
get_list_length() const {
  return _elements.size();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DListObject::get_element
//       Access: Public, Virtual
//  Description: Returns the nth item in the value as a list.  The
//               return value is a freshly-allocated P3DObject object
//               that must be deleted by the caller, or NULL on error.
////////////////////////////////////////////////////////////////////
P3D_object *P3DListObject::
get_element(int n) const {
  if (n >= 0 && n < (int)_elements.size()) {
    return P3D_OBJECT_COPY(_elements[n]);
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DListObject::set_element
//       Access: Public, Virtual
//  Description: Modifies (or deletes, if value is NULL) the nth item
//               in the value as a list.  Returns true on success,
//               false on failure.
////////////////////////////////////////////////////////////////////
bool P3DListObject::
set_element(int n, P3D_object *value) {
  if (n < 0 || n > (int)_elements.size()) {
    // Invalid index.
    return false;
  }

  if (n == _elements.size()) {
    // Append one.
    _elements.push_back(NULL);
  }
  if (_elements[n] != NULL) {
    // Delete prior.
    P3D_OBJECT_FINISH(_elements[n]);
  }
  _elements[n] = value;

  // Delete NULL elements on the end.
  while (!_elements.empty() && _elements.back() == NULL) {
    _elements.pop_back();
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DListObject::append
//       Access: Public, Virtual
//  Description: Adds a new element to the end of the list.  Ownership
//               is transferred to the list.
////////////////////////////////////////////////////////////////////
void P3DListObject::
append(P3D_object *value) {
  _elements.push_back(value);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DListObject::make_xml
//       Access: Public, Virtual
//  Description: Allocates and returns a new XML structure
//               corresponding to this value.
////////////////////////////////////////////////////////////////////
TiXmlElement *P3DListObject::
make_xml() const {
  TiXmlElement *xvalue = new TiXmlElement("value");
  xvalue->SetAttribute("type", "list");
  Elements::const_iterator ei;
  for (ei = _elements.begin(); ei != _elements.end(); ++ei) {
    P3D_object *child = (*ei);
    assert(child->_class == &P3DObject::_object_class);
    TiXmlElement *xchild = ((P3DObject *)child)->make_xml();
    xvalue->LinkEndChild(xchild);
  }

  return xvalue;
}
