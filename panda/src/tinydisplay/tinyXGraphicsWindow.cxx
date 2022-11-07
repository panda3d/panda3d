/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file tinyXGraphicsWindow.cxx
 * @author drose
 * @date 2008-05-03
 */

#include "pandabase.h"

#ifdef HAVE_X11

#include "tinyXGraphicsWindow.h"
#include "tinyGraphicsStateGuardian.h"
#include "tinyXGraphicsPipe.h"
#include "config_tinydisplay.h"

#include "graphicsPipe.h"
#include "keyboardButton.h"
#include "mouseButton.h"
#include "clockObject.h"
#include "pStatTimer.h"
#include "textEncoder.h"
#include "throw_event.h"
#include "lightReMutexHolder.h"
#include "nativeWindowHandle.h"

TypeHandle TinyXGraphicsWindow::_type_handle;

/**
 *
 */
TinyXGraphicsWindow::
TinyXGraphicsWindow(GraphicsEngine *engine, GraphicsPipe *pipe,
                    const std::string &name,
                    const FrameBufferProperties &fb_prop,
                    const WindowProperties &win_prop,
                    int flags,
                    GraphicsStateGuardian *gsg,
                    GraphicsOutput *host) :
  x11GraphicsWindow(engine, pipe, name, fb_prop, win_prop, flags, gsg, host)
{
  _gc = (GC)nullptr;

  _reduced_frame_buffer = nullptr;
  _full_frame_buffer = nullptr;
  _ximage = nullptr;
  update_pixel_factor();
}

/**
 *
 */
TinyXGraphicsWindow::
~TinyXGraphicsWindow() {
  if (_gc != nullptr && _display != nullptr) {
    XFreeGC(_display, _gc);
  }
  if (_ximage != nullptr) {
    PANDA_FREE_ARRAY(_ximage->data);
    _ximage->data = nullptr;
    XDestroyImage(_ximage);
  }
}

/**
 * This function will be called within the draw thread before beginning
 * rendering for a given frame.  It should do whatever setup is required, and
 * return true if the frame should be rendered, or false if it should be
 * skipped.
 */
bool TinyXGraphicsWindow::
begin_frame(FrameMode mode, Thread *current_thread) {
  PStatTimer timer(_make_current_pcollector, current_thread);

  if (_xwindow == (X11_Window)nullptr) {
    return false;
  }

  begin_frame_spam(mode);
  if (_gsg == nullptr) {
    return false;
  }
  if (_awaiting_configure_since != -1) {
    // Don't attempt to draw while we have just reconfigured the window and we
    // haven't got the notification back yet.
    return false;
  }

  TinyGraphicsStateGuardian *tinygsg;
  DCAST_INTO_R(tinygsg, _gsg, false);

  if (_reduced_frame_buffer != nullptr) {
    tinygsg->_current_frame_buffer = _reduced_frame_buffer;
  } else {
    tinygsg->_current_frame_buffer = _full_frame_buffer;
  }
  tinygsg->reset_if_new();

  _gsg->set_current_properties(&get_fb_properties());
  return _gsg->begin_frame(current_thread);
}

/**
 * This function will be called within the draw thread after rendering is
 * completed for a given frame.  It should do whatever finalization is
 * required.
 */
void TinyXGraphicsWindow::
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
void TinyXGraphicsWindow::
end_flip() {
  if (_xwindow == (X11_Window)nullptr || !_flip_ready) {
    GraphicsWindow::end_flip();
    return;
  }

  if (_reduced_frame_buffer != nullptr) {
    // Zoom the reduced buffer onto the full buffer.
    ZB_zoomFrameBuffer(_full_frame_buffer, 0, 0,
                       _full_frame_buffer->xsize, _full_frame_buffer->ysize,
                       _reduced_frame_buffer, 0, 0,
                       _reduced_frame_buffer->xsize, _reduced_frame_buffer->ysize);
  }

  // We can't just point the XPutImage directly at our own framebuffer data,
  // even if the bytes_per_pixel matches, because some X displays will respect
  // the alpha channel and make the window transparent there.  We don't want
  // transparent windows where the alpha data happens to less than 1.0.
  ZB_copyFrameBufferNoAlpha(_full_frame_buffer, _ximage->data, _pitch);

  XPutImage(_display, _xwindow, _gc, _ximage, 0, 0, 0, 0,
            _full_frame_buffer->xsize, _full_frame_buffer->ysize);
  XFlush(_display);
  GraphicsWindow::end_flip();
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
bool TinyXGraphicsWindow::
supports_pixel_zoom() const {
  return true;
}

/**
 * Do whatever processing is necessary to ensure that the window responds to
 * user events.  Also, honor any requests recently made via
 * request_properties()
 *
 * This function is called only within the window thread.
 */
void TinyXGraphicsWindow::
process_events() {
  x11GraphicsWindow::process_events();

  int xsize = (_properties.get_x_size() + 3) & ~3;
  int ysize = _properties.get_y_size();

  if (xsize != _full_frame_buffer->xsize ||
      ysize != _full_frame_buffer->ysize) {
    ZB_resize(_full_frame_buffer, nullptr, xsize, ysize);
    _pitch = (_full_frame_buffer->xsize * _bytes_per_pixel + 3) & ~3;
    create_reduced_frame_buffer();
    create_ximage();
  }
}

/**
 * Closes the window right now.  Called from the window thread.
 */
void TinyXGraphicsWindow::
close_window() {
  if (_gsg != nullptr) {
    TinyGraphicsStateGuardian *tinygsg;
    DCAST_INTO_V(tinygsg, _gsg);
    tinygsg->_current_frame_buffer = nullptr;
    _gsg.clear();
  }

  x11GraphicsWindow::close_window();
}

/**
 * Opens the window right now.  Called from the window thread.  Returns true
 * if the window is successfully opened, or false if there was a problem.
 */
bool TinyXGraphicsWindow::
open_window() {
  TinyXGraphicsPipe *tinyx_pipe;
  DCAST_INTO_R(tinyx_pipe, _pipe, false);

  // GSG CreationInitialization
  TinyGraphicsStateGuardian *tinygsg;
  if (_gsg == 0) {
    // There is no old gsg.  Create a new one.
    tinygsg = new TinyGraphicsStateGuardian(_engine, _pipe, nullptr);
    _gsg = tinygsg;
  } else {
    DCAST_INTO_R(tinygsg, _gsg, false);
  }

  XVisualInfo vinfo_template;
  vinfo_template.screen = _screen;
  vinfo_template.depth = 32;
  vinfo_template.c_class = TrueColor;

  // Try to get each of these properties in turn.
  int try_masks[] = {
    VisualScreenMask | VisualDepthMask | VisualClassMask,
    VisualScreenMask | VisualClassMask,
    VisualScreenMask | VisualDepthMask,
    VisualScreenMask,
    0,
  };

  int i = 0;
  int num_vinfos = 0;
  XVisualInfo *vinfo_array;
  while (try_masks[i] != 0 && num_vinfos == 0) {
    vinfo_array =
      XGetVisualInfo(_display, try_masks[i], &vinfo_template, &num_vinfos);
    ++i;
  }

  if (num_vinfos == 0) {
    // No suitable X visual.
    tinydisplay_cat.error()
      << "No suitable X Visual available; cannot open window.\n";
    return false;
  }
  _visual_info = &vinfo_array[0];

  _visual = _visual_info->visual;
  _depth = _visual_info->depth;
  _bytes_per_pixel = _depth / 8;
  if (_bytes_per_pixel == 3) {
    // Seems to be a special case.
    _bytes_per_pixel = 4;
  }
  tinydisplay_cat.info()
    << "Got X Visual with depth " << _depth << " (bpp " << _bytes_per_pixel << ") and class ";
  switch (_visual_info->c_class) {
  case TrueColor:
    tinydisplay_cat.info(false) << "TrueColor\n";
    break;

  case DirectColor:
    tinydisplay_cat.info(false) << "DirectColor\n";
    break;

  case StaticColor:
    tinydisplay_cat.info(false) << "StaticColor\n";
    break;

  case StaticGray:
    tinydisplay_cat.info(false) << "StaticGray\n";
    break;

  case GrayScale:
    tinydisplay_cat.info(false) << "GrayScale\n";
    break;

  case PseudoColor:
    tinydisplay_cat.info(false) << "PseudoColor\n";
    break;
  }

  setup_colormap(_visual_info);

  if (!x11GraphicsWindow::open_window()) {
    return false;
  }

  _gc = XCreateGC(_display, _xwindow, 0, nullptr);

  create_full_frame_buffer();
  if (_full_frame_buffer == nullptr) {
    tinydisplay_cat.error()
      << "Could not create frame buffer.\n";
    return false;
  }
  create_reduced_frame_buffer();
  create_ximage();
  nassertr(_ximage != nullptr, false);

  tinygsg->_current_frame_buffer = _full_frame_buffer;

  tinygsg->reset_if_new();
  if (!tinygsg->is_valid()) {
    close_window();
    return false;
  }

  XMapWindow(_display, _xwindow);

  if (_properties.get_raw_mice()) {
    open_raw_mice();
  } else {
    if (tinydisplay_cat.is_debug()) {
      tinydisplay_cat.debug()
        << "Raw mice not requested.\n";
    }
  }

  // Create a WindowHandle for ourselves
  _window_handle = NativeWindowHandle::make_x11(_xwindow);

  // And tell our parent window that we're now its child.
  if (_parent_window_handle != nullptr) {
    _parent_window_handle->attach_child(_window_handle);
  }

  return true;
}

/**
 * Called internally when the pixel factor changes.
 */
void TinyXGraphicsWindow::
pixel_factor_changed() {
  x11GraphicsWindow::pixel_factor_changed();
  create_reduced_frame_buffer();
}

/**
 * Creates a suitable frame buffer for the current window size.
 */
void TinyXGraphicsWindow::
create_full_frame_buffer() {
  if (_full_frame_buffer != nullptr) {
    ZB_close(_full_frame_buffer);
    _full_frame_buffer = nullptr;
  }

  int mode;
  switch (_bytes_per_pixel) {
  case  1:
    tinydisplay_cat.error()
      << "Palette images are currently not supported.\n";
    return;

  case 2:
    mode = ZB_MODE_5R6G5B;
    break;
  case 4:
    mode = ZB_MODE_RGBA;
    break;

  default:
    return;
  }

  _full_frame_buffer = ZB_open(_properties.get_x_size(), _properties.get_y_size(), mode, 0, 0, 0, 0);
  _pitch = (_full_frame_buffer->xsize * _bytes_per_pixel + 3) & ~3;
}

/**
 * Creates a suitable frame buffer for the current window size and pixel zoom.
 */
void TinyXGraphicsWindow::
create_reduced_frame_buffer() {
  if (!_full_frame_buffer) {
     return;
  }

  if (_reduced_frame_buffer != nullptr) {
    ZB_close(_reduced_frame_buffer);
    _reduced_frame_buffer = nullptr;
  }

  int x_size = get_fb_x_size();
  int y_size = get_fb_y_size();

  if (x_size == _full_frame_buffer->xsize) {
    // No zooming is necessary.

  } else {
    // The reduced size is different, so we need a separate buffer to render
    // into.
    _reduced_frame_buffer = ZB_open(x_size, y_size, _full_frame_buffer->mode, 0, 0, 0, 0);
  }
}


/**
 * Creates a suitable XImage for the current window size.
 */
void TinyXGraphicsWindow::
create_ximage() {
  if (_ximage != nullptr) {
    PANDA_FREE_ARRAY(_ximage->data);
    _ximage->data = nullptr;
    XDestroyImage(_ximage);
    _ximage = nullptr;
  }

  int image_size = _full_frame_buffer->ysize * _pitch;
  char *data = (char *)PANDA_MALLOC_ARRAY(image_size);

  _ximage = XCreateImage(_display, _visual, _depth, ZPixmap, 0, data,
                         _full_frame_buffer->xsize, _full_frame_buffer->ysize,
                         32, 0);
}

#endif  // HAVE_X11
