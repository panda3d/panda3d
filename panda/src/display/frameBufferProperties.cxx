// Filename: frameBufferProperties.cxx
// Created by:  drose (27Jan03)
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

#include "frameBufferProperties.h"


////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
FrameBufferProperties::
FrameBufferProperties() {
  clear();
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::Copy Assignment Operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void FrameBufferProperties::
operator = (const FrameBufferProperties &copy) {
  _specified = copy._specified;
  _flags = copy._flags;
  _frame_buffer_mode = copy._frame_buffer_mode;
  _depth_bits = copy._depth_bits;
  _color_bits = copy._color_bits;
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::operator == 
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
bool FrameBufferProperties::
operator == (const FrameBufferProperties &other) const {
  return (_specified == other._specified &&
          _flags == other._flags &&
          _frame_buffer_mode == other._frame_buffer_mode &&
          _depth_bits == other._depth_bits &&
          _color_bits == other._color_bits);
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::clear
//       Access: Published
//  Description: Unsets all properties that have been specified so
//               far, and resets the FrameBufferProperties structure to its
//               initial empty state.
////////////////////////////////////////////////////////////////////
void FrameBufferProperties::
clear() {
  _specified = 0;
  _flags = 0;
  _frame_buffer_mode = 0;
  _depth_bits = 0;
  _color_bits = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::add_properties
//       Access: Published
//  Description: Sets any properties that are explicitly specified in
//               other on this object.  Leaves other properties
//               unchanged.
////////////////////////////////////////////////////////////////////
void FrameBufferProperties::
add_properties(const FrameBufferProperties &other) {
  if (other.has_frame_buffer_mode()) {
    set_frame_buffer_mode(other.get_frame_buffer_mode());
  }
  if (other.has_depth_bits()) {
    set_depth_bits(other.get_depth_bits());
  }
  if (other.has_color_bits()) {
    set_color_bits(other.get_color_bits());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::output
//       Access: Published
//  Description: Sets any properties that are explicitly specified in
//               other on this object.  Leaves other properties
//               unchanged.
////////////////////////////////////////////////////////////////////
void FrameBufferProperties::
output(ostream &out) const {
  if (has_frame_buffer_mode()) {
    out << "frameBuffer_mode=";
    int frameBuffer_mode = get_frame_buffer_mode();
    if ((frameBuffer_mode & FM_index) != 0) {
      out << "FM_index";
    } else {
      out << "FM_rgb";
    }

    if ((frameBuffer_mode & FM_triple_buffer) != 0) {
      out << "|FM_triple_buffer";
    } else if ((frameBuffer_mode & FM_double_buffer) != 0) {
      out << "|FM_double_buffer";
    } else {
      out << "|FM_single_buffer";
    }

    if ((frameBuffer_mode & FM_accum) != 0) {
      out << "|FM_accum";
    }
    if ((frameBuffer_mode & FM_alpha) != 0) {
      out << "|FM_alpha";
    }
    if ((frameBuffer_mode & FM_depth) != 0) {
      out << "|FM_depth";
    }
    if ((frameBuffer_mode & FM_stencil) != 0) {
      out << "|FM_stencil";
    }
    if ((frameBuffer_mode & FM_multisample) != 0) {
      out << "|FM_multisample";
    }
    if ((frameBuffer_mode & FM_stereo) != 0) {
      out << "|FM_stereo";
    }
    if ((frameBuffer_mode & FM_luminance) != 0) {
      out << "|FM_luminance";
    }
    out << " ";
  }
  if (has_depth_bits()) {
    out << "depth_bits=" << get_depth_bits() << " ";
  }
  if (has_color_bits()) {
    out << "color_bits=" << get_color_bits() << " ";
  }
}
