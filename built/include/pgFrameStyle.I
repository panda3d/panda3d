/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pgFrameStyle.I
 * @author drose
 * @date 2001-07-03
 */

/**
 *
 */
INLINE PGFrameStyle::
PGFrameStyle() {
  _type = T_none;
  _color.set(1.0f, 1.0f, 1.0f, 1.0f);
  _width.set(0.1f, 0.1f);
  _uv_width.set(0.1f, 0.1f);
  _visible_scale.set(1.0f, 1.0f);
}

/**
 *
 */
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

/**
 *
 */
INLINE void PGFrameStyle::
operator = (const PGFrameStyle &copy) {
  _type = copy._type;
  _color = copy._color;
  _texture = copy._texture;
  _width = copy._width;
  _uv_width = copy._uv_width;
  _visible_scale = copy._visible_scale;
}

/**
 *
 */
INLINE PGFrameStyle::
~PGFrameStyle() {
}

/**
 * Sets the basic type of frame.
 */
INLINE void PGFrameStyle::
set_type(PGFrameStyle::Type type) {
  _type = type;
}

/**
 * Returns the basic type of frame.
 */
INLINE PGFrameStyle::Type PGFrameStyle::
get_type() const {
  return _type;
}

/**
 * Sets the dominant color of the frame.
 */
INLINE void PGFrameStyle::
set_color(PN_stdfloat r, PN_stdfloat g, PN_stdfloat b, PN_stdfloat a) {
  set_color(LColor(r, g, b, a));
}

/**
 * Sets the dominant color of the frame.
 */
INLINE void PGFrameStyle::
set_color(const LColor &color) {
  _color = color;
}

/**
 * Returns the dominant color of the frame.
 */
INLINE LColor PGFrameStyle::
get_color() const {
  return _color;
}

/**
 * Specifies a texture that should be applied to the frame.
 */
INLINE void PGFrameStyle::
set_texture(Texture *texture) {
  _texture = texture;
}

/**
 * Returns true if a texture has been applied to the frame.
 */
INLINE bool PGFrameStyle::
has_texture() const {
  return !_texture.is_null();
}

/**
 * Returns the texture that has been applied to the frame, or NULL if no
 * texture has been applied.
 */
INLINE Texture *PGFrameStyle::
get_texture() const {
  return _texture;
}

/**
 * Removes the texture from the frame.
 */
INLINE void PGFrameStyle::
clear_texture() {
  _texture.clear();
}

/**
 * Sets the width parameter, which has meaning only for certain frame types.
 * For instance, this is the width of the bevel for T_bevel_in or T_bevel_out.
 * The units are in screen units.
 */
INLINE void PGFrameStyle::
set_width(PN_stdfloat x, PN_stdfloat y) {
  set_width(LVecBase2(x, y));
}

/**
 * Sets the width parameter, which has meaning only for certain frame types.
 * For instance, this is the width of the bevel for T_bevel_in or T_bevel_out.
 * The units are in screen units.
 */
INLINE void PGFrameStyle::
set_width(const LVecBase2 &width) {
  _width = width;
}

/**
 * Returns the width parameter, which has meaning only for certain frame
 * types.  For instance, this is the width of the bevel for T_bevel_in or
 * T_bevel_out.  The units are in screen units.
 */
INLINE const LVecBase2 &PGFrameStyle::
get_width() const {
  return _width;
}

/**
 * Sets the uv_width parameter, which indicates the amount of the texture that
 * is consumed by the inner bevel--the width in texture space of the amount
 * indicated by set_width.
 */
INLINE void PGFrameStyle::
set_uv_width(PN_stdfloat u, PN_stdfloat v) {
  set_uv_width(LVecBase2(u, v));
}

/**
 * Sets the uv_width parameter, which indicates the amount of the texture that
 * is consumed by the inner bevel--the width in texture space of the amount
 * indicated by set_width.
 */
INLINE void PGFrameStyle::
set_uv_width(const LVecBase2 &uv_width) {
  _uv_width = uv_width;
}

/**
 * See set_uv_width().
 */
INLINE const LVecBase2 &PGFrameStyle::
get_uv_width() const {
  return _uv_width;
}

/**
 * Sets a scale factor on the visible representation of the frame, in the X
 * and Y directions.  If this scale factor is other than 1, it will affect the
 * size of the visible frame representation within the actual frame border.
 */
INLINE void PGFrameStyle::
set_visible_scale(PN_stdfloat x, PN_stdfloat y) {
  set_visible_scale(LVecBase2(x, y));
}

/**
 * Sets a scale factor on the visible representation of the frame, in the X
 * and Y directions.  If this scale factor is other than 1, it will affect the
 * size of the visible frame representation within the actual frame border.
 */
INLINE void PGFrameStyle::
set_visible_scale(const LVecBase2 &visible_scale) {
  _visible_scale = visible_scale;
}

/**
 * Returns the scale factor on the visible representation of the frame, in the
 * X and Y directions.  If this scale factor is other than 1, it will affect
 * the size of the visible frame representation within the actual frame
 * border.
 */
INLINE const LVecBase2 &PGFrameStyle::
get_visible_scale() const {
  return _visible_scale;
}

/**
 *
 */
INLINE std::ostream &
operator << (std::ostream &out, const PGFrameStyle &pfs) {
  pfs.output(out);
  return out;
}
