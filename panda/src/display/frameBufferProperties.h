// Filename: frameBufferProperties.h
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

#ifndef FRAMEBUFFERPROPERTIES_H
#define FRAMEBUFFERPROPERTIES_H

#include "pandabase.h"
#include "pnotify.h"

////////////////////////////////////////////////////////////////////
//       Class : FrameBufferProperties
// Description : A container for the various kinds of properties we
//               might ask to have on a graphics frameBuffer before we
//               create a GSG.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_DISPLAY FrameBufferProperties {

private:
  enum FrameBufferProperty {
    // This section has to start with "depth" and end with "accum"
    FBP_depth_bits,
    FBP_color_bits,
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
    FBP_indexed_color,
    FBP_rgb_color,
    FBP_stereo,
    FBP_force_hardware,
    FBP_force_software,
    
    // This is a sentinel value.
    FBP_COUNT
  };
  
  int _property[FBP_COUNT];
  int _specified[FBP_COUNT];

PUBLISHED:

  // Individual queries.
  INLINE int get_depth_bits() const;
  INLINE int get_color_bits() const;
  INLINE int get_alpha_bits() const;
  INLINE int get_stencil_bits() const;
  INLINE int get_accum_bits() const;
  INLINE int get_aux_rgba() const;
  INLINE int get_aux_hrgba() const;
  INLINE int get_aux_float() const;
  INLINE int get_multisamples() const;
  INLINE int get_coverage_samples() const;
  INLINE int get_back_buffers() const;
  INLINE int get_indexed_color() const;
  INLINE int get_rgb_color() const;
  INLINE int get_stereo() const;
  INLINE int get_force_hardware() const;
  INLINE int get_force_software() const;

  // Individual assigners.
  INLINE void set_depth_bits(int n);
  INLINE void set_color_bits(int n);
  INLINE void set_alpha_bits(int n);
  INLINE void set_stencil_bits(int n);
  INLINE void set_accum_bits(int n);
  INLINE void set_aux_rgba(int n);
  INLINE void set_aux_hrgba(int n);
  INLINE void set_aux_float(int n);
  INLINE void set_multisamples(int n);
  INLINE void set_coverage_samples(int n);
  INLINE void set_back_buffers(int n);
  INLINE void set_indexed_color(int n);
  INLINE void set_rgb_color(int n);
  INLINE void set_stereo(int n);
  INLINE void set_force_hardware(int n);
  INLINE void set_force_software(int n);

  // Other.

  FrameBufferProperties();
  INLINE FrameBufferProperties(const FrameBufferProperties &copy);
  INLINE ~FrameBufferProperties();
  void operator = (const FrameBufferProperties &copy);
  static const FrameBufferProperties &get_default();
  bool operator == (const FrameBufferProperties &other) const;
  INLINE bool operator != (const FrameBufferProperties &other) const;

  void clear();
  void set_all_specified();
  bool subsumes(const FrameBufferProperties &other) const;
  void add_properties(const FrameBufferProperties &other);
  void output(ostream &out) const;
  void set_one_bit_per_channel();
  
  bool is_stereo() const;
  bool is_single_buffered() const;
  int get_quality(const FrameBufferProperties &reqs) const;
  bool is_any_specified() const;
  bool is_basic() const;
  int get_aux_mask() const;
  int get_buffer_mask() const;
  bool verify_hardware_software(const FrameBufferProperties &props, const string &renderer) const;
};

INLINE ostream &operator << (ostream &out, const FrameBufferProperties &properties);

#include "frameBufferProperties.I"

#endif
