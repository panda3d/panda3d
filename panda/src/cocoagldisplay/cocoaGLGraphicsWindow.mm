/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cocoaGLGraphicsWindow.mm
 * @author rdb
 * @date 2023-03-18
 */

#include "cocoaGLGraphicsWindow.h"
#include "cocoaGLGraphicsStateGuardian.h"
#include "config_cocoadisplay.h"

#import <OpenGL/OpenGL.h>

TypeHandle CocoaGLGraphicsWindow::_type_handle;

/**
 *
 */
CocoaGLGraphicsWindow::
CocoaGLGraphicsWindow(GraphicsEngine *engine, GraphicsPipe *pipe,
                      const std::string &name,
                      const FrameBufferProperties &fb_prop,
                      const WindowProperties &win_prop,
                      int flags,
                      GraphicsStateGuardian *gsg,
                      GraphicsOutput *host) :
  CocoaGraphicsWindow(engine, pipe, name, fb_prop, win_prop, flags, gsg, host)
{
}

/**
 *
 */
CocoaGLGraphicsWindow::
~CocoaGLGraphicsWindow() {
}

/**
 * This function will be called within the draw thread before beginning
 * rendering for a given frame.  It should do whatever setup is required, and
 * return true if the frame should be rendered, or false if it should be
 * skipped.
 */
bool CocoaGLGraphicsWindow::
begin_frame(FrameMode mode, Thread *current_thread) {
  PStatTimer timer(_make_current_pcollector, current_thread);

  begin_frame_spam(mode);
  if (_gsg == nullptr) {
    return false;
  }

  CocoaGLGraphicsStateGuardian *cocoagsg;
  DCAST_INTO_R(cocoagsg, _gsg, false);
  nassertr(cocoagsg->_context != nil, false);
  nassertr(_view != nil, false);

  // Place a lock on the context.
  cocoagsg->lock_context();

  // Set the drawable.
  // Although not recommended, it is technically possible to use the same
  // context with multiple different-sized windows.  If that happens, the
  // context needs to be updated accordingly.
  if ([cocoagsg->_context view] != _view) {
    // XXX I'm not 100% sure that changing the view requires it to update.
    _context_needs_update = true;
    [cocoagsg->_context setView:_view];

    if (cocoadisplay_cat.is_spam()) {
      cocoadisplay_cat.spam()
        << "Switching context to view " << _view << "\n";
    }
  }

  // Update the context if necessary, to make it reallocate buffers etc.
  if (_context_needs_update) {
    if ([NSThread isMainThread]) {
      [cocoagsg->_context update];
      _context_needs_update = false;
    } else {
      cocoagsg->unlock_context();
      return false;
    }
  }

  // Lock the view for drawing.
  if (!_properties.get_fullscreen()) {
    nassertr_always([_view lockFocusIfCanDraw], false);
  }

  // Make the context current.
  [cocoagsg->_context makeCurrentContext];

  // Now that we have made the context current to a window, we can reset the
  // GSG state if this is the first time it has been used.  (We can't just
  // call reset() when we construct the GSG, because reset() requires having a
  // current context.)
  cocoagsg->reset_if_new();

  if (mode == FM_render) {
    // begin_render_texture();
    clear_cube_map_selection();
  }

  _gsg->set_current_properties(&get_fb_properties());
  return _gsg->begin_frame(current_thread);
}

/**
 * This function will be called within the draw thread after rendering is
 * completed for a given frame.  It should do whatever finalization is
 * required.
 */
void CocoaGLGraphicsWindow::
end_frame(FrameMode mode, Thread *current_thread) {
  end_frame_spam(mode);
  nassertv(_gsg != (GraphicsStateGuardian *)NULL);

  if (!_properties.get_fullscreen()) {
    [_view unlockFocus];
  }
  // Release the context.
  CocoaGLGraphicsStateGuardian *cocoagsg;
  DCAST_INTO_V(cocoagsg, _gsg);

  cocoagsg->unlock_context();

  if (mode == FM_render) {
    // end_render_texture();
    copy_to_textures();
  }

  _gsg->end_frame(current_thread);

  if (mode == FM_render) {
    trigger_flip();
    clear_cube_map_selection();
  }
}

/**
 * This function will be called within the draw thread after begin_flip() has
 * been called on all windows, to finish the exchange of the front and back
 * buffers.
 *
 * This should cause the window to wait for the flip, if necessary.
 */
void CocoaGLGraphicsWindow::
end_flip() {
  if (_gsg != nullptr && _flip_ready) {

    CocoaGLGraphicsStateGuardian *cocoagsg;
    DCAST_INTO_V(cocoagsg, _gsg);

    if (sync_video) {
      CocoaGraphicsPipe *cocoapipe = (CocoaGraphicsPipe *)_pipe.p();
      if (!_vsync_enabled) {
        // If this fails, we don't keep trying.
        cocoapipe->init_vsync(_vsync_counter);
        _vsync_enabled = true;
      }
      cocoapipe->wait_vsync(_vsync_counter);
    } else {
      _vsync_enabled = false;
    }

    cocoagsg->lock_context();

    // Swap the front and back buffer.
    [cocoagsg->_context flushBuffer];

    [[_view window] flushWindow];

    cocoagsg->unlock_context();
  }
  GraphicsWindow::end_flip();
}

/**
 * Opens the window right now.  Called from the window thread.  Returns true
 * if the window is successfully opened, or false if there was a problem.
 */
bool CocoaGLGraphicsWindow::
open_window() {
  // GSG CreationInitialization
  CocoaGLGraphicsStateGuardian *cocoagsg;
  if (_gsg == nullptr) {
    // There is no old gsg.  Create a new one.
    cocoagsg = new CocoaGLGraphicsStateGuardian(_engine, _pipe, nullptr);
    cocoagsg->choose_pixel_format(_fb_properties, _display, false);
    _gsg = cocoagsg;
  } else {
    // If the old gsg has the wrong pixel format, create a new one that shares
    // with the old gsg.
    DCAST_INTO_R(cocoagsg, _gsg, false);
    if (cocoagsg->get_engine() != _engine ||
        !cocoagsg->get_fb_properties().subsumes(_fb_properties)) {
      cocoagsg = new CocoaGLGraphicsStateGuardian(_engine, _pipe, cocoagsg);
      cocoagsg->choose_pixel_format(_fb_properties, _display, false);
      _gsg = cocoagsg;
    }
  }

  if (cocoagsg->_context == nil) {
    // Could not obtain a proper context.
    _gsg.clear();
    close_window();
    return false;
  }

  if (!CocoaGraphicsWindow::open_window()) {
    return false;
  }

  // Make the context current.
  cocoagsg->lock_context();
  _context_needs_update = false;
  [cocoagsg->_context makeCurrentContext];
  [cocoagsg->_context setView:_view];
  [cocoagsg->_context update];

  cocoagsg->reset_if_new();
  cocoagsg->unlock_context();

  if (!cocoagsg->is_valid()) {
    close_window();
    return false;
  }

  if (!cocoagsg->get_fb_properties().verify_hardware_software
      (_fb_properties, cocoagsg->get_gl_renderer())) {
    close_window();
    return false;
  }
  _fb_properties = cocoagsg->get_fb_properties();

  return true;
}

/**
 * Updates the context.
 */
void CocoaGLGraphicsWindow::
update_context() {
  CocoaGLGraphicsStateGuardian *cocoagsg;
  cocoagsg = DCAST(CocoaGLGraphicsStateGuardian, _gsg);

  if (cocoagsg != nullptr && cocoagsg->_context != nil) {
    cocoagsg->lock_context();
    _context_needs_update = false;
    [cocoagsg->_context update];
    cocoagsg->unlock_context();
  }
}

/**
 * Unbinds the context from the window.
 */
void CocoaGLGraphicsWindow::
unbind_context() {
  CocoaGLGraphicsStateGuardian *cocoagsg;
  cocoagsg = DCAST(CocoaGLGraphicsStateGuardian, _gsg);

  if (cocoagsg != nullptr && cocoagsg->_context != nil) {
    cocoagsg->lock_context();
    [cocoagsg->_context clearDrawable];
    cocoagsg->unlock_context();
  }
}
