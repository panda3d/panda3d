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
#include "renderBuffer.h"
#include "config_display.h"

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
  for (int i=0; i<FBP_COUNT; i++) {
    _specified[i] = copy._specified[i];
    _property[i]  = copy._property[i];
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::subsumes
//       Access: Public
//  Description: Returns true if this set of properties makes
//               strictly greater or equal demands of the framebuffer
//               than the other set of framebuffer properties.
////////////////////////////////////////////////////////////////////
bool FrameBufferProperties::
subsumes(const FrameBufferProperties &other) const {
  for (int i=0; i<FBP_COUNT; i++) {
    if (other._property[i] > _property[i]) {
      return false;
    }
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::get_default
//       Access: Published, Static
//  Description: Returns a FrameBufferProperties structure with all of
//               the default values filled in according to the user's
//               config file.
////////////////////////////////////////////////////////////////////
const FrameBufferProperties &FrameBufferProperties::
get_default() {
  static bool                  default_ready = false;
  static FrameBufferProperties default_props;

  if (default_ready) {
    return default_props;
  }

  default_props._property[FBP_rgb_color] = 1;
  
  int num_words = framebuffer_mode.get_num_words();
  for (int i = 0; i < num_words; i++) {
    string word = framebuffer_mode.get_word(i);
    if (cmp_nocase_uh(word, "rgb") == 0) {
      default_props._property[FBP_indexed_color] = 0;
      default_props._property[FBP_color_bits] = color_bits;
      default_props._property[FBP_rgb_color] = 1;

    } else if (cmp_nocase_uh(word, "index") == 0) {
      default_props._property[FBP_indexed_color] = 1;
      default_props._property[FBP_rgb_color] = 0;

    } else if (cmp_nocase_uh(word, "single") == 0 ||
               cmp_nocase_uh(word, "single-buffer") == 0) {
      default_props._property[FBP_back_buffers] = 0;

    } else if (cmp_nocase_uh(word, "double") == 0 ||
               cmp_nocase_uh(word, "double-buffer") == 0) {
      default_props._property[FBP_back_buffers] = 1;

    } else if (cmp_nocase_uh(word, "triple") == 0 ||
               cmp_nocase_uh(word, "triple-buffer") == 0) {
      default_props._property[FBP_back_buffers] = 2;

    } else if (cmp_nocase_uh(word, "accum") == 0) {
      default_props._property[FBP_accum_bits] = 1;

    } else if (cmp_nocase_uh(word, "alpha") == 0) {
      default_props._property[FBP_indexed_color] = 0;
      default_props._property[FBP_rgb_color] = 1;
      default_props._property[FBP_alpha_bits] = alpha_bits;
      
    } else if (cmp_nocase_uh(word, "rgba") == 0) {
      default_props._property[FBP_indexed_color] = 0;
      default_props._property[FBP_rgb_color] = 1;
      default_props._property[FBP_color_bits] = color_bits;
      default_props._property[FBP_alpha_bits] = alpha_bits;
      
    } else if (cmp_nocase_uh(word, "depth") == 0) {
      default_props._property[FBP_depth_bits] = depth_bits;

    } else if (cmp_nocase_uh(word, "stencil") == 0) {
      default_props._property[FBP_stencil_bits] = stencil_bits;

    } else if (cmp_nocase_uh(word, "multisample") == 0) {
      default_props._property[FBP_multisamples] = multisamples;

    } else if (cmp_nocase_uh(word, "stereo") == 0) {
      default_props._property[FBP_stereo] = 1;

    } else if (cmp_nocase_uh(word, "software") == 0) {
      default_props._property[FBP_force_software] = 1;

    } else if (cmp_nocase_uh(word, "hardware") == 0) {
      default_props._property[FBP_force_hardware] = 1;

    } else {
      display_cat.warning()
        << "Unknown framebuffer keyword: " << word << "\n";
    }
  }

  if (framebuffer_hardware) {
    default_props.set_force_hardware(1);
  }
  if (framebuffer_software) {
    default_props.set_force_software(1);
  }
  if (framebuffer_multisample) {
    default_props.set_multisamples(1);
  }
  if (framebuffer_depth) {
    default_props.set_depth_bits(depth_bits);
  }
  if (framebuffer_alpha) {
    default_props.set_alpha_bits(alpha_bits);
  }
  if (framebuffer_stencil) {
    default_props.set_stencil_bits(stencil_bits);
  }
  if (framebuffer_stereo) {
    default_props.set_stereo(1);
  }
  
  if ((default_props._property[FBP_force_software])&&
      (default_props._property[FBP_force_hardware])) {
    default_props._property[FBP_force_software] = 0;
    default_props._property[FBP_force_hardware] = 0;
  }

  default_ready = true;
  return default_props;
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::operator == 
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
bool FrameBufferProperties::
operator == (const FrameBufferProperties &other) const {
  for (int i=0; i<FBP_COUNT; i++) {
    if (_specified[i] != other._specified[i]) {
      return false;
    }
    if (_property[i] != other._property[i]) {
      return false;
    }
  }
  return true;
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
  for (int i=0; i<FBP_COUNT; i++) {
    _specified[i] = 0;
    _property[i] = 0;
  }
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
  for (int i=0; i<FBP_COUNT; i++) {
    if (other._specified[i]) {
      _property[i] = other._property[i];
      _specified[i] = true;
    }
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
  if (_property[FBP_depth_bits] > 0) {
    out << "depth_bits=" << _property[FBP_depth_bits] << " ";
  }
  if (_property[FBP_color_bits] > 0) {
    out << "color_bits=" << _property[FBP_color_bits] << " ";
  }
  if (_property[FBP_alpha_bits] > 0) {
    out << "alpha_bits=" << _property[FBP_alpha_bits] << " ";
  }
  if (_property[FBP_stencil_bits] > 0) {
    out << "stencil_bits=" << _property[FBP_stencil_bits] << " ";
  }
  if (_property[FBP_accum_bits] > 0) {
    out << "accum_bits=" << _property[FBP_accum_bits] << " ";
  }
  if (_property[FBP_aux_rgba] > 0) {
    out << "aux_rgba=" << _property[FBP_aux_rgba] << " ";
  }
  if (_property[FBP_aux_hrgba] > 0) {
    out << "aux_hrgba=" << _property[FBP_aux_hrgba] << " ";
  }
  if (_property[FBP_aux_float] > 0) {
    out << "aux_float=" << _property[FBP_aux_float] << " ";
  }
  if (_property[FBP_multisamples] > 0) {
    out << "multisamples=" << _property[FBP_multisamples] << " ";
  }
  if (_property[FBP_back_buffers] > 0) {
    out << "back_buffers=" << _property[FBP_back_buffers] << " ";
  }
  if (_property[FBP_indexed_color] > 0) {
    out << "indexed_color=" << _property[FBP_indexed_color] << " ";
  }
  if (_property[FBP_stereo] > 0) {
    out << "stereo=" << _property[FBP_stereo] << " ";
  }
  if (_property[FBP_force_hardware] > 0) {
    out << "force_hardware=" << _property[FBP_force_hardware] << " ";
  }
  if (_property[FBP_force_software] > 0) {
    out << "force_software=" << _property[FBP_force_software] << " ";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::get_buffer_mask
//       Access: Private
//  Description: Converts the framebuffer properties into 
//               a RenderBuffer::Type.
////////////////////////////////////////////////////////////////////
int FrameBufferProperties::
get_buffer_mask() const {
  int mask = 0;
  
  if (_property[FBP_back_buffers] > 0) {
    mask = RenderBuffer::T_front | RenderBuffer::T_back;
  } else {
    mask = RenderBuffer::T_front;
  }
  if (_property[FBP_depth_bits] > 0) {
    mask |= RenderBuffer::T_depth;
  }
  if (_property[FBP_stencil_bits] > 0) {
    mask |= RenderBuffer::T_stencil;
  }
  for (int aux_rgba=0; aux_rgba < _property[FBP_aux_rgba]; ++aux_rgba) {
    mask |= (RenderBuffer::T_aux_rgba_0 << aux_rgba);
  }
  for (int aux_hrgba=0; aux_hrgba < _property[FBP_aux_hrgba]; ++aux_hrgba) {
    mask |= (RenderBuffer::T_aux_hrgba_0 << aux_hrgba);
  }
  for (int aux_float=0; aux_float < _property[FBP_aux_float]; ++aux_float) {
    mask |= (RenderBuffer::T_aux_float_0 << aux_float);
  }
  
  return mask;
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::is_any_specified
//       Access: Published
//  Description: Returns true if any properties have been specified,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool FrameBufferProperties::
is_any_specified() const {
  for (int i=0; i<FBP_COUNT; i++) {
    if (_specified[i]) {
      return true;
    }
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::set_all_specified
//       Access: Published
//  Description: sets all the specified bits.
////////////////////////////////////////////////////////////////////
void FrameBufferProperties::
set_all_specified() {
  for (int i=0; i<FBP_COUNT; i++) {
    _specified[i] = true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::get_property_set
//       Access: Published
//  Description: returns a summary of which properties are nonzero.
////////////////////////////////////////////////////////////////////
int FrameBufferProperties::
get_property_set() const {
  int result = 0;
  for (int i=0; i<FBP_COUNT; i++) {
    if (_property[i]) {
      result |= (1<<i);
    }
  }
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::is_basic
//       Access: Published
//  Description: Returns true if the properties are extremely basic.
//               The following count as basic: rgb or rgba, depth.
//               If anything else is specified, the properties are
//               non-basic.
////////////////////////////////////////////////////////////////////
bool FrameBufferProperties::
is_basic() const {
  if (_property[FBP_depth_bits] > 1) {
    return false;
  }
  if (_property[FBP_color_bits] > 1) {
    return false;
  }
  if (_property[FBP_alpha_bits] > 1) {
    return false;
  }
  if (_property[FBP_stencil_bits] > 0) {
    return false;
  }
  if (_property[FBP_aux_rgba] > 0) {
    return false;
  }
  if (_property[FBP_aux_hrgba] > 0) {
    return false;
  }
  if (_property[FBP_aux_float] > 0) {
    return false;
  }
  if (_property[FBP_multisamples] > 1) {
    return false;
  }
  if (_property[FBP_back_buffers] > 0) {
    return false;
  }
  if (_property[FBP_indexed_color] > 0) {
    return false;
  }
  if (_property[FBP_force_hardware] > 0) {
    return false;
  }
  if (_property[FBP_force_software] > 0) {
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::set_one_bit_per_channel
//       Access: Published
//  Description: If any of the depth, color, alpha, accum, or
//               stencil properties is set to more than one,
//               then they are reduced to one.
////////////////////////////////////////////////////////////////////
void FrameBufferProperties::
set_one_bit_per_channel() {
  for (int prop=FBP_depth_bits; prop<=FBP_accum_bits; ++prop) {
    if (_property[prop] > 1) {
      _property[prop] = 1;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::get_quality
//       Access: Published
//  Description: Assumes that these properties are a description of
//               a window.
//
//               Measures how well this window satisfies a specified
//               set of requirements.  A higher quality number means
//               that more requirements were satisfied.  A quality of
//               zero means that the window is unsuitable.
//
//               The routine deducts a lot if the window fails to
//               provide a requested feature.  It deducts less
//               if the window provides a feature, but at a degraded
//               level of functionality (ie, the user asks for rgba8,
//               color, but the window only provides rgba4).  The
//               routine also deducts a small amount for unnecessary
//               features.  For example, if the window has an
//               accumulation buffer when one is not requested will
//               reduce quality slightly.  Maximum quality is obtained
//               when the window exactly matches the request.
//
//               If you want to know whether the window satisfies
//               all of the requirements, use the "subsumes" function.
////////////////////////////////////////////////////////////////////
int FrameBufferProperties::
get_quality(const FrameBufferProperties &reqs) const {

  if ((_property[FBP_indexed_color]==0) && (_property[FBP_rgb_color]==0)) {
    // Nonfunctioning window.
    return 0;
  }
  
  if ((reqs._property[FBP_rgb_color]      > _property[FBP_rgb_color])||
      (reqs._property[FBP_indexed_color]  > _property[FBP_indexed_color])) {
    // These properties are nonnegotiable.
    return 0;
  }

  int quality = 100000000;

  // Deduct for using the wrong kind of renderer (hardware or software).
  // Cost: 10,000,000
  
  if ((reqs._property[FBP_force_hardware] > _property[FBP_force_hardware])||
      (reqs._property[FBP_force_software] > _property[FBP_force_software])) {
    quality -= 10000000;
  }

  // Deduct for missing depth, color, alpha, stencil, or accum.
  // Cost: 1,000,000

  for (int prop=FBP_depth_bits; prop<=FBP_accum_bits; prop++) {
    if ((reqs._property[prop]) && (_property[prop]==0)) {
      quality -= 1000000;
    }
  }

  // Deduct for missing aux bitplanes.
  // Cost: 100,000

  for (int prop=FBP_aux_rgba; prop<=FBP_aux_float; prop++) {
    if (reqs._property[prop] > _property[prop]) {
      quality -= 100000;
    }
  }

  // Deduct for stereo not enabled.
  // Cost: 100,000

  if (reqs._property[FBP_stereo] > _property[FBP_stereo]) {
    quality -= 100000;
  }

  // Deduct for insufficient back-buffers.
  // Cost: 100,000
  
  if (reqs._property[FBP_back_buffers] > _property[FBP_back_buffers]) {
    quality -= 100000;
  }

  // Deduct for not enough bits in depth, color, alpha, stencil, or accum.
  // Cost: 10,000

  for (int prop=FBP_depth_bits; prop<=FBP_accum_bits; prop++) {
    if (reqs._property[prop] > _property[prop]) {
      quality -= 10000;
    }
  }

  // deduct for insufficient multisamples.
  // Cost: 1,000

  if (reqs._property[FBP_multisamples] > _property[FBP_multisamples]) {
    quality -= 1000;
  }

  // Deduct for unrequested bitplanes.
  // Cost: 10
  
  for (int prop=FBP_depth_bits; prop<=FBP_accum_bits; prop++) {
    if ((_property[prop]) && (reqs._property[prop] == 0)) {
      quality -= 10;
    }
  }
  for (int prop=FBP_aux_rgba; prop<=FBP_aux_float; prop++) {
    int extra = _property[prop] > reqs._property[prop];
    if (extra > 0) {
      if (extra > 3) {
        quality -= 30;
      } else {
        quality -= extra*10;
      }
    }
  }
  
  // Deduct for excessive resolution in any bitplane.
  // Cost: 10
  
  for (int prop=FBP_depth_bits; prop<=FBP_accum_bits; prop++) {
    if (reqs._property[prop] <= 8) {
      if (_property[prop] > 8) {
        quality -= 10;
      }
    }
    if (reqs._property[prop] <= 32) {
      if (_property[prop] > 32) {
        quality -= 10;
      }
    }
  }
  
  return quality;
};

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::verify_hardware_software
//       Access: Public
//  Description: Validates that the properties represent the desired
//               kind of renderer (hardware or software).  If not,
//               prints out an error message and returns false.
////////////////////////////////////////////////////////////////////
bool FrameBufferProperties::
verify_hardware_software(const FrameBufferProperties &props, const string &renderer) const {

  if (get_force_hardware() < props.get_force_hardware()) {
    display_cat.error()
      << "The application requested harware acceleration, but your OpenGL\n";
    display_cat.error()
      << "driver, " << renderer << ", only supports software rendering.\n";
    display_cat.error()
      << "You need to install a hardware-accelerated OpenGL driver, or,\n";
    display_cat.error()
      << "if you actually *want* to use a software renderer, then\n";
    display_cat.error()
      << "change the word 'hardware' to 'software' in the Config.prc file.\n";
    return false;
  }

  if (get_force_software() < props.get_force_software()) {
    display_cat.error()
      << "The application requested a software renderer, but your OpenGL\n";
    display_cat.error()
      << "driver, " << renderer << ", is probably hardware-accelerated.\n";
    display_cat.error()
      << "If you want to allow hardware acceleration, then change the word\n";
    display_cat.error()
      << "'software' to 'hardware' in the Config.prc file.\n";
    return false;
  }
  
  return true;
}


