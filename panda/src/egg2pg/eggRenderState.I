// Filename: eggRenderState.I
// Created by:  drose (12Mar05)
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
//     Function: EggRenderState::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
EggRenderState::
EggRenderState(EggLoader &loader) :
  _state(RenderState::make_empty()),
  _hidden(false),
  _flat_shaded(false),
  _primitive_type(qpGeom::PT_none),
  _loader(loader)
{
}

////////////////////////////////////////////////////////////////////
//     Function: EggRenderState::add_attrib
//       Access: Public
//  Description: A convenience function to add the indicated render
//               attribute to the aggregate state.
////////////////////////////////////////////////////////////////////
INLINE void EggRenderState::
add_attrib(const RenderAttrib *attrib) {
  _state = _state->add_attrib(attrib);
}
