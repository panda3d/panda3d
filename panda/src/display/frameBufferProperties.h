/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file frameBufferProperties.h
 * @author drose
 * @date 2003-01-27
 */

#ifndef FRAMEBUFFERPROPERTIES_H
#define FRAMEBUFFERPROPERTIES_H

#include "pandabase.h"
#include "pnotify.h"

class Texture;

/**
 * A container for the various kinds of properties we might ask to have on a
 * graphics frameBuffer before we create a GSG.
 */
class EXPCL_PANDA_DISPLAY FrameBufferProperties {

private:
  enum FrameBufferProperty {
    // This section has to start with "depth" and end with "accum"
    FBP_depth_bits,
    FBP_color_bits,
    FBP_red_bits,
    FBP_green_bits,
    FBP_blue_bits,
    FBP_alpha_bits,
    FBP_stencil_bits,
    FBP_accum_bits,

    // This section has to start with "rgba" and end with "float"
    FBP_aux_rgba,
    FBP_aux_hrgba,
    FBP_aux_float,

    // This section can be in any order.
    FBP_multisamples,
    FBP_coverage_samples,
    FBP_back_buffers,

    // This is a sentinel value.
    FBP_COUNT
  };

  enum FrameBufferFlag {
    FBF_indexed_color  = 0x001,
    FBF_rgb_color      = 0x002,
    FBF_stereo         = 0x004,
    FBF_force_hardware = 0x008,
    FBF_force_software = 0x010,
    FBF_srgb_color     = 0x020,
    FBF_float_color    = 0x040,
    FBF_float_depth    = 0x080,
    FBF_all            = 0x100-1,
  };

  int _property[FBP_COUNT] = {0};
  int _specified = 0;

  int _flags = 0;
  int _flags_specified = 0;

PUBLISHED:

  // Individual queries.
  INLINE int get_depth_bits() const;
  INLINE int get_color_bits() const;
  INLINE int get_red_bits() const;
  INLINE int get_green_bits() const;
  INLINE int get_blue_bits() const;
  INLINE int get_alpha_bits() const;
  INLINE int get_stencil_bits() const;
  INLINE int get_accum_bits() const;
  INLINE int get_aux_rgba() const;
  INLINE int get_aux_hrgba() const;
  INLINE int get_aux_float() const;
  INLINE int get_multisamples() const;
  INLINE int get_coverage_samples() const;
  INLINE int get_back_buffers() const;
  INLINE bool get_indexed_color() const;
  INLINE bool get_rgb_color() const;
  INLINE bool get_stereo() const;
  INLINE bool get_force_hardware() const;
  INLINE bool get_force_software() const;
  INLINE bool get_srgb_color() const;
  INLINE bool get_float_color() const;
  INLINE bool get_float_depth() const;

  // Individual assigners.
  INLINE void set_depth_bits(int n);
  INLINE void set_color_bits(int n);
  INLINE void set_rgba_bits(int r, int g, int b, int a);
  INLINE void set_red_bits(int n);
  INLINE void set_green_bits(int n);
  INLINE void set_blue_bits(int n);
  INLINE void set_alpha_bits(int n);
  INLINE void set_stencil_bits(int n);
  INLINE void set_accum_bits(int n);
  INLINE void set_aux_rgba(int n);
  INLINE void set_aux_hrgba(int n);
  INLINE void set_aux_float(int n);
  INLINE void set_multisamples(int n);
  INLINE void set_coverage_samples(int n);
  INLINE void set_back_buffers(int n);
  INLINE void set_indexed_color(bool n);
  INLINE void set_rgb_color(bool n);
  INLINE void set_stereo(bool n);
  INLINE void set_force_hardware(bool n);
  INLINE void set_force_software(bool n);
  INLINE void set_srgb_color(bool n);
  INLINE void set_float_color(bool n);
  INLINE void set_float_depth(bool n);

  MAKE_PROPERTY(depth_bits, get_depth_bits, set_depth_bits);
  MAKE_PROPERTY(color_bits, get_color_bits, set_color_bits);
  MAKE_PROPERTY(red_bits, get_red_bits, set_red_bits);
  MAKE_PROPERTY(green_bits, get_green_bits, set_green_bits);
  MAKE_PROPERTY(blue_bits, get_blue_bits, set_blue_bits);
  MAKE_PROPERTY(alpha_bits, get_alpha_bits, set_alpha_bits);
  MAKE_PROPERTY(stencil_bits, get_stencil_bits, set_stencil_bits);
  MAKE_PROPERTY(accum_bits, get_accum_bits, set_accum_bits);
  MAKE_PROPERTY(aux_rgba, get_aux_rgba, set_aux_rgba);
  MAKE_PROPERTY(aux_hrgba, get_aux_hrgba, set_aux_hrgba);
  MAKE_PROPERTY(aux_float, get_aux_float, set_aux_float);
  MAKE_PROPERTY(multisamples, get_multisamples, set_multisamples);
  MAKE_PROPERTY(coverage_samples, get_coverage_samples, set_coverage_samples);
  MAKE_PROPERTY(back_buffers, get_back_buffers, set_back_buffers);
  MAKE_PROPERTY(indexed_color, get_indexed_color, set_indexed_color);
  MAKE_PROPERTY(rgb_color, get_rgb_color, set_rgb_color);
  MAKE_PROPERTY(stereo, get_stereo, set_stereo);
  MAKE_PROPERTY(force_hardware, get_force_hardware, set_force_hardware);
  MAKE_PROPERTY(force_software, get_force_software, set_force_software);
  MAKE_PROPERTY(srgb_color, get_srgb_color, set_srgb_color);
  MAKE_PROPERTY(float_color, get_float_color, set_float_color);
  MAKE_PROPERTY(float_depth, get_float_depth, set_float_depth);

  // Other.

  constexpr FrameBufferProperties() = default;

  static const FrameBufferProperties &get_default();
  bool operator == (const FrameBufferProperties &other) const;
  INLINE bool operator != (const FrameBufferProperties &other) const;

  void clear();
  void set_all_specified();
  bool subsumes(const FrameBufferProperties &other) const;
  void add_properties(const FrameBufferProperties &other);
  void output(std::ostream &out) const;
  void set_one_bit_per_channel();

  INLINE bool is_stereo() const;
  INLINE bool is_single_buffered() const;
  int get_quality(const FrameBufferProperties &reqs) const;
  bool is_any_specified() const;
  bool is_basic() const;
  int get_aux_mask() const;
  int get_buffer_mask() const;
  bool verify_hardware_software(const FrameBufferProperties &props, const std::string &renderer) const;

  bool setup_color_texture(Texture *tex) const;
  bool setup_depth_texture(Texture *tex) const;
};

INLINE std::ostream &operator << (std::ostream &out, const FrameBufferProperties &properties);

#include "frameBufferProperties.I"

#endif
