// Filename: eggRenderState.I
// Created by:  drose (12Mar05)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
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
  _primitive_type(Geom::PT_none),
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
