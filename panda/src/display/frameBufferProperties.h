// Filename: frameBufferProperties.h
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

#ifndef FRAMEBUFFERPROPERTIES_H
#define FRAMEBUFFERPROPERTIES_H

#include "pandabase.h"

////////////////////////////////////////////////////////////////////
//       Class : FrameBufferProperties
// Description : A container for the various kinds of properties we
//               might ask to have on a graphics frameBuffer before we
//               create a GSG.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA FrameBufferProperties {
PUBLISHED:
  FrameBufferProperties();
  INLINE FrameBufferProperties(const FrameBufferProperties &copy);
  void operator = (const FrameBufferProperties &copy);
  INLINE ~FrameBufferProperties();

  bool operator == (const FrameBufferProperties &other) const;
  INLINE bool operator != (const FrameBufferProperties &other) const;

  enum FrameBufferMode {
    FM_rgba =          0x0000,
    FM_rgb =           0x0000,
    FM_index =         0x0001,
    FM_single_buffer = 0x0000,
    FM_double_buffer = 0x0002,
    FM_triple_buffer = 0x0004,
    FM_buffer        = 0x0006,  // == (FM_single_buffer | FM_double_buffer | FM_triple_buffer)
    FM_accum =         0x0008,
    FM_alpha =         0x0010,
    FM_depth =         0x0020,
    FM_stencil =       0x0040,
    FM_multisample =   0x0080,
    FM_stereo =        0x0100,
    FM_luminance =     0x0200,
  };

  void clear();
  INLINE bool is_any_specified() const;

  INLINE void set_frame_buffer_mode(int frameBuffer_mode);
  INLINE int get_frame_buffer_mode() const;
  INLINE bool has_frame_buffer_mode() const;
  INLINE void clear_frame_buffer_mode();

  INLINE void set_depth_bits(int depth_bits);
  INLINE int get_depth_bits() const;
  INLINE bool has_depth_bits() const;
  INLINE void clear_depth_bits();

  INLINE void set_color_bits(int color_bits);
  INLINE int get_color_bits() const;
  INLINE bool has_color_bits() const;
  INLINE void clear_color_bits();

  void add_properties(const FrameBufferProperties &other);

  void output(ostream &out) const;
  
private:
  // This bitmask indicates which of the parameters in the properties
  // structure have been filled in by the user, and which remain
  // unspecified.
  enum Specified {
    S_frame_buffer_mode = 0x0200,
    S_depth_bits        = 0x0400,
    S_color_bits        = 0x0800,
  };

  // This bitmask represents the true/false settings for various
  // boolean flags (assuming the corresponding S_* bit has been set,
  // above).
  /*
  enum Flags {
  };
  */

  int _specified;
  int _flags;
  int _frame_buffer_mode;
  int _depth_bits;
  int _color_bits;
};

INLINE ostream &operator << (ostream &out, const FrameBufferProperties &properties);

#include "frameBufferProperties.I"

#endif
