// Filename: frameBufferStack.h
// Created by:  drose (06Oct99)
//
////////////////////////////////////////////////////////////////////

#ifndef FRAMEBUFFERSTACK_H
#define FRAMEBUFFERSTACK_H

#include <pandabase.h>

#include "savedFrameBuffer.h"

class GraphicsStateGuardian;

////////////////////////////////////////////////////////////////////
// 	 Class : FrameBufferStack
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
