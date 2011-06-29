// Filename: p3dConcreteSequence.cxx
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

#include "p3dConcreteSequence.h"
#include "p3dSession.h"

////////////////////////////////////////////////////////////////////
//     Function: P3DConcreteSequence::Default Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DConcreteSequence::
P3DConcreteSequence() { 
}

////////////////////////////////////////////////////////////////////
//     Function: P3DConcreteSequence::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DConcreteSequence::
~P3DConcreteSequence() {
  Elements::iterator ei;
  for (ei = _elements.begin(); ei != _elements.end(); ++ei) {
    P3D_OBJECT_DECREF(*ei);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DConcreteSequence::is_sequence_object
//       Access: Public, Virtual
//  Description: Returns true if this is actually an instance of a
//               P3DConcreteSequence, false otherwise.
////////////////////////////////////////////////////////////////////
bool P3DConcreteSequence::
is_sequence_object() {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DConcreteSequence::get_type
//       Access: Public, Virtual
//  Description: Returns the fundamental type of this kind of object.
////////////////////////////////////////////////////////////////////
P3D_object_type P3DConcreteSequence::
get_type() {
  return P3D_OT_object;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DConcreteSequence::get_bool
//       Access: Public, Virtual
//  Description: Returns the object value coerced to a boolean, if
//               possible.
////////////////////////////////////////////////////////////////////
bool P3DConcreteSequence::
get_bool() {
  return !_elements.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DConcreteSequence::make_string
//       Access: Public, Virtual
//  Description: Fills the indicated C++ string object with the value
//               of this object coerced to a string.
////////////////////////////////////////////////////////////////////
void P3DConcreteSequence::
make_string(string &value) {
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
//     Function: P3DConcreteSequence::get_property
//       Access: Public, Virtual
//  Description: Returns the named property element in the object.  The
//               return value is a new-reference P3D_object, or NULL
//               on error.
////////////////////////////////////////////////////////////////////
P3D_object *P3DConcreteSequence::
get_property(const string &property) {
  // We only understand integer "property" names.
  char *endptr;
  int index = strtoul(property.c_str(), &endptr, 10);
  if (*endptr != '\0') {
    return NULL;
  }

  return get_element(index);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DConcreteSequence::set_property
//       Access: Public, Virtual
//  Description: Modifies (or deletes, if value is NULL) the named
//               property element in the object.  Returns true on
//               success, false on failure.
////////////////////////////////////////////////////////////////////
bool P3DConcreteSequence::
set_property(const string &property, P3D_object *value) {
  // We only understand integer "property" names.
  char *endptr;
  int index = strtoul(property.c_str(), &endptr, 10);
  if (*endptr != '\0') {
    return false;
  }

  return set_element(index, value);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DConcreteSequence::fill_xml
//       Access: Public, Virtual
//  Description: If this object has a valid XML representation for the
//               indicated session (that hasn't already been
//               implemented by the generic code in P3DSession), this
//               method will apply it to the indicated "value" element
//               and return true.  Otherwise, this method will leave
//               the element unchanged and return false.
////////////////////////////////////////////////////////////////////
bool P3DConcreteSequence::
fill_xml(TiXmlElement *xvalue, P3DSession *session) {
  xvalue->SetAttribute("type", "concrete_sequence");
  Elements::const_iterator ei;
  for (ei = _elements.begin(); ei != _elements.end(); ++ei) {
    xvalue->LinkEndChild(session->p3dobj_to_xml(*ei));
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DConcreteSequence::get_object_array
//       Access: Public
//  Description: Returns a pointer to the array of objects represented
//               by this object.  Most objects represent only
//               themselves, but a P3DConcreteSequence represents its
//               list.
////////////////////////////////////////////////////////////////////
P3D_object **P3DConcreteSequence::
get_object_array() {
  if (_elements.empty()) {
    return NULL;
  }
  return &_elements[0];
}

////////////////////////////////////////////////////////////////////
//     Function: P3DConcreteSequence::get_object_array_size
//       Access: Public
//  Description: Returns the number of elements in the array returned
//               by get_object_array().
////////////////////////////////////////////////////////////////////
int P3DConcreteSequence::
get_object_array_size() {
  return _elements.size();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DConcreteSequence::get_length
//       Access: Public
//  Description: Returns the number of items in the sequence.
////////////////////////////////////////////////////////////////////
int P3DConcreteSequence::
get_length() const {
  return _elements.size();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DConcreteSequence::get_element
//       Access: Public
//  Description: Returns the nth item in the sequence.  The
//               return value is a new-reference P3DObject object, or
//               NULL on error.
////////////////////////////////////////////////////////////////////
P3D_object *P3DConcreteSequence::
get_element(int n) const {
  if (n >= 0 && n < (int)_elements.size()) {
    P3D_OBJECT_INCREF(_elements[n]);
    return _elements[n];
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DConcreteSequence::set_element
//       Access: Public
//  Description: Modifies (or deletes, if value is NULL) the nth item
//               in the sequence.  Returns true on success, false on
//               failure.
////////////////////////////////////////////////////////////////////
bool P3DConcreteSequence::
set_element(int n, P3D_object *value) {
  if (value == NULL) {
    // Delete an element.
    if (n < 0 || n >= (int)_elements.size()) {
      // Invalid index.
      return false;
    }
    P3D_OBJECT_DECREF(_elements[n]);
    _elements.erase(_elements.begin() + n);
    return true;

  } else if (n == _elements.size()) {
    // Append an element.
    append(value);
    return true;

  } else {
    // Replace an element.
    if (n < 0 || n >= (int)_elements.size()) {
      // Invalid index.
      return false;
    }

    P3D_OBJECT_INCREF(value);
    P3D_OBJECT_DECREF(_elements[n]);
    _elements[n] = value;
    return true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DConcreteSequence::append
//       Access: Public, Virtual
//  Description: Adds a new element to the end of the list.
////////////////////////////////////////////////////////////////////
void P3DConcreteSequence::
append(P3D_object *value) {
  _elements.push_back(value);
  P3D_OBJECT_INCREF(value);
}
