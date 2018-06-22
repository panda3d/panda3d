/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file xFileDataObjectInteger.cxx
 * @author drose
 * @date 2004-10-07
 */

#include "xFileDataObjectInteger.h"
#include "string_utils.h"
#include "indent.h"

TypeHandle XFileDataObjectInteger::_type_handle;

/**
 *
 */
XFileDataObjectInteger::
XFileDataObjectInteger(const XFileDataDef *data_def, int value) :
  XFileDataObject(data_def),
  _value(value)
{
}

/**
 * Writes a suitable representation of this node to an .x file in text mode.
 */
void XFileDataObjectInteger::
output_data(std::ostream &out) const {
  out << _value;
}

/**
 * Writes a suitable representation of this node to an .x file in text mode.
 */
void XFileDataObjectInteger::
write_data(std::ostream &out, int indent_level, const char *separator) const {
  indent(out, indent_level)
    << _value << separator << "\n";
}

/**
 * Sets the object's value as an integer, if this is legal.
 */
void XFileDataObjectInteger::
set_int_value(int int_value) {
  _value = int_value;
}

/**
 * Returns the object's representation as an integer, if it has one.
 */
int XFileDataObjectInteger::
get_int_value() const {
  return _value;
}

/**
 * Returns the object's representation as a double, if it has one.
 */
double XFileDataObjectInteger::
get_double_value() const {
  return _value;
}

/**
 * Returns the object's representation as a string, if it has one.
 */
std::string XFileDataObjectInteger::
get_string_value() const {
  return format_string(_value);
}
