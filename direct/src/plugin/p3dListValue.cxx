// Filename: p3dListValue.cxx
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

#include "p3dListValue.h"


////////////////////////////////////////////////////////////////////
//     Function: P3DListValue::Default Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DListValue::
P3DListValue() : P3DValue(P3D_VT_list) { 
}

////////////////////////////////////////////////////////////////////
//     Function: P3DListValue::Constructor
//       Access: Public
//  Description: Note that the ownership of the elements in the array
//               (but not the array itself) is transferred to the
//               list.
////////////////////////////////////////////////////////////////////
P3DListValue::
P3DListValue(P3DValue * const elements[], int num_elements) :
  P3DValue(P3D_VT_list)
{
  _elements.reserve(num_elements);
  for (int i = 0; i < num_elements; ++i) {
    _elements.push_back(elements[i]);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DListValue::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DListValue::
P3DListValue(const P3DListValue &copy) :
  P3DValue(copy)
{
  _elements.reserve(copy._elements.size());
  Elements::const_iterator ei;
  for (ei = copy._elements.begin(); ei != copy._elements.end(); ++ei) {
    _elements.push_back((*ei)->make_copy());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DListValue::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DListValue::
~P3DListValue() {
  Elements::iterator ei;
  for (ei = _elements.begin(); ei != _elements.end(); ++ei) {
    delete (*ei);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DListValue::make_copy
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
P3DValue *P3DListValue::
make_copy() {
  return new P3DListValue(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DListValue::get_bool
//       Access: Public, Virtual
//  Description: Returns the value value coerced to a boolean, if
//               possible.
////////////////////////////////////////////////////////////////////
bool P3DListValue::
get_bool() const {
  return !_elements.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DListValue::make_string
//       Access: Public, Virtual
//  Description: Fills the indicated C++ string object with the value
//               of this object coerced to a string.
////////////////////////////////////////////////////////////////////
void P3DListValue::
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
//     Function: P3DListValue::get_list_length
//       Access: Public, Virtual
//  Description: Returns the length of the value value as a list.
////////////////////////////////////////////////////////////////////
int P3DListValue::
get_list_length() const {
  return _elements.size();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DListValue::get_list_item
//       Access: Public, Virtual
//  Description: Returns the nth item in the value as a list.  The
//               return value is a freshly-allocated P3DValue object
//               that must be deleted by the caller, or NULL on error.
////////////////////////////////////////////////////////////////////
P3DValue *P3DListValue::
get_list_item(int n) const {
  if (n >= 0 && n < (int)_elements.size()) {
    return _elements[n]->make_copy();
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DListValue::append_item
//       Access: Public, Virtual
//  Description: Appends a new item to the end of the list.  Ownership
//               of the item is transferred to the list.
////////////////////////////////////////////////////////////////////
void P3DListValue::
append_item(P3DValue *item) {
  _elements.push_back(item);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DListValue::make_xml
//       Access: Public, Virtual
//  Description: Allocates and returns a new XML structure
//               corresponding to this value.
////////////////////////////////////////////////////////////////////
TiXmlElement *P3DListValue::
make_xml() const {
  TiXmlElement *xvalue = new TiXmlElement("value");
  xvalue->SetAttribute("type", "list");
  Elements::const_iterator ei;
  for (ei = _elements.begin(); ei != _elements.end(); ++ei) {
    TiXmlElement *xchild = (*ei)->make_xml();
    xvalue->LinkEndChild(xchild);
  }

  return xvalue;
}
