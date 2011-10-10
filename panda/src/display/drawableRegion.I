// Filename: drawableRegion.I
// Created by:  drose (11Jul02)
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
//     Function: DrawableRegion::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE DrawableRegion::
DrawableRegion() :
  _screenshot_buffer_type(RenderBuffer::T_front),
  _draw_buffer_type(RenderBuffer::T_back)
{
  for (int i=0; i<RTP_COUNT; i++) {
    _clear_active[i] = false;
    _clear_value[i] = LColor(0.0f, 0.0f, 0.0f, 0.0f);
  }
  _clear_value[RTP_depth] = LColor(1.0f,1.0f,1.0f,1.0f);
  _pixel_zoom = 1.0f;
  _pixel_factor = 1.0f;
}

////////////////////////////////////////////////////////////////////
//     Function: DrawableRegion::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE DrawableRegion::
DrawableRegion(const DrawableRegion &copy) :
  _screenshot_buffer_type(copy._screenshot_buffer_type),
  _draw_buffer_type(copy._draw_buffer_type),
  _pixel_zoom(copy._pixel_zoom),
  _pixel_factor(copy._pixel_factor)
{
  for (int i=0; i<RTP_COUNT; i++) {
    _clear_active[i] = copy._clear_active[i];
    _clear_value[i] = copy._clear_value[i];
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DrawableRegion::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DrawableRegion::
operator = (const DrawableRegion &copy) {
  _screenshot_buffer_type = copy._screenshot_buffer_type;
  _draw_buffer_type = copy._draw_buffer_type;
  for (int i=0; i<RTP_COUNT; i++) {
    _clear_active[i] = copy._clear_active[i];
    _clear_value[i] = copy._clear_value[i];
  }
  _pixel_zoom = copy._pixel_zoom;
  _pixel_factor = copy._pixel_factor;
}

////////////////////////////////////////////////////////////////////
//     Function: DrawableRegion::copy_clear_settings
//       Access: Public
//  Description: Copies only the clear settings from the other drawable
//               region.
////////////////////////////////////////////////////////////////////
INLINE void DrawableRegion::
copy_clear_settings(const DrawableRegion &copy) {
  for (int i=0; i<RTP_COUNT; i++) {
    _clear_active[i] = copy._clear_active[i];
    _clear_value[i] = copy._clear_value[i];
  }
  update_pixel_factor();
}

////////////////////////////////////////////////////////////////////
//     Function: DrawableRegion::set_clear_color_active
//       Access: Published
//  Description: Toggles the flag that indicates whether the color
//               buffer should be cleared every frame.  If this is
//               true, the color buffer will be cleared to the color
//               indicated by set_clear_color(); otherwise, it will be
//               left alone.
////////////////////////////////////////////////////////////////////
INLINE void DrawableRegion::
set_clear_color_active(bool clear_color_active) {
  set_clear_active(RTP_color, clear_color_active);
}

////////////////////////////////////////////////////////////////////
//     Function: DrawableRegion::get_clear_color_active
//       Access: Published
//  Description: Returns the current setting of the flag that
//               indicates whether the color buffer should be cleared
//               every frame.  See set_clear_color_active().
////////////////////////////////////////////////////////////////////
INLINE bool DrawableRegion::
get_clear_color_active() const {
  return get_clear_active(RTP_color);
}

////////////////////////////////////////////////////////////////////
//     Function: DrawableRegion::set_clear_depth_active
//       Access: Published
//  Description: Toggles the flag that indicates whether the depth
//               buffer should be cleared every frame.  If this is
//               true, the depth buffer will be cleared to the depth
//               value indicated by set_clear_depth(); otherwise, it
//               will be left alone.
////////////////////////////////////////////////////////////////////
INLINE void DrawableRegion::
set_clear_depth_active(bool clear_depth_active) {
  set_clear_active(RTP_depth, clear_depth_active);
}

////////////////////////////////////////////////////////////////////
//     Function: DrawableRegion::get_clear_depth_active
//       Access: Published
//  Description: Returns the current setting of the flag that
//               indicates whether the depth buffer should be cleared
//               every frame.  See set_clear_depth_active().
////////////////////////////////////////////////////////////////////
INLINE bool DrawableRegion::
get_clear_depth_active() const {
  return get_clear_active(RTP_depth);
}

////////////////////////////////////////////////////////////////////
//     Function: DrawableRegion::set_clear_stencil_active
//       Access: Published
//  Description: Toggles the flag that indicates whether the stencil
//               buffer should be cleared every frame.  If this is
//               true, the stencil buffer will be cleared to the value
//               indicated by set_clear_stencil(); otherwise, it will be
//               left alone.
////////////////////////////////////////////////////////////////////
INLINE void DrawableRegion::
set_clear_stencil_active(bool clear_stencil_active) {
  set_clear_active(RTP_stencil, clear_stencil_active);
}

////////////////////////////////////////////////////////////////////
//     Function: DrawableRegion::get_clear_stencil_active
//       Access: Published
//  Description: Returns the current setting of the flag that
//               indicates whether the color buffer should be cleared
//               every frame.  See set_clear_stencil_active().
////////////////////////////////////////////////////////////////////
INLINE bool DrawableRegion::
get_clear_stencil_active() const {
  return get_clear_active(RTP_stencil);
}

////////////////////////////////////////////////////////////////////
//     Function: DrawableRegion::set_clear_color
//       Access: Published
//  Description: Sets the clear color to the indicated value.  This is
//               the value that will be used to clear the color buffer
//               every frame, but only if get_clear_color_active()
//               returns true.  If get_clear_color_active() returns
//               false, this is meaningless.
////////////////////////////////////////////////////////////////////
INLINE void DrawableRegion::
set_clear_color(const LColor &color) {
  set_clear_value(RTP_color, color);
}

////////////////////////////////////////////////////////////////////
//     Function: DrawableRegion::get_clear_color
//       Access: Published
//  Description: Returns the current clear color value.  This is
//               the value that will be used to clear the color buffer
//               every frame, but only if get_clear_color_active()
//               returns true.  If get_clear_color_active() returns
//               false, this is meaningless.
////////////////////////////////////////////////////////////////////
INLINE const LColor &DrawableRegion::
get_clear_color() const {
  return get_clear_value(RTP_color);
}

////////////////////////////////////////////////////////////////////
//     Function: DrawableRegion::set_clear_depth
//       Access: Published
//  Description: Sets the clear depth to the indicated value.  This is
//               the value that will be used to clear the depth buffer
//               every frame, but only if get_clear_depth_active()
//               returns true.  If get_clear_depth_active() returns
//               false, this is meaningless.
////////////////////////////////////////////////////////////////////
INLINE void DrawableRegion::
set_clear_depth(PN_stdfloat depth) {
  set_clear_value(RTP_depth, LColor(depth,depth,depth,depth));
}

////////////////////////////////////////////////////////////////////
//     Function: DrawableRegion::get_clear_depth
//       Access: Published
//  Description: Returns the current clear depth value.  This is
//               the value that will be used to clear the depth buffer
//               every frame, but only if get_clear_depth_active()
//               returns true.  If get_clear_depth_active() returns
//               false, this is meaningless.
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat DrawableRegion::
get_clear_depth() const {
  return get_clear_value(RTP_depth)[0];
}
////////////////////////////////////////////////////////////////////
//     Function: DrawableRegion::set_clear_stencil
//       Access: Published
//  Description: Sets the clear stencil to the indicated value.  This is
//               the value that will be used to clear the stencil buffer
//               every frame, but only if get_clear_color_active()
//               returns true.  If get_clear_stencil_active() returns
//               false, this is meaningless.
////////////////////////////////////////////////////////////////////
INLINE void DrawableRegion::
set_clear_stencil(const unsigned int stencil) {
  set_clear_value(RTP_stencil, LColor(stencil,stencil,stencil,stencil));
}

////////////////////////////////////////////////////////////////////
//     Function: DrawableRegion::get_clear_stencil
//       Access: Published
//  Description: Returns the current clear stencil value.  This is
//               the value that will be used to clear the stencil buffer
//               every frame, but only if get_clear_stencil_active()
//               returns true.  If get_clear_stencil_active() returns
//               false, this is meaningless.
////////////////////////////////////////////////////////////////////
INLINE unsigned int DrawableRegion::
get_clear_stencil() const {
  return (int)(get_clear_value(RTP_stencil)[0]);
}

////////////////////////////////////////////////////////////////////
//     Function: DrawableRegion::get_pixel_zoom
//       Access: Published
//  Description: Returns the value set by set_pixel_zoom(), regardless
//               of whether it is being respected or not.  Also see
//               get_pixel_factor().
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat DrawableRegion::
get_pixel_zoom() const {
  return _pixel_zoom;
}

////////////////////////////////////////////////////////////////////
//     Function: DrawableRegion::get_pixel_factor
//       Access: Published
//  Description: Returns the amount by which the height and width of
//               the region will be scaled internally, based on the
//               zoom factor set by set_pixel_zoom().  This will
//               return 1.0 if the pixel_zoom was not set or if it is
//               not being respected (for instance, because the
//               underlying renderer doesn't support it--see
//               supports_pixel_zoom).
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat DrawableRegion::
get_pixel_factor() const {
  return _pixel_factor;
}

////////////////////////////////////////////////////////////////////
//     Function: DrawableRegion::get_screenshot_buffer_type
//       Access: Public
//  Description: Returns the RenderBuffer that should be used for
//               capturing screenshots from this particular
//               DrawableRegion.
////////////////////////////////////////////////////////////////////
INLINE int DrawableRegion::
get_screenshot_buffer_type() const {
  return _screenshot_buffer_type;
}

////////////////////////////////////////////////////////////////////
//     Function: DrawableRegion::get_draw_buffer_type
//       Access: Public
//  Description: Returns the RenderBuffer into which the GSG should
//               issue draw commands.  Normally, this is the back
//               buffer for double-buffered windows, and the front
//               buffer for single-buffered windows.
////////////////////////////////////////////////////////////////////
INLINE int DrawableRegion::
get_draw_buffer_type() const {
  return _draw_buffer_type;
}

////////////////////////////////////////////////////////////////////
//     Function: DrawableRegion::update_pixel_factor
//       Access: Protected
//  Description: Internal function to reset pixel_factor after it may
//               have changed.
////////////////////////////////////////////////////////////////////
INLINE void DrawableRegion::
update_pixel_factor() {
  PN_stdfloat new_pixel_factor;
  if (supports_pixel_zoom()) {
    new_pixel_factor = 1.0 / sqrt(max(_pixel_zoom, (PN_stdfloat)1.0));
  } else {
    new_pixel_factor = 1.0;
  }
  if (new_pixel_factor != _pixel_factor) {
    _pixel_factor = new_pixel_factor;
    pixel_factor_changed();
  }
}
