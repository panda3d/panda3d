// Filename: texture.I
// Created by:  drose (05Feb99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: Texture::get_wrapu
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Texture::WrapMode Texture::
get_wrapu() const {
  return _wrapu;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_wrapv
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Texture::WrapMode Texture::
get_wrapv() const {
  return _wrapv;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_minfilter
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Texture::FilterType Texture::
get_minfilter() const {
  return _minfilter;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_magfilter
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Texture::FilterType Texture::
get_magfilter() const {
  return _magfilter;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_anisotropic_degree
//       Access: Published
//  Description: Returns the degree of anisotropic filtering that
//               should be applied to the texture.  Normally, this is
//               1, to indicate that anisotropic filtering should be
//               disabled.  If this is a number higher than 1,
//               anisotropic filtering should be enabled (if the
//               rendering backend supports it).
////////////////////////////////////////////////////////////////////
INLINE int Texture::
get_anisotropic_degree() const {
  return _anisotropic_degree;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::uses_mipmaps
//       Access: Public
//  Description: Returns true if the minfilter settings on this
//               texture require the use of mipmapping, false
//               otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool Texture::
uses_mipmaps() const {
  return is_mipmap(get_minfilter());
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::has_ram_image
//       Access: Public
//  Description: Returns true if the Texture has its image contents
//               available in main RAM, false if it exists only in
//               texture memory or in the prepared GSG context.
////////////////////////////////////////////////////////////////////
INLINE bool Texture::
has_ram_image() const {
  return !_pbuffer->_image.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::set_keep_ram_image
//       Access: Public
//  Description: Sets the flag that indicates whether this Texture is
//               eligible to have its main RAM copy of the texture
//               memory dumped when the texture is prepared for
//               rendering.
//
//               This will be true for most textures, which can reload
//               their images if needed by rereading the input file.
//               However, textures that were generated dynamically and
//               cannot be easily reloaded will want to set this flag
//               to true, so that the _pbuffer member will always keep
//               its image copy around.
////////////////////////////////////////////////////////////////////
INLINE void Texture::
set_keep_ram_image(bool keep_ram_image) {
  _keep_ram_image = keep_ram_image;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::get_keep_ram_image
//       Access: Public
//  Description: Returns the flag that indicates whether this Texture
//               is eligible to have its main RAM copy of the texture
//               memory dumped when the texture is prepared for
//               rendering.  See set_keep_ram_image().
////////////////////////////////////////////////////////////////////
INLINE bool Texture::
get_keep_ram_image() const {
  return _keep_ram_image;
}

////////////////////////////////////////////////////////////////////
//     Function: Texture::apply
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void Texture::
apply(GraphicsStateGuardianBase *gsg) {
  gsg->apply_texture(prepare(gsg));
}


