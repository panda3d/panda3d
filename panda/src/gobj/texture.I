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
//     Function: Texture::apply
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void Texture::
apply(GraphicsStateGuardianBase *gsg) {
  gsg->apply_texture(prepare(gsg));
}


////////////////////////////////////////////////////////////////////
//     Function: Texture::get_level
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int Texture::
get_level() const {
  return _level;
}

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
