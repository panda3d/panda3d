// Filename: frameBufferProperties.cxx
// Created by:  drose (27Jan03)
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

#include "frameBufferProperties.h"
#include "string_utils.h"


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
  _alpha_bits = copy._alpha_bits;
  _stencil_bits = copy._stencil_bits;
  _multisamples = copy._multisamples;
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::get_default
//       Access: Published, Static
//  Description: Returns a FrameBufferProperties structure with all of
//               the default values filled in according to the user's
//               config file.
////////////////////////////////////////////////////////////////////
FrameBufferProperties FrameBufferProperties::
get_default() {
  FrameBufferProperties props;

  int mode = 0;
  int num_words = framebuffer_mode.get_num_words();
  for (int i = 0; i < num_words; i++) {
    string word = framebuffer_mode.get_word(i);
    if (cmp_nocase_uh(word, "rgb") == 0) {
      mode |= FM_rgb;

    } else if (cmp_nocase_uh(word, "index") == 0) {
      mode |= FM_index;

    } else if (cmp_nocase_uh(word, "single") == 0 ||
               cmp_nocase_uh(word, "single-buffer") == 0) {
      mode |= FM_single_buffer;

    } else if (cmp_nocase_uh(word, "double") == 0 ||
               cmp_nocase_uh(word, "double-buffer") == 0) {
      mode |= FM_double_buffer;

    } else if (cmp_nocase_uh(word, "triple") == 0 ||
               cmp_nocase_uh(word, "triple-buffer") == 0) {
      mode |= FM_triple_buffer;

    } else if (cmp_nocase_uh(word, "accum") == 0) {
      mode |= FM_accum;

    } else if (cmp_nocase_uh(word, "alpha") == 0) {
      mode |= FM_alpha;

    } else if (cmp_nocase_uh(word, "rgba") == 0) {
      mode |= FM_rgba;

    } else if (cmp_nocase_uh(word, "depth") == 0) {
      mode |= FM_depth;

    } else if (cmp_nocase_uh(word, "stencil") == 0) {
      mode |= FM_stencil;

    } else if (cmp_nocase_uh(word, "multisample") == 0) {
      mode |= FM_multisample;

    } else if (cmp_nocase_uh(word, "stereo") == 0) {
      mode |= FM_stereo;

    } else if (cmp_nocase_uh(word, "software") == 0) {
      mode |= FM_software;

    } else if (cmp_nocase_uh(word, "hardware") == 0) {
      mode |= FM_hardware;

    } else {
      display_cat.warning()
        << "Unknown framebuffer keyword: " << word << "\n";
    }
  }

  props.set_frame_buffer_mode(mode);
  props.set_depth_bits(depth_bits);
  props.set_color_bits(color_bits);
  props.set_alpha_bits(alpha_bits);
  props.set_stencil_bits(stencil_bits);
  props.set_multisamples(multisamples);

  return props;
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
          _color_bits == other._color_bits &&
          _alpha_bits == other._alpha_bits &&
          _stencil_bits == other._stencil_bits &&
          _multisamples == other._multisamples);
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
  _depth_bits = 1;
  _color_bits = 1;
  _alpha_bits = 1;
  _stencil_bits = 1;
  _multisamples = 1;
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
  if (other.has_alpha_bits()) {
    set_alpha_bits(other.get_alpha_bits());
  }
  if (other.has_stencil_bits()) {
    set_stencil_bits(other.get_stencil_bits());
  }
  if (other.has_multisamples()) {
    set_multisamples(other.get_multisamples());
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
    if ((frameBuffer_mode & FM_software) != 0) {
      out << "|FM_software";
    }
    if ((frameBuffer_mode & FM_hardware) != 0) {
      out << "|FM_hardware";
    }
    out << " ";
  }
  if (has_depth_bits()) {
    out << "depth_bits=" << get_depth_bits() << " ";
  }
  if (has_color_bits()) {
    out << "color_bits=" << get_color_bits() << " ";
  }
  if (has_alpha_bits()) {
    out << "alpha_bits=" << get_alpha_bits() << " ";
  }
  if (has_stencil_bits()) {
    out << "stencil_bits=" << get_stencil_bits() << " ";
  }
  if (has_multisamples()) {
    out << "multisamples=" << get_multisamples() << " ";
  }
}
