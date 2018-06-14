/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file p3dConcreteStruct.cxx
 * @author drose
 * @date 2009-07-14
 */

#include "p3dConcreteStruct.h"

using std::string;

/**
 *
 */
P3DConcreteStruct::
P3DConcreteStruct() {
}

/**
 *
 */
P3DConcreteStruct::
~P3DConcreteStruct() {
  Elements::iterator ei;
  for (ei = _elements.begin(); ei != _elements.end(); ++ei) {
    P3D_OBJECT_DECREF((*ei).second);
  }
}

/**
 * Returns the fundamental type of this kind of object.
 */
P3D_object_type P3DConcreteStruct::
get_type() {
  return P3D_OT_object;
}

/**
 * Returns the object value coerced to a boolean, if possible.
 */
bool P3DConcreteStruct::
get_bool() {
  return !_elements.empty();
}

/**
 * Fills the indicated C++ string object with the value of this object coerced
 * to a string.
 */
void P3DConcreteStruct::
make_string(string &value) {
  std::ostringstream strm;
  strm << "{";
  if (!_elements.empty()) {
    Elements::iterator ei;
    ei = _elements.begin();
    strm << (*ei).first << ": " << *(*ei).second;
    ++ei;
    while (ei != _elements.end()) {
      strm << ", " << (*ei).first << ": " << *(*ei).second;
      ++ei;
    }
  }
  strm << "}";

  value = strm.str();
}

/**
 * Returns the named property element in the object.  The return value is a
 * new-reference P3D_object, or NULL on error.
 */
P3D_object *P3DConcreteStruct::
get_property(const string &property) {
  Elements::const_iterator ei = _elements.find(property);
  if (ei != _elements.end()) {
    P3D_OBJECT_INCREF((*ei).second);
    return (*ei).second;
  }

  return nullptr;
}

/**
 * Modifies (or deletes, if value is NULL) the named property element in the
 * object.  Returns true on success, false on failure.
 */
bool P3DConcreteStruct::
set_property(const string &property, P3D_object *value) {
  if (value == nullptr) {
    // Delete an element.
    Elements::iterator ei = _elements.find(property);
    if (ei == _elements.end()) {
      // Invalid property.
      return false;
    }
    P3D_OBJECT_DECREF((*ei).second);
    _elements.erase(ei);
    return true;

  } else {
    // Replace or insert an element.
    P3D_OBJECT_INCREF(value);
    std::pair<Elements::iterator, bool> result = _elements.insert(Elements::value_type(property, value));
    if (!result.second) {
      // Replacing an element.
      Elements::iterator ei = result.first;
      P3D_OBJECT_DECREF((*ei).second);
      (*ei).second = value;
    }
    return true;
  }
}

/**
 * Returns true if the named method exists on this object, false otherwise.
 */
bool P3DConcreteStruct::
has_method(const string &method_name) {
  if (method_name == "toString") {
    return true;
  }

  return false;
}

/**
 * Invokes the named method on the object, passing the indicated parameters.
 * If the method name is empty, invokes the object itself.
 *
 * If needs_response is true, the return value is a new-reference P3D_object
 * on success, or NULL on failure.  If needs_response is false, the return
 * value is always NULL, and there is no way to determine success or failure.
 */
P3D_object *P3DConcreteStruct::
call(const string &method_name, bool needs_response,
     P3D_object *params[], int num_params) {
  P3D_object *result = nullptr;

  if (method_name == "toString") {
    string value;
    make_string(value);
    result = P3D_new_string_object(value.data(), value.length());
  }

  if (result != nullptr && !needs_response) {
    P3D_OBJECT_DECREF(result);
    result = nullptr;
  }

  return result;
}


/**
 * If this object has a valid XML representation for the indicated session
 * (that hasn't already been implemented by the generic code in P3DSession),
 * this method will apply it to the indicated "value" element and return true.
 * Otherwise, this method will leave the element unchanged and return false.
 */
bool P3DConcreteStruct::
fill_xml(TiXmlElement *xvalue, P3DSession *session) {
  xvalue->SetAttribute("type", "concrete_struct");
  Elements::const_iterator ei;
  for (ei = _elements.begin(); ei != _elements.end(); ++ei) {
    TiXmlElement *xitem = session->p3dobj_to_xml((*ei).second);
    xitem->SetAttribute("key", (*ei).first);
    xvalue->LinkEndChild(xitem);
  }

  return true;
}
