// Filename: dxTextureContext9.I
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
//     Function: DXTextureContext9::has_mipmaps
//       Access: Public
//  Description: Returns true if the texture was created with mipmaps,
//               false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool DXTextureContext9::
has_mipmaps() const {
  return _has_mipmaps;
}

////////////////////////////////////////////////////////////////////
//     Function: DXTextureContext9::get_d3d_texture
//       Access: Public
//  Description: Returns the Direct3D object that represents the
//               texture, whatever kind of texture it is.
////////////////////////////////////////////////////////////////////
INLINE IDirect3DBaseTexture9 *DXTextureContext9::
get_d3d_texture() const {
  return _d3d_texture;
}

////////////////////////////////////////////////////////////////////
//     Function: DXTextureContext9::get_d3d_2d_texture
//       Access: Public
//  Description: Returns the Direct3D object that represents the
//               texture, in the case of a 1-d or 2-d texture.
////////////////////////////////////////////////////////////////////
INLINE IDirect3DTexture9 *DXTextureContext9::
get_d3d_2d_texture() const {
  return _d3d_2d_texture;
}

////////////////////////////////////////////////////////////////////
//     Function: DXTextureContext9::get_d3d_volume_texture
//       Access: Public
//  Description: Returns the Direct3D object that represents the
//               texture, in the case of a 3-d texture.
////////////////////////////////////////////////////////////////////
INLINE IDirect3DVolumeTexture9 *DXTextureContext9::
get_d3d_volume_texture() const {
  return _d3d_volume_texture;
}

////////////////////////////////////////////////////////////////////
//     Function: DXTextureContext9::get_d3d_cube_texture
//       Access: Public
//  Description: Returns the Direct3D object that represents the
//               texture, in the case of a cube map texture.
////////////////////////////////////////////////////////////////////
INLINE IDirect3DCubeTexture9 *DXTextureContext9::
get_d3d_cube_texture() const {
  return _d3d_cube_texture;
}
