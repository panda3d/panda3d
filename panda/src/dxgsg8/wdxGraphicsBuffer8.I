// Filename: wdxGraphicsBuffer8.I
// Created by:  zhao (29Sep12)
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
//     Function: wdxGraphicsBuffer8::get_supports_render_texture
//       Access: Published, Virtual
//  Description: Returns true if this particular GraphicsOutput can
//               render directly into a texture, or false if it must
//               always copy-to-texture at the end of each frame to
//               achieve this effect.
////////////////////////////////////////////////////////////////////
INLINE bool wdxGraphicsBuffer8::
get_supports_render_texture() const {
  // DX8 buffers can always bind-to-texture.
  return true;
}
