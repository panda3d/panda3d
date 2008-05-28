// Filename: dxTextureContext8.I
// Created by:  drose (23May05)
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
//               texture, whatever kind of texture it is.
////////////////////////////////////////////////////////////////////
INLINE IDirect3DBaseTexture8 *DXTextureContext8::
get_d3d_texture() const {
  return _d3d_texture;
}

////////////////////////////////////////////////////////////////////
//     Function: DXTextureContext8::get_d3d_2d_texture
//       Access: Public
//  Description: Returns the Direct3D object that represents the
//               texture, in the case of a 1-d or 2-d texture.
////////////////////////////////////////////////////////////////////
INLINE IDirect3DTexture8 *DXTextureContext8::
get_d3d_2d_texture() const {
  return _d3d_2d_texture;
}

////////////////////////////////////////////////////////////////////
//     Function: DXTextureContext8::get_d3d_volume_texture
//       Access: Public
//  Description: Returns the Direct3D object that represents the
//               texture, in the case of a 3-d texture.
////////////////////////////////////////////////////////////////////
INLINE IDirect3DVolumeTexture8 *DXTextureContext8::
get_d3d_volume_texture() const {
  return _d3d_volume_texture;
}

////////////////////////////////////////////////////////////////////
//     Function: DXTextureContext8::get_d3d_cube_texture
//       Access: Public
//  Description: Returns the Direct3D object that represents the
//               texture, in the case of a cube map texture.
////////////////////////////////////////////////////////////////////
INLINE IDirect3DCubeTexture8 *DXTextureContext8::
get_d3d_cube_texture() const {
  return _d3d_cube_texture;
}
