// Filename: frameBufferStack.h
// Created by:  drose (06Oct99)
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

#ifndef FRAMEBUFFERSTACK_H
#define FRAMEBUFFERSTACK_H

#include "pandabase.h"

#include "savedFrameBuffer.h"

class GraphicsStateGuardian;

////////////////////////////////////////////////////////////////////
//       Class : FrameBufferStack
// Description : An instance of this kind of object is returned by
//               GraphicsStateGuardian::push_frame_buffer().  It
//               holds the information needed to restore the previous
//               frame buffer contents in the subsequent matching call
//               to pop_frame_buffer().
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA FrameBufferStack {
public:
  INLINE FrameBufferStack();
  INLINE ~FrameBufferStack();
  INLINE FrameBufferStack(const FrameBufferStack &copy);
  INLINE void operator =(const FrameBufferStack &copy);

private:
  PT(SavedFrameBuffer) _frame_buffer;
  int _stack_level;
  friend class GraphicsStateGuardian;
};

#include "frameBufferStack.I"

#endif
