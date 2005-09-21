// Filename: attribSlots.I
// Created by:  jyelon (01Sep05)
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
//     Function: AttribSlots::clear_to_zero
//       Access: Private
//  Description: clears all values to zero.
////////////////////////////////////////////////////////////////////
INLINE void AttribSlots::
clear_to_zero() {
  _alpha_test     = NULL;
  _antialias      = NULL;
  _clip_plane     = NULL;
  _color          = NULL;
  _color_blend    = NULL;
  _color_scale    = NULL;
  _color_write    = NULL;
  _cull_bin       = NULL;
  _cull_face      = NULL;
  _depth_offset   = NULL;
  _depth_test     = NULL;
  _depth_write    = NULL;
  _fog            = NULL;
  _light          = NULL;
  _material       = NULL;
  _render_mode    = NULL;
  _rescale_normal = NULL;
  _shade_model    = NULL;
  _shader         = NULL;
  _tex_gen        = NULL;
  _tex_matrix     = NULL;
  _texture        = NULL;
  _transparency   = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: AttribSlots::get_defaults
//       Access: Public
//  Description: Returns a set of slots containing all the default
//               attribute values, as defined by make_default_impl.
////////////////////////////////////////////////////////////////////
INLINE const AttribSlots &AttribSlots::
get_defaults() {
  if (_defvals._alpha_test == 0) {
    initialize_defvals();
  }
  return _defvals;
}

////////////////////////////////////////////////////////////////////
//     Function: AttribSlots::clear_to_defaults
//       Access: Private
//  Description: clears all values to their default values.
////////////////////////////////////////////////////////////////////
INLINE void AttribSlots::
clear_to_defaults() {
  (*this) = get_defaults();
}
