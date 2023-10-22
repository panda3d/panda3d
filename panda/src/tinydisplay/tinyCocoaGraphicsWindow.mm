/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file tinyCocoaGraphicsWindow.mm
 * @author rdb
 * @date 2023-03-21
 */

#include "pandabase.h"

#ifdef HAVE_COCOA

#include "tinyCocoaGraphicsWindow.h"
#include "tinyGraphicsStateGuardian.h"
#include "tinyCocoaGraphicsPipe.h"
#include "config_tinydisplay.h"

#import <QuartzCore/CALayer.h>

TypeHandle TinyCocoaGraphicsWindow::_type_handle;

/**
 *
 */
TinyCocoaGraphicsWindow::
TinyCocoaGraphicsWindow(GraphicsEngine *engine, GraphicsPipe *pipe,
                        const std::string &name,
                        const FrameBufferProperties &fb_prop,
                        const WindowProperties &win_prop,
                        int flags,
                        GraphicsStateGuardian *gsg,
                        GraphicsOutput *host) :
  CocoaGraphicsWindow(engine, pipe, name, fb_prop, win_prop, flags, gsg, host),
  _color_space(CGColorSpaceCreateDeviceRGB())
{
  update_pixel_factor();
}

/**
 *
 */
TinyCocoaGraphicsWindow::
~TinyCocoaGraphicsWindow() {
  CGColorSpaceRelease(_color_space);
}

/**
 * This function will be called within the draw thread before beginning
 * rendering for a given frame.  It should do whatever setup is required, and
 * return true if the frame should be rendered, or false if it should be
 * skipped.
 */
bool TinyCocoaGraphicsWindow::
begin_frame(FrameMode mode, Thread *current_thread) {
  begin_frame_spam(mode);
  if (_gsg == nullptr) {
    return false;
  }

  TinyGraphicsStateGuardian *tinygsg;
  DCAST_INTO_R(tinygsg, _gsg, false);

  tinygsg->_current_frame_buffer = _swap_chain[_swap_index]._frame_buffer;
  tinygsg->reset_if_new();

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
void TinyCocoaGraphicsWindow::
end_frame(FrameMode mode, Thread *current_thread) {
  end_frame_spam(mode);
  nassertv(_gsg != nullptr);

  if (mode == FM_render) {
    copy_to_textures();
  }

  _gsg->end_frame(current_thread);

  if (mode == FM_render) {
    if (_swap_chain.size() == 1) {
      do_present();
    }
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
void TinyCocoaGraphicsWindow::
end_flip() {
  if (_flip_ready) {
    do_present();
    _swap_index = (_swap_index + 1) % _swap_chain.size();

    // We don't really support proper VSync because we just update the backing
    // store and let the OS update it when needed, but we still need to wait
    // for VBlank so that the frame rate is appropriately limited.
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
  }

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
bool TinyCocoaGraphicsWindow::
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
void TinyCocoaGraphicsWindow::
process_events() {
  CocoaGraphicsWindow::process_events();

  if (!_swap_chain.empty()) {
    int xsize = (get_fb_x_size() + 3) & ~3;
    int ysize = get_fb_y_size();

    ZBuffer *frame_buffer = _swap_chain[0]._frame_buffer;
    if (xsize != frame_buffer->xsize ||
        ysize != frame_buffer->ysize) {
      create_swap_chain();
    }
  }
}

/**
 * Closes the window right now.  Called from the window thread.
 */
void TinyCocoaGraphicsWindow::
close_window() {
  if (_gsg != nullptr) {
    TinyGraphicsStateGuardian *tinygsg;
    DCAST_INTO_V(tinygsg, _gsg);
    tinygsg->_current_frame_buffer = nullptr;
    _gsg.clear();
  }

  for (SwapBuffer &swap_buffer : _swap_chain) {
    CFRelease(swap_buffer._data_provider);
    ZB_close(swap_buffer._frame_buffer);
  }
  _swap_chain.clear();

  CocoaGraphicsWindow::close_window();
}

/**
 * Opens the window right now.  Called from the window thread.  Returns true
 * if the window is successfully opened, or false if there was a problem.
 */
bool TinyCocoaGraphicsWindow::
open_window() {
  TinyCocoaGraphicsPipe *tinycocoa_pipe;
  DCAST_INTO_R(tinycocoa_pipe, _pipe, false);

  // GSG CreationInitialization
  TinyGraphicsStateGuardian *tinygsg;
  if (_gsg == nullptr) {
    // There is no old gsg.  Create a new one.
    tinygsg = new TinyGraphicsStateGuardian(_engine, _pipe, nullptr);
    _gsg = tinygsg;
  } else {
    DCAST_INTO_R(tinygsg, _gsg, false);
  }

  if (!CocoaGraphicsWindow::open_window()) {
    return false;
  }

  // Make sure we have a CALayer for drawing into.
  _view.wantsLayer = YES;
  _view.layerContentsRedrawPolicy = NSViewLayerContentsRedrawNever;

  create_swap_chain();
  if (_swap_chain.empty()) {
    tinydisplay_cat.error()
      << "Could not create frame buffer.\n";
    return false;
  }

  tinygsg->_current_frame_buffer = _swap_chain[_swap_index]._frame_buffer;

  tinygsg->reset_if_new();
  if (!tinygsg->is_valid()) {
    close_window();
    return false;
  }

  return true;
}

/**
 * Called internally when the pixel factor changes.
 */
void TinyCocoaGraphicsWindow::
pixel_factor_changed() {
  CocoaGraphicsWindow::pixel_factor_changed();
  create_swap_chain();
}

/**
 * Creates a suitable frame buffer for the current window size.
 */
void TinyCocoaGraphicsWindow::
create_swap_chain() {
  [_view layer].contents = nil;
  _swap_index = 0;

  for (SwapBuffer &swap_buffer : _swap_chain) {
    CFRelease(swap_buffer._data_provider);
    ZB_close(swap_buffer._frame_buffer);
  }

  LVecBase2i size = get_fb_size();

  int num_buffers = get_fb_properties().get_back_buffers() + 1;
  _swap_chain.resize(num_buffers);

  for (SwapBuffer &swap_buffer : _swap_chain) {
    ZBuffer *frame_buffer = ZB_open(size[0], size[1], ZB_MODE_RGBA, 0, 0, 0, 0);
    swap_buffer._frame_buffer = frame_buffer;
    swap_buffer._data_provider =
      CGDataProviderCreateWithData(nullptr, frame_buffer->pbuf,
                                   frame_buffer->linesize * frame_buffer->ysize,
                                   nullptr);
  }
}

/**
 * Assigns the current framebuffer contents to the layer.
 */
void TinyCocoaGraphicsWindow::
do_present() {
  LVecBase2i size = get_fb_size();
  SwapBuffer &swap_buffer = _swap_chain[_swap_index];
  CGImageRef image =
    CGImageCreate(size[0], size[1], 8, 32, swap_buffer._frame_buffer->linesize,
                  _color_space, kCGBitmapByteOrder32Host | kCGImageAlphaNoneSkipFirst,
                  swap_buffer._data_provider, nullptr, false, kCGRenderingIntentDefault);

  [_view layer].contents = (id)image;
  CFRelease(image);
}

#endif  // HAVE_COCOA
