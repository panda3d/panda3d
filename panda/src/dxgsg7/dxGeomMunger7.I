// Filename: dxGeomMunger7.I
// Created by:  drose (11Mar05)
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
//     Function: DXGeomMunger7::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE DXGeomMunger7::
DXGeomMunger7(GraphicsStateGuardian *gsg, const RenderState *state) :
  StandardMunger(gsg, state, 1, NT_packed_dabc, C_color),
  _texture(state->get_texture()),
  _tex_gen(state->get_tex_gen())
{
  if (_texture != (TextureAttrib *)NULL) {
    _texture = _texture->filter_to_max(gsg->get_max_texture_stages());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGeomMunger7::operator new
//       Access: Public
//  Description: Calls up to do_operator_new() to implement the
//               low-overhead allocation/deallocation for this type of
//               GeomMunger.
////////////////////////////////////////////////////////////////////
INLINE void *DXGeomMunger7::
operator new(size_t size) {
  return do_operator_new(size, &_deleted_chain);
}
