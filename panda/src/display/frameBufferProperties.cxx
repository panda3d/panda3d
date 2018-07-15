/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file frameBufferProperties.cxx
 * @author drose
 * @date 2003-01-27
 */

#include "frameBufferProperties.h"
#include "string_utils.h"
#include "renderBuffer.h"
#include "config_display.h"
#include "texture.h"

/**
 * Returns true if this set of properties makes strictly greater or equal
 * demands of the framebuffer than the other set of framebuffer properties.
 */
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

/**
 * Returns a FrameBufferProperties structure with all of the default values
 * filled in according to the user's config file.
 */
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
    display_cat.error() << "  red-bits N\n";
    display_cat.error() << "  green-bits N\n";
    display_cat.error() << "  blue-bits N\n";
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
  switch (color_bits.size()) {
  case 0:
    break;
  case 1:
    default_props.set_color_bits(color_bits[0]);
    default_props.set_red_bits(1);
    default_props.set_green_bits(1);
    default_props.set_blue_bits(1);
    break;
  case 3:
    default_props.set_color_bits(color_bits[0] + color_bits[1] + color_bits[2]);
    default_props.set_red_bits(color_bits[0]);
    default_props.set_green_bits(color_bits[1]);
    default_props.set_blue_bits(color_bits[2]);
    break;
  default:
    default_props.set_color_bits(color_bits[0]);
    display_cat.error()
      << "Configuration variable color-bits takes either 1 or 3 values, not "
      << color_bits.size() << "\n";
    break;
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

/**
 *
 */
bool FrameBufferProperties::
operator == (const FrameBufferProperties &other) const {
  if ((_flags & _flags_specified) != (other._flags & other._flags_specified)) {
    return false;
  }

  if (_specified != other._specified) {
    return false;
  }

  for (int i = 0; i < FBP_COUNT; ++i) {
    if (_property[i] != other._property[i]) {
      return false;
    }
  }

  return true;
}

/**
 * Unsets all properties that have been specified so far, and resets the
 * FrameBufferProperties structure to its initial empty state.
 */
void FrameBufferProperties::
clear() {
  _flags = 0;
  _flags_specified = 0;

  for (int i = 0; i < FBP_COUNT; ++i) {
    _property[i] = 0;
  }
  _specified = 0;
}

/**
 * Sets any properties that are explicitly specified in other on this object.
 * Leaves other properties unchanged.
 */
void FrameBufferProperties::
add_properties(const FrameBufferProperties &other) {
  _flags &= ~other._flags_specified;
  _flags |= other._flags & other._flags_specified;

  for (int i = 0; i < FBP_COUNT; ++i) {
    if (other._specified & (1 << i)) {
      _property[i] = other._property[i];
      _specified |= (1 << i);
    }
  }
}

/**
 * Generates a string representation.
 */
void FrameBufferProperties::
output(std::ostream &out) const {
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
  if (_property[FBP_red_bits] > 0) {
    out << "red_bits=" << _property[FBP_red_bits] << " ";
  }
  if (_property[FBP_green_bits] > 0) {
    out << "green_bits=" << _property[FBP_green_bits] << " ";
  }
  if (_property[FBP_blue_bits] > 0) {
    out << "blue_bits=" << _property[FBP_blue_bits] << " ";
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

/**
 * Converts the aux bitplanes of the framebuffer into a RenderBuffer::Type.
 */
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

/**
 * Converts the non-aux bitplanes of the framebuffer into a
 * RenderBuffer::Type.
 */
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

/**
 * Returns true if any properties have been specified, false otherwise.
 */
bool FrameBufferProperties::
is_any_specified() const {
  return (_flags_specified | _specified) != 0;
}

/**
 * Marks all bits as having been specified.
 */
void FrameBufferProperties::
set_all_specified() {
  _flags_specified = FBF_all;
  _specified = (1 << FBP_COUNT) - 1;
}

/**
 * Returns true if the properties are extremely basic.  The following count as
 * basic: rgb or rgba, depth.  If anything else is specified, the properties
 * are non-basic.
 */
bool FrameBufferProperties::
is_basic() const {
  if (_property[FBP_depth_bits] > 1) {
    return false;
  }
  if (_property[FBP_color_bits] > 1) {
    return false;
  }
  if (_property[FBP_red_bits] > 1) {
    return false;
  }
  if (_property[FBP_green_bits] > 1) {
    return false;
  }
  if (_property[FBP_blue_bits] > 1) {
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

/**
 * If any of the depth, color, alpha, accum, or stencil properties is set to
 * more than one, then they are reduced to one.
 */
void FrameBufferProperties::
set_one_bit_per_channel() {
  for (int prop = FBP_depth_bits; prop <= FBP_accum_bits; ++prop) {
    if (_property[prop] > 1) {
      _property[prop] = 1;
    }
  }
}

/**
 * Assumes that these properties are a description of a window.
 *
 * Measures how well this window satisfies a specified set of requirements.  A
 * higher quality number means that more requirements were satisfied.  A
 * quality of zero means that the window is unsuitable.
 *
 * The routine deducts a lot if the window fails to provide a requested
 * feature.  It deducts less if the window provides a feature, but at a
 * degraded level of functionality (ie, the user asks for rgba8, color, but
 * the window only provides rgba4).  The routine also deducts a small amount
 * for unnecessary features.  For example, if the window has an accumulation
 * buffer when one is not requested will reduce quality slightly.  Maximum
 * quality is obtained when the window exactly matches the request.
 *
 * If you want to know whether the window satisfies all of the requirements,
 * use the "subsumes" function.
 */
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

  // Deduct for using the wrong kind of renderer (hardware or software). Cost:
  // 10,000,000

  if ((reqs._flags & FBF_force_hardware) > (_flags & FBF_force_hardware) ||
      (reqs._flags & FBF_force_software) > (_flags & FBF_force_software)) {
    quality -= 10000000;
  }

  // Deduct for software-only renderers in absence of a special request.
  // Cost: 2,000,000

  if (get_force_software() && !reqs.get_force_software()) {
    quality -= 2000000;
  }

  // Deduct for missing depth, color, alpha, stencil, or accum.  Cost:
  // 1,000,000

  for (int prop = FBP_depth_bits; prop <= FBP_accum_bits; ++prop) {
    if (reqs._property[prop] && _property[prop] == 0) {
      quality -= 1000000;
    }
  }

  // Deduct for missing aux bitplanes.  Cost: 100,000

  for (int prop = FBP_aux_rgba; prop <= FBP_aux_float; ++prop) {
    if (reqs._property[prop] > _property[prop]) {
      quality -= 100000;
    }
  }

  // Deduct for stereo not enabled.  Cost: 100,000

  if (reqs.get_stereo() && !get_stereo()) {
    quality -= 100000;
  }

  // Deduct for not being sRGB-capable.  Cost: 100,000

  if (reqs.get_srgb_color() && !get_srgb_color()) {
    quality -= 100000;
  }

  // Deduct for not having a floating-point format if we requested it.  Cost:
  // 100,000

  if (reqs.get_float_color() && !get_float_color()) {
    quality -= 100000;
  }

  if (reqs.get_float_depth() && !get_float_depth()) {
    quality -= 100000;
  }

  // Deduct for insufficient back-buffers.  Cost: 100,000

  if (reqs._property[FBP_back_buffers] > _property[FBP_back_buffers]) {
    quality -= 100000;
  }

  // Deduct for lacking multisamples altogether.  Cost: 100,000
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

  // deduct for insufficient multisamples.  Cost: 1,000

  if (_property[FBP_multisamples] != 0 &&
      reqs._property[FBP_multisamples] > _property[FBP_multisamples]) {
    quality -= 1000;
  }

  // Deduct for unrequested bitplanes.  Cost: 50

  for (int prop = FBP_depth_bits; prop <= FBP_accum_bits; ++prop) {
    if ((_property[prop]) && (reqs._property[prop] == 0)) {
      quality -= 50;
    }
  }
  for (int prop = FBP_aux_rgba; prop <= FBP_aux_float; ++prop) {
    int extra = _property[prop] > reqs._property[prop];
    if (extra > 0) {
      extra = std::min(extra, 3);
      quality -= extra*50;
    }
  }

  // Deduct for excessive resolution in any bitplane (unless we asked for only
  // 1 bit, which is the convention for any amount).

  // Cost: 50

  for (int prop = FBP_depth_bits; prop <= FBP_accum_bits; ++prop) {
    if (reqs._property[prop] > 1 &&
        _property[prop] > reqs._property[prop]) {
      quality -= 50;
    }
  }

  // However, deduct for color bits above 24, if we are requesting only 1.
  // This is to prevent choosing a 64-bit color mode in NVIDIA cards that
  // is linear and therefore causes the gamma to be off in non-sRGB pipelines.
  if (reqs._property[FBP_color_bits] <= 3 && _property[FBP_color_bits] > 24) {
    quality -= 100;
  }

  // Bonus for each depth bit.  Extra: 8 per bit.
  // Please note that the Intel Windows driver only gives extra depth in
  // combination with a stencil buffer, so we need 8 extra depth bits to
  // outweigh the penalty of 50 for the unwanted stencil buffer, otherwise we
  // will end up only getting 16-bit depth.
  if (reqs._property[FBP_depth_bits] != 0) {
    quality += 8 * _property[FBP_depth_bits];
  }

  // Bonus for each multisample.  Extra: 2 per sample.
  if (reqs._property[FBP_multisamples] != 0) {
    quality += 2 * _property[FBP_multisamples];
  }

  // Bonus for each coverage sample.  Extra: 2 per sample.
  if (reqs._property[FBP_coverage_samples] != 0) {
    quality += 2 * _property[FBP_coverage_samples];
  }

  // Bonus for each color, alpha, stencil, and accum.  Extra: 1 per bit.
  for (int prop=FBP_color_bits; prop<=FBP_accum_bits; prop++) {
    if (reqs._property[prop] != 0) {
      quality += _property[prop];
    }
  }

  return quality;
}

/**
 * Validates that the properties represent the desired kind of renderer
 * (hardware or software).  If not, prints out an error message and returns
 * false.
 */
bool FrameBufferProperties::
verify_hardware_software(const FrameBufferProperties &props, const std::string &renderer) const {

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

/**
 * Sets the texture up for render-to-texture matching these framebuffer
 * properties.
 *
 * Returns true if there was a format that had enough bits, false otherwise.
 * Of course, this is no guarantee that a particular graphics back-end
 * supports rendering to textures of that format.
 */
bool FrameBufferProperties::
setup_color_texture(Texture *tex) const {
  // Note by rdb: I'm not entirely happy about this system.  I'd eventually
  // like to move to a system in which framebuffer color formats and texture
  // formats are unified (like in Direct3D and OpenGL) and where a table such
  // as the below one would be generated dynamically by the GSG to reflect the
  // formats that are supported for render-to-texture.

  static const int num_formats = 17;
  static const struct {
    unsigned char color_bits, red_bits, green_bits, blue_bits, alpha_bits;
    bool has_float;
    Texture::Format format;
  } formats[num_formats] = {
    {  1,  1,  0,  0,  0, false, Texture::F_red },
    {  1,  1,  1,  0,  0, false, Texture::F_rg },
    {  1,  1,  1,  1,  0, false, Texture::F_rgb },
    {  1,  1,  1,  1,  1, false, Texture::F_rgba },
    {  8,  8,  0,  0,  0, false, Texture::F_red },
    { 16,  8,  8,  0,  0, false, Texture::F_rg },
    { 24,  8,  8,  8,  0, false, Texture::F_rgb8 },
    { 32,  8,  8,  8,  8, false, Texture::F_rgba8 },
    { 16, 16,  0,  0,  0,  true, Texture::F_r16 },
    { 32, 16, 16,  0,  0,  true, Texture::F_rg16 },
    { 32, 11, 11, 10,  0,  true, Texture::F_r11_g11_b10 },
    { 48, 16, 16, 16,  0,  true, Texture::F_rgb16 },
    { 48, 16, 16, 16, 16,  true, Texture::F_rgba16 },
    { 32, 32,  0,  0,  0,  true, Texture::F_r32 },
    { 64, 32, 32,  0,  0,  true, Texture::F_rg32 },
    { 96, 32, 32, 32,  0,  true, Texture::F_rgb32 },
    { 96, 32, 32, 32, 32,  true, Texture::F_rgba32 },
  };

  if (get_srgb_color()) {
    // These are the only sRGB formats.  Deal with it.
    if (get_alpha_bits() == 0) {
      tex->set_format(Texture::F_srgb);
    } else {
      tex->set_format(Texture::F_srgb_alpha);
    }

    return (get_color_bits() <= 24 &&
            get_red_bits() <= 8 &&
            get_green_bits() <= 8 &&
            get_blue_bits() <= 8 &&
            get_alpha_bits() <= 8);

  } else {
    if (get_float_color()) {
      tex->set_component_type(Texture::T_float);
    }

    for (int i = 0; i < num_formats; ++i) {
      if (get_color_bits() <= (int)formats[i].color_bits &&
          get_red_bits() <= (int)formats[i].red_bits &&
          get_green_bits() <= (int)formats[i].green_bits &&
          get_blue_bits() <= (int)formats[i].blue_bits &&
          get_alpha_bits() <= (int)formats[i].alpha_bits &&
          get_float_color() <= formats[i].has_float) {

        tex->set_format(formats[i].format);
        return true;
      }
    }

    // Can't get requested bits.  Choose a generic format and return.
    tex->set_format((get_alpha_bits() == 0) ? Texture::F_rgb : Texture::F_rgba);
    return false;
  }
}

/**
 * Sets the texture up for render-to-texture matching these framebuffer
 * properties.
 *
 * Returns true if there was a format that had enough bits, false otherwise.
 * Of course, this is no guarantee that a particular graphics back-end
 * supports rendering to textures of that format.
 */
bool FrameBufferProperties::
setup_depth_texture(Texture *tex) const {
  if (get_float_depth()) {
    tex->set_component_type(Texture::T_float);
    tex->set_format(Texture::F_depth_component32);
    return (get_depth_bits() <= 32);

  } else if (get_depth_bits() <= 1) {
    tex->set_format(Texture::F_depth_component);
    return true;

  } else if (get_depth_bits() <= 16) {
    tex->set_format(Texture::F_depth_component16);
    return true;

  } else if (get_depth_bits() <= 24) {
    tex->set_format(Texture::F_depth_component24);
    return true;

  } else if (get_depth_bits() <= 32) {
    tex->set_format(Texture::F_depth_component32);
    return true;
  }

  tex->set_format(Texture::F_depth_component);
  return false;
}
