// Filename: glGeomMunger_src.I
// Created by:  drose (10Mar05)
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
//     Function: CLP(GeomMunger)::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CLP(GeomMunger)::
CLP(GeomMunger)(GraphicsStateGuardian *gsg, const RenderState *state) :
  StandardMunger(gsg, state, 4, NT_uint8, C_color),
  _texture(state->get_texture()),
  _tex_gen(state->get_tex_gen())
{
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GeomMunger)::operator new
//       Access: Public
//  Description: Calls up to do_operator_new() to implement the
//               low-overhead allocation/deallocation for this type of
//               GeomMunger.
////////////////////////////////////////////////////////////////////
INLINE void *CLP(GeomMunger)::
operator new(size_t size) {
  return do_operator_new(size, &_deleted_chain);
}
