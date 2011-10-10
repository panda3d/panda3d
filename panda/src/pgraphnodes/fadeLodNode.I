// Filename: fadelodNode.I
// Created by:  sshodhan (14Jun04)
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
//     Function: FadeLODNode::set_fade_time
//       Access: Published
//  Description: set the time taken to complete an LOD switch
////////////////////////////////////////////////////////////////////
INLINE void FadeLODNode::
set_fade_time(PN_stdfloat t) {
  _fade_time = t;
}

////////////////////////////////////////////////////////////////////
//     Function: FadeLODNode::get_fade_time
//       Access: Published
//  Description: get the time taken to complete an LOD switch
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat FadeLODNode::
get_fade_time() const {
  return _fade_time;
}


////////////////////////////////////////////////////////////////////
//     Function: FadeLODNode::get_fade_bin_name
//       Access: Published
//  Description: Returns the cull bin that is assigned to the fading
//               part of the geometry during a transition.
////////////////////////////////////////////////////////////////////
INLINE const string &FadeLODNode::
get_fade_bin_name() const {
  return _fade_bin_name;
}

////////////////////////////////////////////////////////////////////
//     Function: FadeLODNode::get_fade_bin_draw_order
//       Access: Published
//  Description: Returns the draw order that is assigned (along with
//               the bin name) to the fading part of the geometry
//               during a transition.
////////////////////////////////////////////////////////////////////
INLINE int FadeLODNode::
get_fade_bin_draw_order() const {
  return _fade_bin_draw_order;
}

////////////////////////////////////////////////////////////////////
//     Function: FadeLODNode::get_fade_state_override
//       Access: Published
//  Description: Returns the override value that is applied to the
//               state changes necessary to apply the fade effect.
//               This should be larger than any attrib overrides on
//               the fading geometry.
////////////////////////////////////////////////////////////////////
INLINE int FadeLODNode::
get_fade_state_override() const {
  return _fade_state_override;
}


