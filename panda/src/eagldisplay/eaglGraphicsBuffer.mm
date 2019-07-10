/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eaglGraphicsBuffer.mm
 * @author D. Lawrence
 * @date 2019-07-09
 */

#include "eaglGraphicsBuffer.h"
#include "eaglGraphicsStateGuardian.h"
#include "config_eagldisplay.h"

TypeHandle EAGLGraphicsBuffer::_type_handle;

EAGLGraphicsBuffer::
EAGLGraphicsBuffer(GraphicsEngine *engine, GraphicsPipe *pipe,
                    const std::string &name,
                    const FrameBufferProperties &fb_prop,
                    const WindowProperties &win_prop,
                    int flags,
                    GraphicsStateGuardian *gsg,
                    GraphicsOutput *host) : // Ignore the host.
  GLES2GraphicsBuffer(engine, pipe, name, fb_prop, win_prop, flags, gsg, nullptr) {

}

/**
 * This function will be called within the draw thread before beginning
 * rendering for a given frame.  It should do whatever setup is required, and
 * return true if the frame should be rendered, or false if it should be
 * skipped.
 */
bool EAGLGraphicsBuffer::
begin_frame(FrameMode mode, Thread *current_thread) {
  if (_gsg == nullptr) {
    return false;
  }

  EAGLGraphicsStateGuardian *eaglgsg;
  DCAST_INTO_R(eaglgsg, _gsg, false);
  nassertr(eaglgsg->_context != nil, false);

  {
    PStatTimer timer(_make_current_pcollector, current_thread);
    eaglgsg->lock_context();
    [EAGLContext setCurrentContext:eaglgsg->_context];
    eaglgsg->set_current_properties(&get_fb_properties());
  }

  return GLES2GraphicsBuffer::begin_frame(mode, current_thread);
}

/**
 * This function will be called within the draw thread after rendering is
 * completed for a given frame.  It should do whatever finalization is
 * required.
 */
void EAGLGraphicsBuffer::
end_frame(FrameMode mode, Thread *current_thread) {
  nassertv(_gsg != nullptr);

  GLES2GraphicsBuffer::end_frame(mode, current_thread);

  // Release the context.
  EAGLGraphicsStateGuardian *eaglgsg;
  DCAST_INTO_V(eaglgsg, _gsg);
  eaglgsg->unlock_context();
}
