// Filename: tinyGraphicsStateGuardian.I
// Created by:  drose (24Apr08)
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
//     Function: TinyGraphicsStateGuardian::clear_light_state
//       Access: Private
//  Description: Removes the current list of active lights from the
//               current state context.
////////////////////////////////////////////////////////////////////
INLINE void TinyGraphicsStateGuardian::
clear_light_state() {
  _c->lighting_enabled = false;
#ifndef NDEBUG
  GLLight *gl_light = _c->first_light;
  while (gl_light != (GLLight *)NULL) {
    GLLight *next = gl_light->next;
    gl_light->next = NULL;
    gl_light = next;
  }
#endif  // NDEBUG
  _c->first_light = NULL;
}
