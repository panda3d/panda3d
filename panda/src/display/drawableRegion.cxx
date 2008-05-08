// Filename: drawableRegion.cxx
// Created by:  drose (11Jul02)
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

#include "drawableRegion.h"


////////////////////////////////////////////////////////////////////
//     Function: DrawableRegion::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
DrawableRegion::
~DrawableRegion() {
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
//       Access: Published, Virtual
//  Description: Called internally when the pixel factor changes.
////////////////////////////////////////////////////////////////////
void DrawableRegion::
pixel_factor_changed() {
}
