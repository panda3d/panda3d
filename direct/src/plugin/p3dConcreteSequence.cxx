/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file p3dConcreteSequence.cxx
 * @author drose
 * @date 2009-06-30
 */

#include "p3dConcreteSequence.h"
#include "p3dSession.h"

/**
 *
 */
P3DConcreteSequence::
P3DConcreteSequence() {
}

/**
 *
 */
P3DConcreteSequence::
~P3DConcreteSequence() {
  Elements::iterator ei;
  for (ei = _elements.begin(); ei != _elements.end(); ++ei) {
    P3D_OBJECT_DECREF(*ei);
  }
}

/**
 * Returns true if this is actually an instance of a P3DConcreteSequence,
 * false otherwise.
 */
bool P3DConcreteSequence::
is_sequence_object() {
  return true;
}

/**
 * Returns the fundamental type of this kind of object.
 */
P3D_object_type P3DConcreteSequence::
get_type() {
  return P3D_OT_object;
}

/**
 * Returns the object value coerced to a boolean, if possible.
 */
bool P3DConcreteSequence::
get_bool() {
  return !_elements.empty();
}

/**
 * Fills the indicated C++ string object with the value of this object coerced
 * to a string.
 */
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

/**
 * Returns the named property element in the object.  The return value is a
 * new-reference P3D_object, or NULL on error.
 */
P3D_object *P3DConcreteSequence::
get_property(const string &property) {
  // We only understand integer "property" names.
  char *endptr;
  int index = strtoul(property.c_str(), &endptr, 10);
  if (*endptr != '\0') {
    return nullptr;
  }

  return get_element(index);
}

/**
 * Modifies (or deletes, if value is NULL) the named property element in the
 * object.  Returns true on success, false on failure.
 */
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

/**
 * If this object has a valid XML representation for the indicated session
 * (that hasn't already been implemented by the generic code in P3DSession),
 * this method will apply it to the indicated "value" element and return true.
 * Otherwise, this method will leave the element unchanged and return false.
 */
bool P3DConcreteSequence::
fill_xml(TiXmlElement *xvalue, P3DSession *session) {
  xvalue->SetAttribute("type", "concrete_sequence");
  Elements::const_iterator ei;
  for (ei = _elements.begin(); ei != _elements.end(); ++ei) {
    xvalue->LinkEndChild(session->p3dobj_to_xml(*ei));
  }

  return true;
}

/**
 * Returns a pointer to the array of objects represented by this object.  Most
 * objects represent only themselves, but a P3DConcreteSequence represents its
 * list.
 */
P3D_object **P3DConcreteSequence::
get_object_array() {
  if (_elements.empty()) {
    return nullptr;
  }
  return &_elements[0];
}

/**
 * Returns the number of elements in the array returned by get_object_array().
 */
int P3DConcreteSequence::
get_object_array_size() {
  return _elements.size();
}

/**
 * Returns the number of items in the sequence.
 */
int P3DConcreteSequence::
get_length() const {
  return _elements.size();
}

/**
 * Returns the nth item in the sequence.  The return value is a new-reference
 * P3DObject object, or NULL on error.
 */
P3D_object *P3DConcreteSequence::
get_element(int n) const {
  if (n >= 0 && n < (int)_elements.size()) {
    P3D_OBJECT_INCREF(_elements[n]);
    return _elements[n];
  }

  return nullptr;
}

/**
 * Modifies (or deletes, if value is NULL) the nth item in the sequence.
 * Returns true on success, false on failure.
 */
bool P3DConcreteSequence::
set_element(int n, P3D_object *value) {
  if (value == nullptr) {
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

/**
 * Adds a new element to the end of the list.
 */
void P3DConcreteSequence::
append(P3D_object *value) {
  _elements.push_back(value);
  P3D_OBJECT_INCREF(value);
}
