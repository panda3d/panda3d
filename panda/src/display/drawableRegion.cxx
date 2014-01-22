// Filename: drawableRegion.cxx
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

#include "drawableRegion.h"
#include "config_display.h"


////////////////////////////////////////////////////////////////////
//     Function: DrawableRegion::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
DrawableRegion::
~DrawableRegion() {
}

////////////////////////////////////////////////////////////////////
//     Function: DrawableRegion::set_clear_active
//       Access: Published, Virtual
//  Description: Sets the clear-active flag for any bitplane.
////////////////////////////////////////////////////////////////////
void DrawableRegion::
set_clear_active(int n, bool clear_active) {
  nassertv((n >= 0)&&(n < RTP_COUNT));
  _clear_active[n] = clear_active;
  update_pixel_factor();
}

////////////////////////////////////////////////////////////////////
//     Function: DrawableRegion::get_clear_active
//       Access: Published, Virtual
//  Description: Gets the clear-active flag for any bitplane.
////////////////////////////////////////////////////////////////////
bool DrawableRegion::
get_clear_active(int n) const {
  nassertr((n >= 0)&&(n < RTP_COUNT), false);
  return _clear_active[n];
}

////////////////////////////////////////////////////////////////////
//     Function: DrawableRegion::set_clear_value
//       Access: Published, Virtual
//  Description: Sets the clear value for any bitplane.
////////////////////////////////////////////////////////////////////
void DrawableRegion::
set_clear_value(int n, const LColor &clear_value) {
  nassertv((n >= 0) && (n < RTP_COUNT));
  _clear_value[n] = clear_value;
}

////////////////////////////////////////////////////////////////////
//     Function: DrawableRegion::get_clear_value
//       Access: Published, Virtual
//  Description: Returns the clear value for any bitplane.
////////////////////////////////////////////////////////////////////
const LColor &DrawableRegion::
get_clear_value(int n) const {
  static LColor blank(0.5,0.5,0.5,0.0);
  nassertr((n >= 0) && (n < RTP_COUNT), blank);
  return _clear_value[n];
}

////////////////////////////////////////////////////////////////////
//     Function: DrawableRegion::disable_clears
//       Access: Published, Virtual
//  Description: Disables both the color and depth clear.  See
//               set_clear_color_active and set_clear_depth_active.
////////////////////////////////////////////////////////////////////
void DrawableRegion::
disable_clears() {
  for (int i = 0; i < RTP_COUNT; ++i) {
    _clear_active[i] = false;
  }
  update_pixel_factor();
}

////////////////////////////////////////////////////////////////////
//     Function: DrawableRegion::is_any_clear_active
//       Access: Published, Virtual
//  Description: Returns true if any of the clear types (so far there
//               are just color or depth) have been set active, or
//               false if none of them are active and there is no need
//               to clear.
////////////////////////////////////////////////////////////////////
bool DrawableRegion::
is_any_clear_active() const {
  for (int i = 0; i < RTP_COUNT; ++i) {
    if (get_clear_active(i)) {
      return true;
    }
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: DrawableRegion::set_pixel_zoom
//       Access: Published, Virtual
//  Description: Sets the amount by which the pixels of the region are
//               scaled internally when filling the image interally.
//               Setting this number larger makes the pixels blockier,
//               but may make the rendering faster, particularly for
//               software renderers.  Setting this number to 2.0
//               reduces the number of pixels that have to be filled
//               by the renderer by a factor of 2.0.  It doesn't make
//               sense to set this lower than 1.0.
//
//               It is possible to set this on either individual
//               DisplayRegions or on overall GraphicsWindows, but you
//               will get better performance for setting it on the
//               window rather than its individual DisplayRegions.
//               Also, you may not set it on a DisplayRegion that
//               doesn't have both clear_color() and clear_depth()
//               enabled.
//
//               This property is only supported on renderers for
//               which it is particularly useful--currently, this is
//               the tinydisplay software renderer.  Other kinds of
//               renderers allow you to set this property, but ignore
//               it.
////////////////////////////////////////////////////////////////////
void DrawableRegion::
set_pixel_zoom(PN_stdfloat pixel_zoom) {
  _pixel_zoom = pixel_zoom;
  update_pixel_factor();
}

////////////////////////////////////////////////////////////////////
//     Function: DrawableRegion::supports_pixel_zoom
//       Access: Published, Virtual
//  Description: Returns true if a call to set_pixel_zoom() will be
//               respected, false if it will be ignored.  If this
//               returns false, then get_pixel_factor() will always
//               return 1.0, regardless of what value you specify for
//               set_pixel_zoom().
//
//               This may return false if the underlying renderer
//               doesn't support pixel zooming, or if you have called
//               this on a DisplayRegion that doesn't have both
//               set_clear_color() and set_clear_depth() enabled.
////////////////////////////////////////////////////////////////////
bool DrawableRegion::
supports_pixel_zoom() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: DrawableRegion::get_renderbuffer_type
//       Access: Static, Published
//  Description: Returns the RenderBuffer::Type that corresponds
//               to a RenderTexturePlane.
////////////////////////////////////////////////////////////////////
int DrawableRegion::
get_renderbuffer_type(int rtp) {
  switch(rtp) {
  case RTP_stencil:        return RenderBuffer::T_stencil;
  case RTP_depth:          return RenderBuffer::T_depth;
  case RTP_depth_stencil:  return RenderBuffer::T_depth | RenderBuffer::T_stencil;
  case RTP_color:          return RenderBuffer::T_color;
  case RTP_aux_rgba_0:     return RenderBuffer::T_aux_rgba_0;
  case RTP_aux_rgba_1:     return RenderBuffer::T_aux_rgba_1;
  case RTP_aux_rgba_2:     return RenderBuffer::T_aux_rgba_2;
  case RTP_aux_rgba_3:     return RenderBuffer::T_aux_rgba_3;
  case RTP_aux_hrgba_0:    return RenderBuffer::T_aux_hrgba_0;
  case RTP_aux_hrgba_1:    return RenderBuffer::T_aux_hrgba_1;
  case RTP_aux_hrgba_2:    return RenderBuffer::T_aux_hrgba_2;
  case RTP_aux_hrgba_3:    return RenderBuffer::T_aux_hrgba_3;
  case RTP_aux_float_0:    return RenderBuffer::T_aux_float_0;
  case RTP_aux_float_1:    return RenderBuffer::T_aux_float_1;
  case RTP_aux_float_2:    return RenderBuffer::T_aux_float_2;
  case RTP_aux_float_3:    return RenderBuffer::T_aux_float_3;
  default:
    display_cat.error() << "DrawableRegion::get_renderbuffer_type unexpected case!\n";
    return 0;
  };
}

////////////////////////////////////////////////////////////////////
//     Function: DrawableRegion::pixel_factor_changed
//       Access: Protected, Virtual
//  Description: Called internally when the pixel factor changes.
////////////////////////////////////////////////////////////////////
void DrawableRegion::
pixel_factor_changed() {
}
