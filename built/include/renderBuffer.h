/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file renderBuffer.h
 * @author drose
 * @date 1999-02-02
 */

#ifndef RENDERBUFFER_H
#define RENDERBUFFER_H

#include "pandabase.h"

class GraphicsStateGuardian;

/**
 * A RenderBuffer is an arbitrary subset of the various layers (depth buffer,
 * color buffer, etc.) of a drawing region.  It consists of a
 * GraphicsStateGuardian pointer, along with a bitmask of the layers we're
 * interested in.
 */
class EXPCL_PANDA_DISPLAY RenderBuffer {
public:
  enum Type {
    T_aux_rgba_0       = 0x00000001,
    T_aux_rgba_1       = 0x00000002,
    T_aux_rgba_2       = 0x00000004,
    T_aux_rgba_3       = 0x00000008,
    T_aux_rgba_ALL     = 0x0000000F,

    T_aux_hrgba_0      = 0x00000010, // These can't really be implemented until
    T_aux_hrgba_1      = 0x00000020, // we have support for hrgba textures.
    T_aux_hrgba_2      = 0x00000040, // I've just added the bits for the future.
    T_aux_hrgba_3      = 0x00000080,
    T_aux_hrgba_ALL    = 0x000000F0,

    T_aux_float_0      = 0x00000100, // These can't really be implemented until
    T_aux_float_1      = 0x00000200, // we have support for float textures.
    T_aux_float_2      = 0x00000400, // I've just added the bits for the future.
    T_aux_float_3      = 0x00000800,
    T_aux_float_ALL    = 0x00000F00,

    T_aux_undef_0      = 0x00001000,
    T_aux_undef_1      = 0x00002000,
    T_aux_undef_2      = 0x00004000,
    T_aux_undef_3      = 0x00008000,

    T_aux              = 0x0000FFFF,

    T_front_left       = 0x00010000,
    T_back_left        = 0x00020000,
    T_front_right      = 0x00040000,
    T_back_right       = 0x00080000,

    T_front            = 0x00050000,
    T_back             = 0x000a0000,
    T_left             = 0x00030000,
    T_right            = 0x000c0000,

    T_color            = 0x000F0000,

    T_depth            = 0x00100000,
    T_stencil          = 0x00200000,
    T_accum            = 0x00400000,
  };


  RenderBuffer(GraphicsStateGuardian *gsg, int buffer_type)
    : _gsg(gsg), _buffer_type(buffer_type) { }

  GraphicsStateGuardian *_gsg;
  int _buffer_type;
};

#endif
