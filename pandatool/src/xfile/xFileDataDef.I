// Filename: xFileDataDef.I
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
//     Function: XFileDataDef::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE XFileDataDef::
XFileDataDef(XFile *x_file, const string &name, 
             XFileDataDef::Type type, XFileTemplate *xtemplate) :
  XFileNode(x_file, name),
  _type(type),
  _template(xtemplate)
{
}

////////////////////////////////////////////////////////////////////
//     Function: XFileDataDef::get_data_type
//       Access: Public
//  Description: Returns the primitive type of this element, or
//               T_template if this represents a nested template
//               object.
////////////////////////////////////////////////////////////////////
INLINE XFileDataDef::Type XFileDataDef::
get_data_type() const {
  return _type;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileDataDef::get_template
//       Access: Public
//  Description: If get_data_type() returned T_template, this returns
//               the particular template pointer that this object
//               represents.
////////////////////////////////////////////////////////////////////
INLINE XFileTemplate *XFileDataDef::
get_template() const {
  return _template;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileDataDef::get_num_array_defs
//       Access: Public
//  Description: Returns the number of dimensions of array elements on
//               this data object, or 0 if the data object is not an
//               array.
////////////////////////////////////////////////////////////////////
INLINE int XFileDataDef::
get_num_array_defs() const {
  return _array_def.size();
}

////////////////////////////////////////////////////////////////////
//     Function: XFileDataDef::get_array_def
//       Access: Public
//  Description: Returns the description of the nth dimension of array
//               elements on this data object.
////////////////////////////////////////////////////////////////////
INLINE const XFileArrayDef &XFileDataDef::
get_array_def(int i) const {
  nassertr(i >= 0 && i < (int)_array_def.size(), _array_def[0]);
  return _array_def[i];
}
