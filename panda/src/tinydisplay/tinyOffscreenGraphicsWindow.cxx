/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file tinyOffscreenGraphicsWindow.cxx
 * @author drose
 * @date 2008-04-24
 */

#include "pandabase.h"


#include "tinyOffscreenGraphicsWindow.h"
#include "tinyGraphicsStateGuardian.h"
#include "config_tinydisplay.h"
#include "tinyOffscreenGraphicsPipe.h"
#include "mouseButton.h"
#include "keyboardButton.h"
#include "graphicsPipe.h"

TypeHandle TinyOffscreenGraphicsWindow::_type_handle;

/**
 *
 */
TinyOffscreenGraphicsWindow::
TinyOffscreenGraphicsWindow(GraphicsEngine *engine, GraphicsPipe *pipe,
                      const std::string &name,
                      const FrameBufferProperties &fb_prop,
                      const WindowProperties &win_prop,
                      int flags,
                      GraphicsStateGuardian *gsg,
                      GraphicsOutput *host) :
  GraphicsWindow(engine, pipe, name, fb_prop, win_prop, flags, gsg, host)
{
  //_screen = nullptr;
  _frame_buffer = nullptr;
  _pitch = 0;
  update_pixel_factor();

  add_input_device(GraphicsWindowInputDevice::pointer_and_keyboard(this, "keyboard_mouse"));
}

/**
 *
 */
TinyOffscreenGraphicsWindow::
~TinyOffscreenGraphicsWindow() {
}

/**
 * This function will be called within the draw thread before beginning
 * rendering for a given frame.  It should do whatever setup is required, and
 * return true if the frame should be rendered, or false if it should be
 * skipped.
 */
bool TinyOffscreenGraphicsWindow::
begin_frame(FrameMode mode, Thread *current_thread) {
  begin_frame_spam(mode);
  if (_gsg == nullptr) {
    return false;
  }

  TinyGraphicsStateGuardian *tinygsg;
  DCAST_INTO_R(tinygsg, _gsg, false);

  tinygsg->_current_frame_buffer = _frame_buffer;
  tinygsg->reset_if_new();

  _gsg->set_current_properties(&get_fb_properties());
  return _gsg->begin_frame(current_thread);
}

/**
 * This function will be called within the draw thread after rendering is
 * completed for a given frame.  It should do whatever finalization is
 * required.
 */
void TinyOffscreenGraphicsWindow::
end_frame(FrameMode mode, Thread *current_thread) {
  end_frame_spam(mode);
  nassertv(_gsg != nullptr);

  if (mode == FM_render) {
    // end_render_texture();
    copy_to_textures();
  }

  _gsg->end_frame(current_thread);

  if (mode == FM_render) {
    trigger_flip();
  }
}

/**
 * This function will be called within the draw thread after begin_flip() has
 * been called on all windows, to finish the exchange of the front and back
 * buffers.
 *
 * This should cause the window to wait for the flip, if necessary.
 */
void TinyOffscreenGraphicsWindow::
end_flip() {
  if (!_flip_ready) {
    GraphicsWindow::end_flip();
    return;
  }
/*
  int fb_xsize = get_fb_x_size();
  int fb_ysize = get_fb_y_size();
  ZB_copyFrameBuffer(_frame_buffer, _screen->pixels, _pitch);
*/
  GraphicsWindow::end_flip();
}

/**
 * Do whatever processing is necessary to ensure that the window responds to
 * user events.  Also, honor any requests recently made via
 * request_properties()
 *
 * This function is called only within the window thread.
 */
void TinyOffscreenGraphicsWindow::
process_events() {
  GraphicsWindow::process_events();
/*
  if (_screen == nullptr) {
    return;
  }
*/
}

/**
 * Applies the requested set of properties to the window, if possible, for
 * instance to request a change in size or minimization status.
 *
 * The window properties are applied immediately, rather than waiting until
 * the next frame.  This implies that this method may *only* be called from
 * within the window thread.
 *
 * The return value is true if the properties are set, false if they are
 * ignored.  This is mainly useful for derived classes to implement extensions
 * to this function.
 */
void TinyOffscreenGraphicsWindow::
set_properties_now(WindowProperties &properties) {
  GraphicsWindow::set_properties_now(properties);
  if (!properties.is_any_specified()) {
    // The base class has already handled this case.
    return;
  }
}

/**
 * Returns true if a call to set_pixel_zoom() will be respected, false if it
 * will be ignored.  If this returns false, then get_pixel_factor() will
 * always return 1.0, regardless of what value you specify for
 * set_pixel_zoom().
 *
 * This may return false if the underlying renderer doesn't support pixel
 * zooming, or if you have called this on a DisplayRegion that doesn't have
 * both set_clear_color() and set_clear_depth() enabled.
 */
bool TinyOffscreenGraphicsWindow::
supports_pixel_zoom() const {
  return true;
}

/**
 * Closes the window right now.  Called from the window thread.
 */
void TinyOffscreenGraphicsWindow::
close_window() {
  GraphicsWindow::close_window();
}

/**
 * Opens the window right now.  Called from the window thread.  Returns true
 * if the window is successfully opened, or false if there was a problem.
 */
bool TinyOffscreenGraphicsWindow::
open_window() {

  // GSG CreationInitialization
  TinyGraphicsStateGuardian *tinygsg;
  if (_gsg == nullptr) {
    // There is no old gsg.  Create a new one.
    tinygsg = new TinyGraphicsStateGuardian(_engine, _pipe, nullptr);
    _gsg = tinygsg;

  } else {
    DCAST_INTO_R(tinygsg, _gsg, false);
  }

  create_frame_buffer();
  if (_frame_buffer == nullptr) {
    tinydisplay_cat.error()
      << "Could not create frame buffer.\n";
    return false;
  }

  tinygsg->_current_frame_buffer = _frame_buffer;

  // Now that we have made the context current to a window, we can reset the
  // GSG state if this is the first time it has been used.  (We can't just
  // call reset() when we construct the GSG, because reset() requires having a
  // current context.)
  tinygsg->reset_if_new();

  return true;
}

/**
 * Creates a suitable frame buffer for the current window size.
 */
void TinyOffscreenGraphicsWindow::
create_frame_buffer() {
  if (_frame_buffer != nullptr) {
    ZB_close(_frame_buffer);
    _frame_buffer = nullptr;
  }

  int mode;
/*
  switch (_screen->format->BitsPerPixel) {
  case  8:
    tinydisplay_cat.error() << "Offscreen Palettes are currently not supported.\n";
    return;

  case 16:
    mode = ZB_MODE_5R6G5B;
    break;
  case 24:
    mode = ZB_MODE_RGB24;
    break;
  case 32:
    mode = ZB_MODE_RGBA;
    break;

  default:
    return;
  }
*/
    mode = ZB_MODE_RGBA;

    _frame_buffer = ZB_open(_properties.get_x_size(), _properties.get_y_size(), mode, 0, 0, 0, 0);

    _pitch = 4; //  _screen->pitch * 32 / _screen->format->BitsPerPixel;
}

/**
 * Maps from an Offscreen keysym to the corresponding Panda ButtonHandle.
 */
ButtonHandle TinyOffscreenGraphicsWindow::
get_keyboard_button(int sym) {
  tinydisplay_cat.info() << "unhandled keyboard button " << sym << "\n";
  return ButtonHandle::none();
}

/**
 * Maps from an Offscreen mouse button index to the corresponding Panda
 * ButtonHandle.
 */
ButtonHandle TinyOffscreenGraphicsWindow::
get_mouse_button(int button) {
  tinydisplay_cat.info() << "unhandled mouse button " << button << "\n";

  return ButtonHandle::none();
}

