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
//     Function: EggVertexUV::set_name
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggVertexUV::
set_name(const string &name) {
  if (name == "default") {
    clear_name();
  } else {
    Namable::set_name(name);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggVertexUV::get_num_dimensions
//       Access: Published
//  Description: Returns the number of components of the texture
//               coordinate set.  This is either 2 (the normal case)
//               or 3 (for a 3-d texture coordinate).
////////////////////////////////////////////////////////////////////
INLINE int EggVertexUV::
get_num_dimensions() const {
  return has_w() ? 3 : 2;
}

////////////////////////////////////////////////////////////////////
//     Function: EggVertexUV::has_w
//       Access: Published
//  Description: Returns true if the texture coordinate has a third, w
//               component, false if it is just a normal 2-d texture
//               coordinate.
////////////////////////////////////////////////////////////////////
INLINE bool EggVertexUV::
has_w() const {
  return (_flags & F_has_w) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: EggVertexUV::get_uv
//       Access: Published
//  Description: Returns the texture coordinate pair, if
//               get_num_dimensions() is 2.
////////////////////////////////////////////////////////////////////
INLINE TexCoordd EggVertexUV::
get_uv() const {
  nassertr(!has_w(), TexCoordd::zero());
  return TexCoordd(_uvw[0], _uvw[1]);
}

////////////////////////////////////////////////////////////////////
//     Function: EggVertexUV::get_uvw
//       Access: Published
//  Description: Returns the texture coordinate triple, if
//               get_num_dimensions() is 3.  This is also legal to
//               call if get_num_dimensions() is 2 (but the last
//               dimension will be zero).
////////////////////////////////////////////////////////////////////
INLINE const TexCoord3d &EggVertexUV::
get_uvw() const {
  return _uvw;
}

////////////////////////////////////////////////////////////////////
//     Function: EggVertexUV::set_uv
//       Access: Published
//  Description: Sets the texture coordinate pair.  This makes the
//               texture coordinate a 2-d texture coordinate, which is
//               the usual case.
////////////////////////////////////////////////////////////////////
INLINE void EggVertexUV::
set_uv(const TexCoordd &uv) {
  _uvw.set(uv[0], uv[1], 0.0);
  _flags &= ~F_has_w;
}

////////////////////////////////////////////////////////////////////
//     Function: EggVertexUV::set_uvw
//       Access: Published
//  Description: Sets the texture coordinate triple.  This makes the
//               texture coordinate a 3-d texture coordinate.
////////////////////////////////////////////////////////////////////
INLINE void EggVertexUV::
set_uvw(const TexCoord3d &uvw) {
  _uvw = uvw;
  _flags |= F_has_w;
}

////////////////////////////////////////////////////////////////////
//     Function: EggVertexUV::has_tangent
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool EggVertexUV::
has_tangent() const {
  return (_flags & F_has_tangent) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: EggVertexUV::get_tangent
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const Normald &EggVertexUV::
get_tangent() const {
  nassertr(has_tangent(), _tangent);
  return _tangent;
}

////////////////////////////////////////////////////////////////////
//     Function: EggVertexUV::set_tangent
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggVertexUV::
set_tangent(const Normald &tangent) {
  _tangent = tangent;
  _flags |= F_has_tangent;
}

////////////////////////////////////////////////////////////////////
//     Function: EggVertexUV::clear_tangent
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggVertexUV::
clear_tangent() {
  _flags &= ~F_has_tangent;
}

////////////////////////////////////////////////////////////////////
//     Function: EggVertexUV::has_binormal
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool EggVertexUV::
has_binormal() const {
  return (_flags & F_has_binormal) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: EggVertexUV::get_binormal
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const Normald &EggVertexUV::
get_binormal() const {
  nassertr(has_binormal(), _binormal);
  return _binormal;
}

////////////////////////////////////////////////////////////////////
//     Function: EggVertexUV::set_binormal
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggVertexUV::
set_binormal(const Normald &binormal) {
  _binormal = binormal;
  _flags |= F_has_binormal;
}

////////////////////////////////////////////////////////////////////
//     Function: EggVertexUV::clear_binormal
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggVertexUV::
clear_binormal() {
  _flags &= ~F_has_binormal;
}
