/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cocoaGraphicsBuffer.mm
 * @author rdb
 * @date 2017-12-19
 */

#include "cocoaGraphicsBuffer.h"
#include "cocoaGraphicsStateGuardian.h"
#include "config_cocoadisplay.h"
#include "cocoaGraphicsPipe.h"

#import <OpenGL/OpenGL.h>

TypeHandle CocoaGraphicsBuffer::_type_handle;

/**
 *
 */
CocoaGraphicsBuffer::
CocoaGraphicsBuffer(GraphicsEngine *engine, GraphicsPipe *pipe,
                    const std::string &name,
                    const FrameBufferProperties &fb_prop,
                    const WindowProperties &win_prop,
                    int flags,
                    GraphicsStateGuardian *gsg,
                    GraphicsOutput *host) : // Ignore the host.
  GLGraphicsBuffer(engine, pipe, name, fb_prop, win_prop, flags, gsg, nullptr)
{
}

/**
 * This function will be called within the draw thread before beginning
 * rendering for a given frame.  It should do whatever setup is required, and
 * return true if the frame should be rendered, or false if it should be
 * skipped.
 */
bool CocoaGraphicsBuffer::
begin_frame(FrameMode mode, Thread *current_thread) {
  if (_gsg == nullptr) {
    return false;
  }

  CocoaGraphicsStateGuardian *cocoagsg;
  DCAST_INTO_R(cocoagsg, _gsg, false);
  nassertr(cocoagsg->_context != nil, false);

  // Lock the context and make it current.
  {
    PStatTimer timer(_make_current_pcollector, current_thread);
    cocoagsg->lock_context();
    [cocoagsg->_context makeCurrentContext];
  }

  return GLGraphicsBuffer::begin_frame(mode, current_thread);
}

/**
 * This function will be called within the draw thread after rendering is
 * completed for a given frame.  It should do whatever finalization is
 * required.
 */
void CocoaGraphicsBuffer::
end_frame(FrameMode mode, Thread *current_thread) {
  nassertv(_gsg != nullptr);

  GLGraphicsBuffer::end_frame(mode, current_thread);

  // Release the context.
  CocoaGraphicsStateGuardian *cocoagsg;
  DCAST_INTO_V(cocoagsg, _gsg);
  cocoagsg->unlock_context();
}

/**
 * Opens the buffer right now.  Called from the window thread.  Returns true
 * if the buffer is successfully opened, or false if there was a problem.
 */
bool CocoaGraphicsBuffer::
open_buffer() {
  CocoaGraphicsPipe *cocoa_pipe;
  DCAST_INTO_R(cocoa_pipe, _pipe, false);

  // GSG CreationInitialization
  CocoaGraphicsStateGuardian *cocoagsg;
  if (_gsg == nullptr) {
    // There is no old gsg.  Create a new one.
    cocoagsg = new CocoaGraphicsStateGuardian(_engine, _pipe, nullptr);
    cocoagsg->choose_pixel_format(_fb_properties, cocoa_pipe->get_display_id(), false);
    _gsg = cocoagsg;
  } else {
    // If the old gsg has the wrong pixel format, create a new one that shares
    // with the old gsg.
    DCAST_INTO_R(cocoagsg, _gsg, false);
    if (!cocoagsg->get_fb_properties().subsumes(_fb_properties)) {
      cocoagsg = new CocoaGraphicsStateGuardian(_engine, _pipe, cocoagsg);
      cocoagsg->choose_pixel_format(_fb_properties, cocoa_pipe->get_display_id(), false);
      _gsg = cocoagsg;
    }
  }

  if (cocoagsg->_context == nil) {
    // Could not obtain a proper context.
    _gsg.clear();
    return false;
  }

  FrameBufferProperties desired_props(_fb_properties);

  // Lock the context, so we can safely operate on it.
  cocoagsg->lock_context();

  // Make the context current and initialize what we need.
  [cocoagsg->_context makeCurrentContext];
  [cocoagsg->_context update];
  cocoagsg->reset_if_new();

  // These properties are determined by choose_pixel_format.
  _fb_properties.set_force_hardware(cocoagsg->_fbprops.get_force_hardware());
  _fb_properties.set_force_software(cocoagsg->_fbprops.get_force_software());

  bool success = GLGraphicsBuffer::open_buffer();
  if (success) {
    rebuild_bitplanes();
    if (_needs_rebuild) {
      // If it still needs rebuild, then something must have gone wrong.
      success = false;
    }
  }

  if (success && !_fb_properties.verify_hardware_software
      (desired_props, cocoagsg->get_gl_renderer())) {
    GLGraphicsBuffer::close_buffer();
    success = false;
  }

  // Release the context.
  cocoagsg->unlock_context();

  if (!success) {
    return false;
  }

  return true;
}

/**
 * Closes the buffer right now.  Called from the window thread.
 */
void CocoaGraphicsBuffer::
close_buffer() {
  if (_gsg != nullptr) {
    CocoaGraphicsStateGuardian *cocoagsg;
    cocoagsg = DCAST(CocoaGraphicsStateGuardian, _gsg);

    if (cocoagsg != nullptr && cocoagsg->_context != nil) {
      cocoagsg->lock_context();
      GLGraphicsBuffer::close_buffer();
      cocoagsg->unlock_context();
    }
    _gsg.clear();
  } else {
    GLGraphicsBuffer::close_buffer();
  }
}
