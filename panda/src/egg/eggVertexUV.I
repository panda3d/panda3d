// Filename: eggVertexUV.I
// Created by:  drose (20Jul04)
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
//     Function: EggVertexUV::get_uv
//       Access: Published
//  Description: Returns the texture coordinate pair.
////////////////////////////////////////////////////////////////////
INLINE const TexCoordd &EggVertexUV::
get_uv() const {
  return _uv;
}

////////////////////////////////////////////////////////////////////
//     Function: EggVertexUV::set_uv
//       Access: Published
//  Description: Sets the texture coordinate pair.
////////////////////////////////////////////////////////////////////
INLINE void EggVertexUV::
set_uv(const TexCoordd &uv) {
  _uv = uv;
}
