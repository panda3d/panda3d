// Filename: builderAttrib.I
// Created by:  drose (22Jan99)
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
//     Function: BuilderAttrib::set_normal_value
//       Access: Public
//  Description: Reassigns the normal, without knowing whether the
//               attribute is indexed or nonindexed.  A nonindexed
//               attribute will look up the index in the array and
//               store the resulting value, while an indexed attribute
//               will just store the index number (which assumes the
//               array is the same one it's indexing on).
////////////////////////////////////////////////////////////////////
INLINE void BuilderAttrib::
set_normal_value(const BuilderN *array, ushort index) {
  set_normal(array[index]);
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderAttrib::set_color_value
//       Access: Public
//  Description: Reassigns the color, without knowing whether the
//               attribute is indexed or nonindexed.  A nonindexed
//               attribute will look up the index in the array and
//               store the resulting value, while an indexed attribute
//               will just store the index number (which assumes the
//               array is the same one it's indexing on).
////////////////////////////////////////////////////////////////////
INLINE void BuilderAttrib::
set_color_value(const BuilderC *array, ushort index) {
  set_color(array[index]);
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderAttribI::set_normal_value
//       Access: Public
//  Description: Reassigns the normal, without knowing whether the
//               attribute is indexed or nonindexed.  A nonindexed
//               attribute will look up the index in the array and
//               store the resulting value, while an indexed attribute
//               will just store the index number (which assumes the
//               array is the same one it's indexing on).
////////////////////////////////////////////////////////////////////
INLINE void BuilderAttribI::
set_normal_value(const BuilderN *, ushort index) {
  set_normal(index);
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderAttribI::set_color_value
//       Access: Public
//  Description: Reassigns the color, without knowing whether the
//               attribute is indexed or nonindexed.  A nonindexed
//               attribute will look up the index in the array and
//               store the resulting value, while an indexed attribute
//               will just store the index number (which assumes the
//               array is the same one it's indexing on).
////////////////////////////////////////////////////////////////////
INLINE void BuilderAttribI::
set_color_value(const BuilderC *, ushort index) {
  set_color(index);
}
