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
    T_front_left       = 0x0001,
    T_back_left        = 0x0002,
    T_front_right      = 0x0004,
    T_back_right       = 0x0008,

    T_front            = 0x0005,
    T_back             = 0x000a,
    T_left             = 0x0003,
    T_right            = 0x000c,

    T_color            = 0x000f,

    T_depth            = 0x0010,
    T_stencil          = 0x0020,
    T_accum            = 0x0040,
  };


  RenderBuffer(GraphicsStateGuardian *gsg, int buffer_type)
    : _gsg(gsg), _buffer_type(buffer_type) { }

  GraphicsStateGuardian *_gsg;
  int _buffer_type;
};

#endif
