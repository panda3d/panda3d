/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file graphicsBuffer.cxx
 * @author drose
 * @date 2004-02-06
 */

#include "graphicsBuffer.h"

TypeHandle GraphicsBuffer::_type_handle;

/**
 * Normally, the GraphicsBuffer constructor is not called directly; these are
 * created instead via the GraphicsEngine::make_buffer() function.
 */
GraphicsBuffer::
GraphicsBuffer(GraphicsEngine *engine, GraphicsPipe *pipe,
               const std::string &name,
               const FrameBufferProperties &fb_prop,
               const WindowProperties &win_prop, int flags,
               GraphicsStateGuardian *gsg,
               GraphicsOutput *host) :
  GraphicsOutput(engine, pipe, name, fb_prop, win_prop, flags, gsg, host, false)
{
#ifdef DO_MEMORY_USAGE
  MemoryUsage::update_type(this, this);
#endif

  if (display_cat.is_debug()) {
    display_cat.debug()
      << "Creating new offscreen buffer " << get_name() << "\n";
  }

  _overlay_display_region->compute_pixels(_size.get_x(), _size.get_y());
  _open_request = OR_none;
}

/**
 *
 */
GraphicsBuffer::
~GraphicsBuffer() {
}

/**
 * This is called by the GraphicsEngine to request that the buffer resize
 * itself.  Although calls to get the size will return the new value, much of
 * the actual resizing work doesn't take place until the next begin_frame.
 * Not all buffers are resizeable.
 */
void GraphicsBuffer::
set_size(int x, int y) {
  if ((_creation_flags & GraphicsPipe::BF_resizeable) == 0) {
    nassert_raise("Cannot resize buffer unless it is created with BF_resizeable flag");
    return;
  }
  set_size_and_recalc(x, y);
}

/**
 * This is called by the GraphicsEngine to request that the buffer (or
 * whatever) open itself or, in general, make itself valid, at the next call
 * to process_events().
 */
void GraphicsBuffer::
request_open() {
  _open_request = OR_open;
}

/**
 * This is called by the GraphicsEngine to request that the buffer (or
 * whatever) close itself or, in general, make itself invalid, at the next
 * call to process_events().  By that time we promise the gsg pointer will be
 * cleared.
 */
void GraphicsBuffer::
request_close() {
  _open_request = OR_none;
}

/**
 * This is called by the GraphicsEngine to insist that the buffer be closed
 * immediately.  This is only called from the buffer thread.
 */
void GraphicsBuffer::
set_close_now() {
  _open_request = OR_none;
  close_buffer();
}

/**
 * Honor any requests recently made via request_open() or request_close().
 *
 * This function is called only within the window thread.
 */
void GraphicsBuffer::
process_events() {
  // Save the current request and reset it immediately, in case we end up
  // calling recursively back into this function.
  OpenRequest this_request = _open_request;
  _open_request = OR_none;

  switch (this_request) {
  case OR_none:
    return;

  case OR_open:
    if (open_buffer()) {
      _is_valid = true;
      set_inverted(_gsg->get_copy_texture_inverted());
    }
    break;

  case OR_close:
    close_buffer();
    break;
  }
}

/**
 * Closes the buffer right now.  Called from the window thread.
 */
void GraphicsBuffer::
close_buffer() {
  display_cat.info()
    << "Closing " << get_type() << "\n";
}

/**
 * Opens the buffer right now.  Called from the window thread.  Returns true
 * if the buffer is successfully opened, or false if there was a problem.
 */
bool GraphicsBuffer::
open_buffer() {
  return false;
}
