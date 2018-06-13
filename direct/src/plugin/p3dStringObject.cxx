/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file p3dStringObject.cxx
 * @author drose
 * @date 2009-06-30
 */

#include "p3dStringObject.h"

/**
 *
 */
P3DStringObject::
P3DStringObject(const std::string &value) : _value(value) {
}

/**
 *
 */
P3DStringObject::
P3DStringObject(const char *data, size_t size) : _value(data, size) {
}

/**
 *
 */
P3DStringObject::
P3DStringObject(const P3DStringObject &copy) :
  P3DObject(copy),
  _value(copy._value)
{
}

/**
 *
 */
P3DStringObject::
~P3DStringObject() {
}

/**
 * Returns the fundamental type of this kind of object.
 */
P3D_object_type P3DStringObject::
get_type() {
  return P3D_OT_string;
}

/**
 * Returns the object value coerced to a boolean, if possible.
 */
bool P3DStringObject::
get_bool() {
  return !_value.empty();
}

/**
 * Fills the indicated C++ string object with the value of this object coerced
 * to a string.
 */
void P3DStringObject::
make_string(std::string &value) {
  value = _value;
}

/**
 * Writes a formatted representation of the value to the indicated string.
 * This is intended for developer assistance.
 */
void P3DStringObject::
output(std::ostream &out) {
  out << '"';
  for (std::string::const_iterator si = _value.begin(); si != _value.end(); ++si) {
    if (isprint(*si)) {
      switch (*si) {
      case '"':
        out << "\\\x22";
        break;

      default:
        out << *si;
      }
    } else {
      switch (*si) {
      case '\n':
        out << "\\n";
        break;

      case '\t':
        out << "\\t";
        break;

      default:
        {
          char buffer[128];
          sprintf(buffer, "%02x", (unsigned char)(*si));
          out << "\\x" << buffer;
        }
      }
    }
  }
  out << '"';
}
