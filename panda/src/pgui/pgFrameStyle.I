// Filename: pgFrameStyle.I
// Created by:  drose (03Jul01)
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
//     Function: PGFrameStyle::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE PGFrameStyle::
PGFrameStyle() {
  _type = T_none;
  _color.set(1.0f, 1.0f, 1.0f, 1.0f);
  _width.set(0.1f, 0.1f);
  _uv_width.set(0.1f, 0.1f);
  _visible_scale.set(1.0f, 1.0f);
}

////////////////////////////////////////////////////////////////////
//     Function: PGFrameStyle::Copy Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE PGFrameStyle::
PGFrameStyle(const PGFrameStyle &copy) :
  _type(copy._type),
  _color(copy._color),
  _texture(copy._texture),
  _width(copy._width),
  _uv_width(copy._uv_width),
  _visible_scale(copy._visible_scale)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PGFrameStyle::Copy Assignment Operator
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void PGFrameStyle::
operator = (const PGFrameStyle &copy) {
  _type = copy._type;
  _color = copy._color;
  _texture = copy._texture;
  _width = copy._width;
  _uv_width = copy._uv_width;
  _visible_scale = copy._visible_scale;
}

////////////////////////////////////////////////////////////////////
//     Function: PGFrameStyle::Destructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE PGFrameStyle::
~PGFrameStyle() {
}

////////////////////////////////////////////////////////////////////
//     Function: PGFrameStyle::set_type
//       Access: Published
//  Description: Sets the basic type of frame.
////////////////////////////////////////////////////////////////////
INLINE void PGFrameStyle::
set_type(PGFrameStyle::Type type) {
  _type = type;
}

////////////////////////////////////////////////////////////////////
//     Function: PGFrameStyle::get_type
//       Access: Published
//  Description: Returns the basic type of frame.
////////////////////////////////////////////////////////////////////
INLINE PGFrameStyle::Type PGFrameStyle::
get_type() const {
  return _type;
}

////////////////////////////////////////////////////////////////////
//     Function: PGFrameStyle::set_color
//       Access: Published
//  Description: Sets the dominant color of the frame.
////////////////////////////////////////////////////////////////////
INLINE void PGFrameStyle::
set_color(PN_stdfloat r, PN_stdfloat g, PN_stdfloat b, PN_stdfloat a) {
  set_color(LColor(r, g, b, a));
}

////////////////////////////////////////////////////////////////////
//     Function: PGFrameStyle::set_color
//       Access: Published
//  Description: Sets the dominant color of the frame.
////////////////////////////////////////////////////////////////////
INLINE void PGFrameStyle::
set_color(const LColor &color) {
  _color = color;
}

////////////////////////////////////////////////////////////////////
//     Function: PGFrameStyle::set_color
//       Access: Published
//  Description: Returns the dominant color of the frame.
////////////////////////////////////////////////////////////////////
INLINE LColor PGFrameStyle::
get_color() const {
  return _color;
}

////////////////////////////////////////////////////////////////////
//     Function: PGFrameStyle::set_texture
//       Access: Published
//  Description: Specifies a texture that should be applied to the
//               frame.
////////////////////////////////////////////////////////////////////
INLINE void PGFrameStyle::
set_texture(Texture *texture) {
  _texture = texture;
}

////////////////////////////////////////////////////////////////////
//     Function: PGFrameStyle::has_texture
//       Access: Published
//  Description: Returns true if a texture has been applied to the
//               frame.
////////////////////////////////////////////////////////////////////
INLINE bool PGFrameStyle::
has_texture() const {
  return !_texture.is_null();
}

////////////////////////////////////////////////////////////////////
//     Function: PGFrameStyle::get_texture
//       Access: Published
//  Description: Returns the texture that has been applied to the
//               frame, or NULL if no texture has been applied.
////////////////////////////////////////////////////////////////////
INLINE Texture *PGFrameStyle::
get_texture() const {
  return _texture;
}

////////////////////////////////////////////////////////////////////
//     Function: PGFrameStyle::clear_texture
//       Access: Published
//  Description: Removes the texture from the frame.
////////////////////////////////////////////////////////////////////
INLINE void PGFrameStyle::
clear_texture() {
  _texture.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: PGFrameStyle::set_width
//       Access: Published
//  Description: Sets the width parameter, which has meaning only for
//               certain frame types.  For instance, this is the width
//               of the bevel for T_bevel_in or T_bevel_out.  The
//               units are in screen units.
////////////////////////////////////////////////////////////////////
INLINE void PGFrameStyle::
set_width(PN_stdfloat x, PN_stdfloat y) {
  set_width(LVecBase2(x, y));
}

////////////////////////////////////////////////////////////////////
//     Function: PGFrameStyle::set_width
//       Access: Published
//  Description: Sets the width parameter, which has meaning only for
//               certain frame types.  For instance, this is the width
//               of the bevel for T_bevel_in or T_bevel_out.  The
//               units are in screen units.
////////////////////////////////////////////////////////////////////
INLINE void PGFrameStyle::
set_width(const LVecBase2 &width) {
  _width = width;
}

////////////////////////////////////////////////////////////////////
//     Function: PGFrameStyle::get_width
//       Access: Published
//  Description: Returns the width parameter, which has meaning only
//               for certain frame types.  For instance, this is the
//               width of the bevel for T_bevel_in or T_bevel_out.
//               The units are in screen units.
////////////////////////////////////////////////////////////////////
INLINE const LVecBase2 &PGFrameStyle::
get_width() const {
  return _width;
}

////////////////////////////////////////////////////////////////////
//     Function: PGFrameStyle::set_uv_width
//       Access: Published
//  Description: Sets the uv_width parameter, which indicates the
//               amount of the texture that is consumed by the inner
//               bevel--the width in texture space of the amount
//               indicated by set_width.
////////////////////////////////////////////////////////////////////
INLINE void PGFrameStyle::
set_uv_width(PN_stdfloat u, PN_stdfloat v) {
  set_uv_width(LVecBase2(u, v));
}

////////////////////////////////////////////////////////////////////
//     Function: PGFrameStyle::set_uv_width
//       Access: Published
//  Description: Sets the uv_width parameter, which indicates the
//               amount of the texture that is consumed by the inner
//               bevel--the width in texture space of the amount
//               indicated by set_width.
////////////////////////////////////////////////////////////////////
INLINE void PGFrameStyle::
set_uv_width(const LVecBase2 &uv_width) {
  _uv_width = uv_width;
}

////////////////////////////////////////////////////////////////////
//     Function: PGFrameStyle::get_uv_width
//       Access: Published
//  Description: See set_uv_width().
////////////////////////////////////////////////////////////////////
INLINE const LVecBase2 &PGFrameStyle::
get_uv_width() const {
  return _uv_width;
}

////////////////////////////////////////////////////////////////////
//     Function: PGFrameStyle::set_visible_scale
//       Access: Published
//  Description: Sets a scale factor on the visible representation of
//               the frame, in the X and Y directions.  If this scale
//               factor is other than 1, it will affect the size of
//               the visible frame representation within the actual
//               frame border.
////////////////////////////////////////////////////////////////////
INLINE void PGFrameStyle::
set_visible_scale(PN_stdfloat x, PN_stdfloat y) {
  set_visible_scale(LVecBase2(x, y));
}

////////////////////////////////////////////////////////////////////
//     Function: PGFrameStyle::set_visible_scale
//       Access: Published
//  Description: Sets a scale factor on the visible representation of
//               the frame, in the X and Y directions.  If this scale
//               factor is other than 1, it will affect the size of
//               the visible frame representation within the actual
//               frame border.
////////////////////////////////////////////////////////////////////
INLINE void PGFrameStyle::
set_visible_scale(const LVecBase2 &visible_scale) {
  _visible_scale = visible_scale;
}

////////////////////////////////////////////////////////////////////
//     Function: PGFrameStyle::get_visible_scale
//       Access: Published
//  Description: Returns the scale factor on the visible
//               representation of the frame, in the X and Y
//               directions.  If this scale factor is other than 1, it
//               will affect the size of the visible frame
//               representation within the actual frame border.
////////////////////////////////////////////////////////////////////
INLINE const LVecBase2 &PGFrameStyle::
get_visible_scale() const {
  return _visible_scale;
}

////////////////////////////////////////////////////////////////////
//     Function: PGFrameStyle ostream output
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE ostream &
operator << (ostream &out, const PGFrameStyle &pfs) {
  pfs.output(out);
  return out;
}
