/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file x11GraphicsWindow.h
 * @author rdb
 * @date 2009-07-07
 */

#ifndef X11GRAPHICSWINDOW_H
#define X11GRAPHICSWINDOW_H

#include "pandabase.h"

#include "x11GraphicsPipe.h"
#include "graphicsWindow.h"
#include "buttonHandle.h"

/**
 * Interfaces to the X11 window system.
 */
class x11GraphicsWindow : public GraphicsWindow {
public:
  x11GraphicsWindow(GraphicsEngine *engine, GraphicsPipe *pipe,
                    const std::string &name,
                    const FrameBufferProperties &fb_prop,
                    const WindowProperties &win_prop,
                    int flags,
                    GraphicsStateGuardian *gsg,
                    GraphicsOutput *host);
  virtual ~x11GraphicsWindow();

  virtual MouseData get_pointer(int device) const;
  virtual bool move_pointer(int device, int x, int y);

  virtual void clear(Thread *current_thread);
  virtual bool begin_frame(FrameMode mode, Thread *current_thread);
  virtual void end_frame(FrameMode mode, Thread *current_thread);

  virtual void process_events();
  virtual void set_properties_now(WindowProperties &properties);

  INLINE X11_Window get_xwindow() const;

protected:
  virtual void close_window();
  virtual bool open_window();

  virtual void mouse_mode_absolute();
  virtual void mouse_mode_relative();

  void set_wm_properties(const WindowProperties &properties,
                         bool already_mapped);

  virtual void setup_colormap(XVisualInfo *visual);
  void handle_keystroke(XKeyEvent &event);
  void handle_keypress(XKeyEvent &event);
  void handle_keyrelease(XKeyEvent &event);

  ButtonHandle get_button(XKeyEvent &key_event, bool allow_shift);
  ButtonHandle map_button(KeySym key) const;
  ButtonHandle map_raw_button(KeyCode key) const;
  ButtonHandle get_mouse_button(XButtonEvent &button_event);
  virtual ButtonMap *get_keyboard_map() const;

  static Bool check_event(X11_Display *display, XEvent *event, char *arg);

  void open_raw_mice();

private:
  X11_Cursor get_cursor(const Filename &filename);
  X11_Cursor read_ico(std::istream &ico);

protected:
  X11_Display *_display;
  int _screen;
  X11_Window _xwindow;
  Colormap _colormap;
  XIC _ic;
  XVisualInfo *_visual_info;
  Rotation _orig_rotation;
  SizeID _orig_size_id;

  LVecBase2i _fixed_size;

  GraphicsWindowInputDevice *_input;

  long _event_mask;
  bool _awaiting_configure;
  bool _dga_mouse_enabled;
  Bool _override_redirect;
  Atom _wm_delete_window;

  x11GraphicsPipe::pfn_XRRGetScreenInfo _XRRGetScreenInfo;
  x11GraphicsPipe::pfn_XRRSetScreenConfig _XRRSetScreenConfig;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GraphicsWindow::init_type();
    register_type(_type_handle, "x11GraphicsWindow",
                  GraphicsWindow::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  // Since the Panda API requests icons and cursors by filename, we need a
  // table mapping filenames to handles, so we can avoid re-reading the file
  // each time we change icons.
  pmap<Filename, X11_Cursor> _cursor_filenames;
};

#include "x11GraphicsWindow.I"

#endif
