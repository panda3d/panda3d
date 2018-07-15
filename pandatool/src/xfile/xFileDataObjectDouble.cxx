/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file xFileDataObjectDouble.cxx
 * @author drose
 * @date 2004-10-07
 */

#include "xFileDataObjectDouble.h"
#include "string_utils.h"
#include "indent.h"

TypeHandle XFileDataObjectDouble::_type_handle;

/**
 *
 */
XFileDataObjectDouble::
XFileDataObjectDouble(const XFileDataDef *data_def, double value) :
  XFileDataObject(data_def),
  _value(value)
{
}

/**
 * Writes a suitable representation of this node to an .x file in text mode.
 */
void XFileDataObjectDouble::
output_data(std::ostream &out) const {
  out << get_string_value();
}

/**
 * Writes a suitable representation of this node to an .x file in text mode.
 */
void XFileDataObjectDouble::
write_data(std::ostream &out, int indent_level, const char *separator) const {
  indent(out, indent_level)
    << get_string_value() << separator << "\n";
}

/**
 * Sets the object's value as an integer, if this is legal.
 */
void XFileDataObjectDouble::
set_int_value(int int_value) {
  _value = (double)int_value;
}

/**
 * Sets the object's value as a floating-point number, if this is legal.
 */
void XFileDataObjectDouble::
set_double_value(double double_value) {
  _value = double_value;
}

/**
 * Returns the object's representation as an integer, if it has one.
 */
int XFileDataObjectDouble::
get_int_value() const {
  return (int)_value;
}

/**
 * Returns the object's representation as a double, if it has one.
 */
double XFileDataObjectDouble::
get_double_value() const {
  return _value;
}

/**
 * Returns the object's representation as a string, if it has one.
 */
std::string XFileDataObjectDouble::
get_string_value() const {
  // It's important to format with a decimal point, even if the value is
  // integral, since the DirectX .x reader differentiates betweens doubles and
  // integers on parsing.
  char buffer[128];
  sprintf(buffer, "%f", _value);

  return buffer;
}
