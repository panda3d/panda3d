// Filename: xFileDataNodeReference.cxx
// Created by:  drose (08Oct04)
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

#include "xFileDataNodeReference.h"
#include "indent.h"

TypeHandle XFileDataNodeReference::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: XFileDataNodeReference::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
XFileDataNodeReference::
XFileDataNodeReference(XFileDataNodeTemplate *object) :
  XFileDataNode(object->get_x_file(), object->get_name(),
                object->get_template()),
  _object(object)
{
}

////////////////////////////////////////////////////////////////////
//     Function: XFileDataNodeReference::is_complex_object
//       Access: Public, Virtual
//  Description: Returns true if this kind of data object is a complex
//               object that can hold nested data elements, false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool XFileDataNodeReference::
is_complex_object() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileDataNodeReference::write_text
//       Access: Public, Virtual
//  Description: Writes a suitable representation of this node to an
//               .x file in text mode.
////////////////////////////////////////////////////////////////////
void XFileDataNodeReference::
write_text(ostream &out, int indent_level) const {
  indent(out, indent_level)
    << "{ " << _object->get_name() << " }\n";
}

////////////////////////////////////////////////////////////////////
//     Function: XFileDataNodeReference::get_num_elements
//       Access: Protected, Virtual
//  Description: Returns the number of nested data elements within the
//               object.  This may be, e.g. the size of the array, if
//               it is an array.
////////////////////////////////////////////////////////////////////
int XFileDataNodeReference::
get_num_elements() const {
  return _object->size();
}

////////////////////////////////////////////////////////////////////
//     Function: XFileDataNodeReference::get_element
//       Access: Protected, Virtual
//  Description: Returns the nth nested data element within the
//               object.
////////////////////////////////////////////////////////////////////
const XFileDataObject *XFileDataNodeReference::
get_element(int n) const {
  return &((*_object)[n]);
}

////////////////////////////////////////////////////////////////////
//     Function: XFileDataNodeReference::get_element
//       Access: Protected, Virtual
//  Description: Returns the nested data element within the
//               object that has the indicated name.
////////////////////////////////////////////////////////////////////
const XFileDataObject *XFileDataNodeReference::
get_element(const string &name) const {
  return &((*_object)[name]);
}
