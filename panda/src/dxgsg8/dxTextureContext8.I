// Filename: dxTextureContext8.I
// Created by:  drose (23May05)
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
//     Function: DXTextureContext8::has_mipmaps
//       Access: Public
//  Description: Returns true if the texture was created with mipmaps,
//               false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool DXTextureContext8::
has_mipmaps() const {
  return _has_mipmaps;
}

////////////////////////////////////////////////////////////////////
//     Function: DXTextureContext8::get_d3d_texture
//       Access: Public
//  Description: Returns the Direct3D object that represents the
//               texture.
////////////////////////////////////////////////////////////////////
INLINE IDirect3DTexture8 *DXTextureContext8::
get_d3d_texture() const {
  return _d3d_texture;
}
