// Filename: xFileDataObject.cxx
// Created by:  drose (03Oct04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "xFileDataObject.h"
#include "indent.h"

TypeHandle XFileDataObject::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: XFileDataObject::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
XFileDataObject::
~XFileDataObject() {
}

////////////////////////////////////////////////////////////////////
//     Function: XFileDataObject::is_complex_object
//       Access: Public, Virtual
//  Description: Returns true if this kind of data object is a complex
//               object that can hold nested data elements, false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool XFileDataObject::
is_complex_object() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileDataObject::add_element
//       Access: Public, Virtual
//  Description: Adds the indicated element as a nested data element,
//               if this data object type supports it.  Returns true
//               if added successfully, false if the data object type
//               does not support nested data elements.
////////////////////////////////////////////////////////////////////
bool XFileDataObject::
add_element(XFileDataObject *element) {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileDataObject::output_data
//       Access: Public, Virtual
//  Description: Writes a suitable representation of this node to an
//               .x file in text mode.
////////////////////////////////////////////////////////////////////
void XFileDataObject::
output_data(ostream &out) const {
  out << "(" << get_type() << "::output_data() not implemented.)";
}

////////////////////////////////////////////////////////////////////
//     Function: XFileDataObject::write_data
//       Access: Public, Virtual
//  Description: Writes a suitable representation of this node to an
//               .x file in text mode.
////////////////////////////////////////////////////////////////////
void XFileDataObject::
write_data(ostream &out, int indent_level, const char *) const {
  indent(out, indent_level)
    << "(" << get_type() << "::write_data() not implemented.)\n";
}

////////////////////////////////////////////////////////////////////
//     Function: XFileDataObject::as_integer_value
//       Access: Protected, Virtual
//  Description: Returns the object's representation as an integer, if
//               it has one.
////////////////////////////////////////////////////////////////////
int XFileDataObject::
as_integer_value() const {
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileDataObject::as_double_value
//       Access: Protected, Virtual
//  Description: Returns the object's representation as a double, if
//               it has one.
////////////////////////////////////////////////////////////////////
double XFileDataObject::
as_double_value() const {
  return 0.0;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileDataObject::as_string_value
//       Access: Protected, Virtual
//  Description: Returns the object's representation as a string, if
//               it has one.
////////////////////////////////////////////////////////////////////
string XFileDataObject::
as_string_value() const {
  return string();
}

////////////////////////////////////////////////////////////////////
//     Function: XFileDataObject::get_num_elements
//       Access: Protected, Virtual
//  Description: Returns the number of nested data elements within the
//               object.  This may be, e.g. the size of the array, if
//               it is an array.
////////////////////////////////////////////////////////////////////
int XFileDataObject::
get_num_elements() const {
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileDataObject::get_element
//       Access: Protected, Virtual
//  Description: Returns the nth nested data element within the
//               object.
////////////////////////////////////////////////////////////////////
const XFileDataObject *XFileDataObject::
get_element(int n) const {
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileDataObject::get_element
//       Access: Protected, Virtual
//  Description: Returns the nested data element within the
//               object that has the indicated name.
////////////////////////////////////////////////////////////////////
const XFileDataObject *XFileDataObject::
get_element(const string &name) const {
  return NULL;
}
