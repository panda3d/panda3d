// Filename: xFileDataObject.I
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


////////////////////////////////////////////////////////////////////
//     Function: XFileDataObject::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE XFileDataObject::
XFileDataObject(const XFileDataDef *data_def) :
  _data_def(data_def)
{
}

////////////////////////////////////////////////////////////////////
//     Function: XFileDataObject::get_data_def
//       Access: Public
//  Description: Returns the data object that this object is
//               represented by, if any, or NULL if there is none.
////////////////////////////////////////////////////////////////////
INLINE const XFileDataDef *XFileDataObject::
get_data_def() const {
  return _data_def;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileDataObject::i
//       Access: Public
//  Description: Unambiguously returns the object's representation as
//               an integer, or 0 if the object has no integer
//               representation.  See also the typecast operators, and
//               get_data_def() to determine what kind of
//               representation this object has.
////////////////////////////////////////////////////////////////////
INLINE int XFileDataObject::
i() const {
  return as_integer_value();
}

////////////////////////////////////////////////////////////////////
//     Function: XFileDataObject::d
//       Access: Public
//  Description: Unambiguously returns the object's representation as
//               a double, or 0.0 if the object has no double
//               representation.  See also the typecast operators, and
//               get_data_def() to determine what kind of
//               representation this object has.
////////////////////////////////////////////////////////////////////
INLINE double XFileDataObject::
d() const {
  return as_double_value();
}

////////////////////////////////////////////////////////////////////
//     Function: XFileDataObject::s
//       Access: Public
//  Description: Unambiguously returns the object's representation as
//               a string, or empty string if the object has no string
//               representation.  See also the typecast operators, and
//               get_data_def() to determine what kind of
//               representation this object has.
////////////////////////////////////////////////////////////////////
INLINE string XFileDataObject::
s() const {
  return as_string_value();
}

////////////////////////////////////////////////////////////////////
//     Function: XFileDataObject::int typecast operator
//       Access: Public
//  Description: Returns the object's representation as an integer, or
//               0 if the object has no integer representation.
////////////////////////////////////////////////////////////////////
INLINE XFileDataObject::
operator int () const {
  return as_integer_value();
}

////////////////////////////////////////////////////////////////////
//     Function: XFileDataObject::double typecast operator
//       Access: Public
//  Description: Returns the object's representation as a double, or
//               0.0 if the object has no integer representation.
////////////////////////////////////////////////////////////////////
INLINE XFileDataObject::
operator double () const {
  return as_double_value();
}

////////////////////////////////////////////////////////////////////
//     Function: XFileDataObject::string typecast operator
//       Access: Public
//  Description: Returns the object's representation as a string, or
//               empty string if the object has no string
//               representation.
////////////////////////////////////////////////////////////////////
INLINE XFileDataObject::
operator string () const {
  return as_string_value();
}

////////////////////////////////////////////////////////////////////
//     Function: XFileDataObject::size
//       Access: Public
//  Description: Returns the number of nested data objects within this
//               object.
////////////////////////////////////////////////////////////////////
INLINE int XFileDataObject::
size() const {
  return get_num_elements();
}

////////////////////////////////////////////////////////////////////
//     Function: XFileDataObject::operator [] (int)
//       Access: Public
//  Description: Returns the nth nested object within this object.
//               Call get_num_children() to determine the number of
//               nested objects.
////////////////////////////////////////////////////////////////////
INLINE const XFileDataObject &XFileDataObject::
operator [] (int n) const {
  const XFileDataObject *element = get_element(n);
  nassertr(element != (XFileDataObject *)NULL, *this);
  return *element;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileDataObject::operator [] (string)
//       Access: Public
//  Description: Returns the named nested object within this object.
//               It is an error if the named object does not exist.
//               Call find_child() instead if there is any doubt.
////////////////////////////////////////////////////////////////////
INLINE const XFileDataObject &XFileDataObject::
operator [] (const string &name) const {
  const XFileDataObject *element = get_element(name);
  nassertr(element != (XFileDataObject *)NULL, *this);
  return *element;
}

INLINE ostream &
operator << (ostream &out, const XFileDataObject &data_object) {
  data_object.output_data(out);
  return out;
}
