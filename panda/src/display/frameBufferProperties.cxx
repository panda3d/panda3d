// Filename: frameBufferProperties.cxx
// Created by:  drose (27Jan03)
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
  _flags_specified = copy._flags_specified;
  _flags = copy._flags;

  for (int i = 0; i < FBP_COUNT; ++i) {
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
  if (((other._flags & other._flags_specified) & ~(_flags & _flags_specified)) != 0) {
    // The other has bits enabled that we don't have enabled.
    return false;
  }

  for (int i = 0; i < FBP_COUNT; ++i) {
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
  static bool default_ready = false;
  static FrameBufferProperties default_props;

  if (default_ready) {
    return default_props;
  }

  default_props.set_rgb_color(true);
  default_props.set_back_buffers(back_buffers);

  int num_words = framebuffer_mode.get_num_words();
  if (num_words > 0) {
    display_cat.error()
      << "The config-variable 'framebuffer-mode' no longer functions.\n";
    display_cat.error()
      << "Instead, use one or more of these:\n";
    display_cat.error() << "  framebuffer-hardware #t\n";
    display_cat.error() << "  framebuffer-software #t\n";
    display_cat.error() << "  framebuffer-depth #t\n";
    display_cat.error() << "  framebuffer-alpha #t\n";
    display_cat.error() << "  framebuffer-stencil #t\n";
    display_cat.error() << "  framebuffer-multisample #t\n";
    display_cat.error() << "  framebuffer-stereo #t\n";
    display_cat.error() << "  depth-bits N\n";
    display_cat.error() << "  color-bits N\n";
    display_cat.error() << "  alpha-bits N\n";
    display_cat.error() << "  stencil-bits N\n";
    display_cat.error() << "  multisamples N\n";
    display_cat.error() << "  coverage-samples N\n";
    display_cat.error() << "  back-buffers N\n";
  }

  if (framebuffer_hardware) {
    default_props.set_force_hardware(true);
  }
  if (framebuffer_software) {
    default_props.set_force_software(true);
  }
  if (framebuffer_depth) {
    default_props.set_depth_bits(1);
  }
  if (framebuffer_alpha) {
    default_props.set_alpha_bits(1);
  }
  if (framebuffer_stencil) {
    default_props.set_stencil_bits(1);
  }
  if (framebuffer_accum) {
    default_props.set_accum_bits(1);
  }
  if (framebuffer_multisample) {
    default_props.set_multisamples(1);
  }
  if (framebuffer_stereo) {
    default_props.set_stereo(true);
  }
  if (framebuffer_srgb) {
    default_props.set_srgb_color(true);
  }
  if (framebuffer_float) {
    default_props.set_float_color(true);
  }
  if (depth_bits > 0) {
    default_props.set_depth_bits(depth_bits);
  }
  if (color_bits > 0) {
    default_props.set_color_bits(color_bits);
  }
  if (alpha_bits > 0) {
    default_props.set_alpha_bits(alpha_bits);
  }
  if (stencil_bits > 0) {
    default_props.set_stencil_bits(stencil_bits);
  }
  if (accum_bits > 0) {
    default_props.set_accum_bits(accum_bits);
  }
  if (multisamples > 0) {
    default_props.set_multisamples(multisamples);
  }

  if ((default_props._flags & FBF_force_software) != 0 &&
      (default_props._flags & FBF_force_hardware) != 0){
    default_props._flags &= ~(FBF_force_software | FBF_force_hardware);
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
  if ((_flags & _flags_specified) != (other._flags & other._flags_specified)) {
    return false;
  }

  for (int i = 0; i < FBP_COUNT; ++i) {
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
  _flags = 0;
  _flags_specified = 0;

  for (int i = 0; i < FBP_COUNT; ++i) {
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
  _flags &= ~other._flags_specified;
  _flags |= other._flags & other._flags_specified;

  for (int i = 0; i < FBP_COUNT; ++i) {
    if (other._specified[i]) {
      _property[i] = other._property[i];
      _specified[i] = true;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::output
//       Access: Published
//  Description: Generates a string representation.
////////////////////////////////////////////////////////////////////
void FrameBufferProperties::
output(ostream &out) const {
  if ((_flags & FBF_float_depth) != 0) {
    out << "float_depth ";
  }
  if (_property[FBP_depth_bits] > 0) {
    out << "depth_bits=" << _property[FBP_depth_bits] << " ";
  }
  if ((_flags & FBF_float_color) != 0) {
    out << "float_color ";
  }
  if ((_flags & FBF_srgb_color) != 0) {
    out << "srgb_color ";
  }
  if ((_flags & FBF_indexed_color) != 0) {
    out << "indexed_color ";
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
  if (_property[FBP_coverage_samples] > 0) {
    out << "coverage_samples=" << _property[FBP_coverage_samples] << " ";
  }
  if (_property[FBP_back_buffers] > 0) {
    out << "back_buffers=" << _property[FBP_back_buffers] << " ";
  }
  if ((_flags & FBF_stereo) != 0) {
    out << "stereo ";
  }
  if ((_flags & FBF_force_hardware) != 0) {
    out << "force_hardware ";
  }
  if ((_flags & FBF_force_software) != 0) {
    out << "force_software ";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::get_aux_mask
//       Access: Published
//  Description: Converts the aux bitplanes of the
//               framebuffer into a RenderBuffer::Type.
////////////////////////////////////////////////////////////////////
int FrameBufferProperties::
get_aux_mask() const {
  int mask = 0;
  for (int i=0; i<_property[FBP_aux_rgba]; i++) {
    mask |= (RenderBuffer::T_aux_rgba_0 << i);
  }
  for (int i=0; i<_property[FBP_aux_hrgba]; i++) {
    mask |= (RenderBuffer::T_aux_hrgba_0 << i);
  }
  for (int i=0; i<_property[FBP_aux_float]; i++) {
    mask |= (RenderBuffer::T_aux_float_0 << i);
  }
  return mask;
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferProperties::get_buffer_mask
//       Access: Private
//  Description: Converts the non-aux bitplanes of the
//               framebuffer into a RenderBuffer::Type.
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
  if (_flags_specified != 0) {
    return true;
  }

  for (int i = 0; i < FBP_COUNT; ++i) {
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
  _flags_specified = FBF_all;

  for (int i = 0; i < FBP_COUNT; ++i) {
    _specified[i] = true;
  }
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
  if (_property[FBP_coverage_samples] > 0) {
    return false;
  }
  if (_property[FBP_back_buffers] > 0) {
    return false;
  }
  if ((_flags & FBF_indexed_color) != 0) {
    return false;
  }
  if ((_flags & FBF_force_hardware) != 0) {
    return false;
  }
  if ((_flags & FBF_force_software) != 0) {
    return false;
  }
  if ((_flags & FBF_srgb_color) != 0) {
    return false;
  }
  if ((_flags & FBF_float_color) != 0) {
    return false;
  }
  if ((_flags & FBF_float_depth) != 0) {
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
  for (int prop = FBP_depth_bits; prop <= FBP_accum_bits; ++prop) {
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

  if (!get_indexed_color() && !get_rgb_color()) {
    // Nonfunctioning window.
    return 0;
  }

  if ((reqs.get_rgb_color() && !get_rgb_color()) ||
      (reqs.get_indexed_color() && !get_indexed_color())) {
    // These properties are nonnegotiable.
    return 0;
  }

  int quality = 100000000;

  // Deduct for using the wrong kind of renderer (hardware or software).
  // Cost: 10,000,000

  if ((reqs._flags & FBF_force_hardware) > (_flags & FBF_force_hardware) ||
      (reqs._flags & FBF_force_software) > (_flags & FBF_force_software)) {
    quality -= 10000000;
  }

  // Deduct for missing depth, color, alpha, stencil, or accum.
  // Cost: 1,000,000

  for (int prop = FBP_depth_bits; prop <= FBP_accum_bits; ++prop) {
    if (reqs._property[prop] && _property[prop] == 0) {
      quality -= 1000000;
    }
  }

  // Deduct for missing aux bitplanes.
  // Cost: 100,000

  for (int prop = FBP_aux_rgba; prop <= FBP_aux_float; ++prop) {
    if (reqs._property[prop] > _property[prop]) {
      quality -= 100000;
    }
  }

  // Deduct for stereo not enabled.
  // Cost: 100,000

  if (reqs.get_stereo() && !get_stereo()) {
    quality -= 100000;
  }

  // Deduct for not being sRGB-capable.
  // Cost: 100,000

  if (reqs.get_srgb_color() && !get_srgb_color()) {
    quality -= 100000;
  }

  // Deduct for not having a floating-point format if we requested it.
  // Cost: 100,000

  if (reqs.get_float_color() && !get_float_color()) {
    quality -= 100000;
  }

  if (reqs.get_float_depth() && !get_float_depth()) {
    quality -= 100000;
  }

  // Deduct for insufficient back-buffers.
  // Cost: 100,000

  if (reqs._property[FBP_back_buffers] > _property[FBP_back_buffers]) {
    quality -= 100000;
  }

  // Deduct for lacking multisamples altogether.
  // Cost: 100,000
  if (reqs._property[FBP_multisamples] != 0 && _property[FBP_multisamples] == 0) {
    quality -= 100000;
  }

  // Deduct for not enough bits in depth, color, alpha, stencil, or accum.
  // Cost: 10,000

  for (int prop = FBP_depth_bits; prop <= FBP_accum_bits; ++prop) {
    if (_property[prop] != 0 && reqs._property[prop] > _property[prop]) {
      quality -= 10000;
    }
  }

  // deduct for insufficient multisamples.
  // Cost: 1,000

  if (_property[FBP_multisamples] != 0 &&
      reqs._property[FBP_multisamples] > _property[FBP_multisamples]) {
    quality -= 1000;
  }

  // Deduct for unrequested bitplanes.
  // Cost: 50

  for (int prop = FBP_depth_bits; prop <= FBP_accum_bits; ++prop) {
    if ((_property[prop]) && (reqs._property[prop] == 0)) {
      quality -= 50;
    }
  }
  for (int prop = FBP_aux_rgba; prop <= FBP_aux_float; ++prop) {
    int extra = _property[prop] > reqs._property[prop];
    if (extra > 0) {
      extra = min(extra, 3);
      quality -= extra*50;
    }
  }

  // Deduct for excessive resolution in any bitplane (unless we asked
  // for only 1 bit, which is the convention for any amount).

  // Cost: 50

  for (int prop = FBP_depth_bits; prop <= FBP_accum_bits; ++prop) {
    if (reqs._property[prop] > 1 &&
        _property[prop] > reqs._property[prop]) {
      quality -= 50;
    }
  }

  // Bonus for each depth bit.
  // Extra: 2 per bit.
  if (reqs._property[FBP_depth_bits] != 0) {
    quality += 2 * _property[FBP_depth_bits];
  }

  // Bonus for each multisample.
  // Extra: 2 per sample.
  if (reqs._property[FBP_multisamples] != 0) {
    quality += 2 * _property[FBP_multisamples];
  }

  // Bonus for each coverage sample.
  // Extra: 2 per sample.
  if (reqs._property[FBP_coverage_samples] != 0) {
    quality += 2 * _property[FBP_coverage_samples];
  }

  // Bonus for each color, alpha, stencil, and accum.
  // Extra: 1 per bit.
  for (int prop=FBP_color_bits; prop<=FBP_accum_bits; prop++) {
    if (reqs._property[prop] != 0) {
      quality += _property[prop];
    }
  }

  return quality;
}

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
      << "alter the hardware/software configuration in your Config.prc file.\n";
    return false;
  }

  if (get_force_software() < props.get_force_software()) {
    display_cat.error()
      << "The application requested a software renderer, but your OpenGL\n";
    display_cat.error()
      << "driver, " << renderer << ", is probably hardware-accelerated.\n";
    display_cat.error()
      << "If you want to allow hardware acceleration, then alter the\n";
    display_cat.error()
      << "hardware/software configuration in your Config.prc file.\n";
    return false;
  }

  return true;
}
