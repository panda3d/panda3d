// Filename: cullBinAttrib.I
// Created by:  drose (01Mar02)
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
//     Function: CullBinAttrib::Constructor
//       Access: Private
//  Description: Use CullBinAttrib::make() to construct a new
//               CullBinAttrib object.
////////////////////////////////////////////////////////////////////
INLINE CullBinAttrib::
CullBinAttrib() {
  _draw_order = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinAttrib::get_bin_name
//       Access: Published
//  Description: Returns the name of the bin this attribute specifies.
//               If this is the empty string, it refers to the default
//               bin.
////////////////////////////////////////////////////////////////////
INLINE const string &CullBinAttrib::
get_bin_name() const {
  return _bin_name;
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinAttrib::get_draw_order
//       Access: Published
//  Description: Returns the draw order this attribute specifies.
//               Some bins (in particular, CullBinFixed bins) use this
//               to further specify the order in which objects should
//               be rendered.
////////////////////////////////////////////////////////////////////
INLINE int CullBinAttrib::
get_draw_order() const {
  return _draw_order;
}
