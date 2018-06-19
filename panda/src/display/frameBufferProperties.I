/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file frameBufferProperties.I
 * @author drose
 * @date 2003-01-27
 */

/**
 *
 */
INLINE bool FrameBufferProperties::
operator != (const FrameBufferProperties &other) const {
  return !operator == (other);
}

/**
 *
 */
INLINE bool FrameBufferProperties::
is_single_buffered() const {
  return (_property[FBP_back_buffers] == 0);
}

/**
 *
 */
INLINE bool FrameBufferProperties::
is_stereo() const {
  return (_flags & FBF_stereo) != 0;
}

/**
 *
 */
INLINE std::ostream &
operator << (std::ostream &out, const FrameBufferProperties &properties) {
  properties.output(out);
  return out;
}

/**
 *
 */
INLINE int FrameBufferProperties::
get_depth_bits() const {
  return _property[FBP_depth_bits];
}

/**
 *
 */
INLINE int FrameBufferProperties::
get_color_bits() const {
  return std::max(_property[FBP_color_bits],
             _property[FBP_red_bits] +
             _property[FBP_green_bits] +
             _property[FBP_blue_bits]);
}

/**
 *
 */
INLINE int FrameBufferProperties::
get_red_bits() const {
  return _property[FBP_red_bits];
}

/**
 *
 */
INLINE int FrameBufferProperties::
get_green_bits() const {
  return _property[FBP_green_bits];
}

/**
 *
 */
INLINE int FrameBufferProperties::
get_blue_bits() const {
  return _property[FBP_blue_bits];
}

/**
 *
 */
INLINE int FrameBufferProperties::
get_alpha_bits() const {
  return _property[FBP_alpha_bits];
}

/**
 *
 */
INLINE int FrameBufferProperties::
get_stencil_bits() const {
  return _property[FBP_stencil_bits];
}

/**
 *
 */
INLINE int FrameBufferProperties::
get_accum_bits() const {
  return _property[FBP_accum_bits];
}

/**
 *
 */
INLINE int FrameBufferProperties::
get_aux_rgba() const {
  return _property[FBP_aux_rgba];
}

/**
 *
 */
INLINE int FrameBufferProperties::
get_aux_hrgba() const {
  return _property[FBP_aux_hrgba];
}

/**
 *
 */
INLINE int FrameBufferProperties::
get_aux_float() const {
  return _property[FBP_aux_float];
}

/**
 *
 */
INLINE int FrameBufferProperties::
get_multisamples() const {
  return _property[FBP_multisamples];
}

/**
 * If coverage samples are specified, and there is hardware support, we use
 * coverage multisampling.
 */
INLINE int FrameBufferProperties::
get_coverage_samples() const {
  return _property[FBP_coverage_samples];
}

/**
 *
 */
INLINE int FrameBufferProperties::
get_back_buffers() const {
  return _property[FBP_back_buffers];
}

/**
 *
 */
INLINE bool FrameBufferProperties::
get_indexed_color() const {
  return (_flags & FBF_indexed_color) != 0;
}

/**
 *
 */
INLINE bool FrameBufferProperties::
get_rgb_color() const {
  return (_flags & FBF_rgb_color) != 0;
}

/**
 *
 */
INLINE bool FrameBufferProperties::
get_stereo() const {
  return (_flags & FBF_stereo) != 0;
}

/**
 *
 */
INLINE bool FrameBufferProperties::
get_force_hardware() const {
  return (_flags & FBF_force_hardware) != 0;
}

/**
 *
 */
INLINE bool FrameBufferProperties::
get_force_software() const {
  return (_flags & FBF_force_software) != 0;
}

/**
 *
 */
INLINE bool FrameBufferProperties::
get_srgb_color() const {
  return (_flags & FBF_srgb_color) != 0;
}

/**
 *
 */
INLINE bool FrameBufferProperties::
get_float_color() const {
  return (_flags & FBF_float_color) != 0;
}

/**
 *
 */
INLINE bool FrameBufferProperties::
get_float_depth() const {
  return (_flags & FBF_float_depth) != 0;
}

/**
 *
 */
INLINE void FrameBufferProperties::
set_depth_bits(int n) {
  _property[FBP_depth_bits] = n;
  _specified |= (1 << FBP_depth_bits);
}

/**
 * Sets the number of requested color bits as a single number that represents
 * the sum of the individual numbers of red, green and blue bits.  Panda won't
 * care how the individual bits are divided up.
 *
 * See also set_rgba_bits, which allows you to specify requirements for the
 * individual components.
 */
INLINE void FrameBufferProperties::
set_color_bits(int n) {
  _property[FBP_color_bits] = n;
  _specified |= (1 << FBP_color_bits);
}

/**
 * Convenience method for setting the red, green, blue and alpha bits in one
 * go.
 */
INLINE void FrameBufferProperties::
set_rgba_bits(int r, int g, int b, int a) {
  _property[FBP_red_bits] = r;
  _property[FBP_green_bits] = g;
  _property[FBP_blue_bits] = b;
  _property[FBP_alpha_bits] = a;
  _property[FBP_color_bits] = r + g + b;
  _specified |= (1 << FBP_color_bits) | (1 << FBP_red_bits) |
                (1 << FBP_green_bits) | (1 << FBP_blue_bits) |
                (1 << FBP_alpha_bits);
}

/**
 *
 */
INLINE void FrameBufferProperties::
set_red_bits(int n) {
  _property[FBP_red_bits] = n;
  _specified |= (1 << FBP_red_bits);
}

/**
 *
 */
INLINE void FrameBufferProperties::
set_green_bits(int n) {
  _property[FBP_green_bits] = n;
  _specified |= (1 << FBP_green_bits);
}

/**
 *
 */
INLINE void FrameBufferProperties::
set_blue_bits(int n) {
  _property[FBP_blue_bits] = n;
  _specified |= (1 << FBP_blue_bits);
}

/**
 *
 */
INLINE void FrameBufferProperties::
set_alpha_bits(int n) {
  _property[FBP_alpha_bits] = n;
  _specified |= (1 << FBP_alpha_bits);
}

/**
 *
 */
INLINE void FrameBufferProperties::
set_stencil_bits(int n) {
  _property[FBP_stencil_bits] = n;
  _specified |= (1 << FBP_stencil_bits);
}

/**
 *
 */
INLINE void FrameBufferProperties::
set_accum_bits(int n) {
  _property[FBP_accum_bits] = n;
  _specified |= (1 << FBP_accum_bits);
}

/**
 *
 */
INLINE void FrameBufferProperties::
set_aux_rgba(int n) {
  nassertv(n <= 4);
  _property[FBP_aux_rgba] = n;
  _specified |= (1 << FBP_aux_rgba);
}

/**
 *
 */
INLINE void FrameBufferProperties::
set_aux_hrgba(int n) {
  nassertv(n <= 4);
  _property[FBP_aux_hrgba] = n;
  _specified |= (1 << FBP_aux_hrgba);
}

/**
 *
 */
INLINE void FrameBufferProperties::
set_aux_float(int n) {
  nassertv(n <= 4);
  _property[FBP_aux_float] = n;
  _specified |= (1 << FBP_aux_float);
}

/**
 *
 */
INLINE void FrameBufferProperties::
set_multisamples(int n) {
  _property[FBP_multisamples] = n;
  _specified |= (1 << FBP_multisamples);
}

/**
 * If coverage samples are specified, and there is hardware support, we use
 * coverage multisampling
 */
INLINE void FrameBufferProperties::
set_coverage_samples(int n) {
  _property[FBP_coverage_samples] = n;
  _specified |= (1 << FBP_coverage_samples);
}

/**
 *
 */
INLINE void FrameBufferProperties::
set_back_buffers(int n) {
  _property[FBP_back_buffers] = n;
  _specified |= (1 << FBP_back_buffers);
}

/**
 *
 */
INLINE void FrameBufferProperties::
set_indexed_color(bool n) {
  if (n) {
    _flags |= FBF_indexed_color;
  } else {
    _flags &= ~FBF_indexed_color;
  }
  _flags_specified |= FBF_indexed_color;
}

/**
 *
 */
INLINE void FrameBufferProperties::
set_rgb_color(bool n) {
  if (n) {
    _flags |= FBF_rgb_color;
  } else {
    _flags &= ~FBF_rgb_color;
  }
  _flags_specified |= FBF_rgb_color;
}

/**
 *
 */
INLINE void FrameBufferProperties::
set_stereo(bool n) {
  if (n) {
    _flags |= FBF_stereo;
  } else {
    _flags &= ~FBF_stereo;
  }
  _flags_specified |= FBF_stereo;
}

/**
 *
 */
INLINE void FrameBufferProperties::
set_force_hardware(bool n) {
  if (n) {
    _flags |= FBF_force_hardware;
  } else {
    _flags &= ~FBF_force_hardware;
  }
  _flags_specified |= FBF_force_hardware;
}

/**
 *
 */
INLINE void FrameBufferProperties::
set_force_software(bool n) {
  if (n) {
    _flags |= FBF_force_software;
  } else {
    _flags &= ~FBF_force_software;
  }
  _flags_specified |= FBF_force_software;
}

/**
 *
 */
INLINE void FrameBufferProperties::
set_srgb_color(bool n) {
  if (n) {
    _flags |= FBF_srgb_color;
  } else {
    _flags &= ~FBF_srgb_color;
  }
  _flags_specified |= FBF_srgb_color;
}

/**
 *
 */
INLINE void FrameBufferProperties::
set_float_color(bool n) {
  if (n) {
    _flags |= FBF_float_color;
  } else {
    _flags &= ~FBF_float_color;
  }
  _flags_specified |= FBF_float_color;
}

/**
 *
 */
INLINE void FrameBufferProperties::
set_float_depth(bool n) {
  if (n) {
    _flags |= FBF_float_depth;
  } else {
    _flags &= ~FBF_float_depth;
  }
  _flags_specified |= FBF_float_depth;
}
