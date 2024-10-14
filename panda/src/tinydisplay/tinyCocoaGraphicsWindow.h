/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file tinyCocoaGraphicsWindow.h
 * @author rdb
 * @date 2023-03-21
 */

#ifndef TINYCOCOAGRAPHICSWINDOW_H
#define TINYCOCOAGRAPHICSWINDOW_H

#include "pandabase.h"

#ifdef HAVE_COCOA

#include "tinyCocoaGraphicsPipe.h"
#include "cocoaGraphicsWindow.h"
#include "small_vector.h"

/**
 * Opens a window on macOS to display the TinyPanda software rendering.
 */
class EXPCL_TINYDISPLAY TinyCocoaGraphicsWindow : public CocoaGraphicsWindow {
public:
  TinyCocoaGraphicsWindow(GraphicsEngine *engine, GraphicsPipe *pipe,
                          const std::string &name,
                          const FrameBufferProperties &fb_prop,
                          const WindowProperties &win_prop,
                          int flags,
                          GraphicsStateGuardian *gsg,
                          GraphicsOutput *host);
  virtual ~TinyCocoaGraphicsWindow();

  virtual bool begin_frame(FrameMode mode, Thread *current_thread);
  virtual void end_frame(FrameMode mode, Thread *current_thread);
  virtual void end_flip();
  virtual bool supports_pixel_zoom() const;

  virtual void process_events();

protected:
  virtual void close_window();
  virtual bool open_window();
  virtual void pixel_factor_changed();

private:
  void create_swap_chain();
  void do_present();

private:
  CGColorSpaceRef _color_space = nil;

  struct SwapBuffer {
    ZBuffer *_frame_buffer = nullptr;
    CGDataProviderRef _data_provider = nil;
  };

  small_vector<SwapBuffer, 2> _swap_chain;
  int _swap_index = 0;
  uint32_t _vsync_counter = 0;
  bool _vsync_enabled = false;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    CocoaGraphicsWindow::init_type();
    register_type(_type_handle, "TinyCocoaGraphicsWindow",
                  CocoaGraphicsWindow::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "tinyCocoaGraphicsWindow.I"

#endif  // HAVE_COCOA

#endif
