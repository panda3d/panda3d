// Filename: pgFrameStyle.I
// Created by:  drose (03Jul01)
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
//     Function: PGFrameStyle::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE PGFrameStyle::
PGFrameStyle() {
  _type = T_none;
  _color.set(1.0f, 1.0f, 1.0f, 1.0f);
  _width.set(0.1f, 0.1f);
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
set_color(float r, float g, float b, float a) {
  set_color(Colorf(r, g, b, a));
}

////////////////////////////////////////////////////////////////////
//     Function: PGFrameStyle::set_color
//       Access: Published
//  Description: Sets the dominant color of the frame.
////////////////////////////////////////////////////////////////////
INLINE void PGFrameStyle::
set_color(const Colorf &color) {
  _color = color;
}

////////////////////////////////////////////////////////////////////
//     Function: PGFrameStyle::set_color
//       Access: Published
//  Description: Returns the dominant color of the frame.
////////////////////////////////////////////////////////////////////
INLINE const Colorf &PGFrameStyle::
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
set_width(float x, float y) {
  set_width(LVecBase2f(x, y));
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
set_width(const LVecBase2f &width) {
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
INLINE const LVecBase2f &PGFrameStyle::
get_width() const {
  return _width;
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
set_visible_scale(float x, float y) {
  set_visible_scale(LVecBase2f(x, y));
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
set_visible_scale(const LVecBase2f &visible_scale) {
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
INLINE const LVecBase2f &PGFrameStyle::
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
