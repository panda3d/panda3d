// Filename: geomSprite.I
// Created by:  charles (18Jul00)
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
//    Function : GeomSprite::set_texture
//      Access : public
// Description :
////////////////////////////////////////////////////////////////////
INLINE void GeomSprite::
set_texture(Texture *tex) {
  _texture = tex;
}

////////////////////////////////////////////////////////////////////
//    Function : GeomSprite::set_ll_uv
//      Access : public
// Description : Sets the UV coordinate of the lower-left corner of
//               all the sprites within this GeomSprite object.
//               Normally this is (0, 0), but it might be set to
//               something else to use only a portion of the texture.
////////////////////////////////////////////////////////////////////
INLINE void GeomSprite::
set_ll_uv(const TexCoordf &ll_uv) {
  _ll_uv = ll_uv;
}

////////////////////////////////////////////////////////////////////
//    Function : GeomSprite::get_ll_uv
//      Access : public
// Description : Returns the UV coordinate of the lower-left corner;
//               see set_ll_uv().
////////////////////////////////////////////////////////////////////
INLINE const TexCoordf &GeomSprite::
get_ll_uv() const {
  return _ll_uv;
}

////////////////////////////////////////////////////////////////////
//    Function : GeomSprite::set_ur_uv
//      Access : public
// Description : Sets the UV coordinate of the upper-right corner of
//               all the sprites within this GeomSprite object.
//               Normally this is (1, 1), but it might be set to
//               something else to use only a portion of the texture.
////////////////////////////////////////////////////////////////////
INLINE void GeomSprite::
set_ur_uv(const TexCoordf &ur_uv) {
  _ur_uv = ur_uv;
}

////////////////////////////////////////////////////////////////////
//    Function : GeomSprite::get_ur_uv
//      Access : public
// Description : Returns the UV coordinate of the upper-right corner;
//               see set_ur_uv().
////////////////////////////////////////////////////////////////////
INLINE const TexCoordf &GeomSprite::
get_ur_uv() const {
  return _ur_uv;
}

////////////////////////////////////////////////////////////////////
//    Function : GeomSprite::get_texture
//      Access : public
// Description :
////////////////////////////////////////////////////////////////////
INLINE Texture *GeomSprite::
get_texture(void) const {
  return _texture;
}

////////////////////////////////////////////////////////////////////
//    Function : GeomSprite::set_alpha_disable
//      Access : public
// Description :
////////////////////////////////////////////////////////////////////
INLINE void GeomSprite::
set_alpha_disable(bool a) {
  _alpha_disable = a;
}

////////////////////////////////////////////////////////////////////
//    Function : GeomSprite::get_alpha_disable
//      Access : public
// Description :
////////////////////////////////////////////////////////////////////
INLINE bool GeomSprite::
get_alpha_disable(void) const {
  return _alpha_disable;
}

////////////////////////////////////////////////////////////////////
//    Function : GeomSprite::set_x_texel_ratio
//      Access : public
// Description :
////////////////////////////////////////////////////////////////////
INLINE void GeomSprite::
set_x_texel_ratio(PTA_float x_texel_ratio, GeomBindType x_bind_type) {
  _x_texel_ratio = x_texel_ratio;
  _x_bind_type = x_bind_type;
}

////////////////////////////////////////////////////////////////////
//    Function : GeomSprite::set_y_texel_ratio
//      Access : public
// Description :
////////////////////////////////////////////////////////////////////
INLINE void GeomSprite::
set_y_texel_ratio(PTA_float y_texel_ratio, GeomBindType y_bind_type) {
  _y_texel_ratio = y_texel_ratio;
  _y_bind_type = y_bind_type;
}

////////////////////////////////////////////////////////////////////
//    Function : GeomSprite::set_thetas
//      Access : public
// Description :
////////////////////////////////////////////////////////////////////
INLINE void GeomSprite::
set_thetas(PTA_float theta, GeomBindType theta_bind_type) {
  _theta = theta;
  _theta_bind_type = theta_bind_type;
}

////////////////////////////////////////////////////////////////////
//    Function : GeomSprite::get_x_bind_type
//      Access : public
// Description :
////////////////////////////////////////////////////////////////////
INLINE GeomBindType GeomSprite::
get_x_bind_type(void) const {
  return _x_bind_type;
}

////////////////////////////////////////////////////////////////////
//    Function : GeomSprite::get_y_bind_type
//      Access : public
// Description :
////////////////////////////////////////////////////////////////////
INLINE GeomBindType GeomSprite::
get_y_bind_type(void) const {
  return _y_bind_type;
}

////////////////////////////////////////////////////////////////////
//    Function : GeomSprite::get_theta_bind_type
//      Access : public
// Description :
////////////////////////////////////////////////////////////////////
INLINE GeomBindType GeomSprite::
get_theta_bind_type(void) const {
  return _theta_bind_type;
}
