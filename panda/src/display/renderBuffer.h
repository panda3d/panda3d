// Filename: renderBuffer.h
// Created by:  drose (02Feb99)
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

#ifndef RENDERBUFFER_H
#define RENDERBUFFER_H

#include "pandabase.h"

class GraphicsStateGuardian;

////////////////////////////////////////////////////////////////////
//       Class : RenderBuffer
// Description : A RenderBuffer is an arbitrary subset of the various
//               layers (depth buffer, color buffer, etc.) of a
//               drawing region.  It consists of a
//               GraphicsStateGuardian pointer, along with a bitmask
//               of the layers we're interested in.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA RenderBuffer {
public:
  enum Type {
    T_aux_rgba_0       = 0x00000001,
    T_aux_rgba_1       = 0x00000002,
    T_aux_rgba_2       = 0x00000004,
    T_aux_rgba_3       = 0x00000008,

    T_aux_hrgba_0      = 0x00000010,
    T_aux_hrgba_1      = 0x00000020,
    T_aux_hrgba_2      = 0x00000040,
    T_aux_hrgba_3      = 0x00000080,

    T_aux_float_0      = 0x00000100,
    T_aux_float_1      = 0x00000200,
    T_aux_float_2      = 0x00000400,
    T_aux_float_3      = 0x00000800,

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
