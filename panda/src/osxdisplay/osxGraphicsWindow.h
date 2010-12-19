////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef OSXGRAPHICSWINDOW_H
#define OSXGRAPHICSWINDOW_H

#include "pandabase.h"
#include "graphicsWindow.h"
#include "buttonHandle.h"

#include <Carbon/Carbon.h>

#define __glext_h_
#include <OpenGL/gl.h>
#include <AGL/agl.h>

#define HACK_SCREEN_HASH_CONTEXT true
OSStatus report_agl_error(const string &comment);

////////////////////////////////////////////////////////////////////
//       Class : osxGraphicsWindow
// Description : An interface to the osx/ system for managing GL
//               windows under X.
////////////////////////////////////////////////////////////////////
class osxGraphicsWindow : public GraphicsWindow {
public:
  osxGraphicsWindow(GraphicsEngine *engine, GraphicsPipe *pipe, 
                    const string &name,
                    const FrameBufferProperties &fb_prop,
                    const WindowProperties &win_prop,
                    int flags,
                    GraphicsStateGuardian *gsg,
                    GraphicsOutput *host);
  virtual ~osxGraphicsWindow();

  virtual bool move_pointer(int device, int x, int y);

  virtual bool begin_frame(FrameMode mode, Thread *current_thread);
  virtual void end_frame(FrameMode mode, Thread *current_thread);
  virtual void begin_flip();
  virtual void end_flip();
  virtual void process_events();
  
  virtual bool do_reshape_request(int x_origin, int y_origin, bool has_origin,
                                  int x_size, int y_size);
  
  virtual void mouse_mode_absolute();
  virtual void mouse_mode_relative();

  virtual void set_properties_now(WindowProperties &properties);

private:
  void release_system_resources(bool destructing);
  inline void send_key_event(ButtonHandle key, bool down);

protected:
  virtual void close_window();
  virtual bool open_window();

private:
  bool os_open_window(WindowProperties &properties);

  //
  // a singleton .. for the events to find the right pipe to push the event into
  //

public: // do not call direct ..
  OSStatus handle_key_input(EventHandlerCallRef myHandler, EventRef event, 
                            Boolean keyDown);
  OSStatus handle_text_input(EventHandlerCallRef myHandler, EventRef event);
  OSStatus handle_window_mouse_events(EventHandlerCallRef myHandler, EventRef event);
  ButtonHandle osx_translate_key(UInt32 key,  EventRef event);
  static osxGraphicsWindow *get_current_osx_window(WindowRef hint);

  void handle_modifier_delta(UInt32 new_modifiers);
  void handle_button_delta(UInt32 new_buttons);
  void do_resize();

  OSStatus event_handler(EventHandlerCallRef myHandler, EventRef event);

  virtual void user_close_request();
  void system_close_window();
  void system_set_window_foreground(bool foreground);
  void system_point_to_local_point(Point &global_point);
  void local_point_to_system_point(Point &local_point);
  AGLContext get_gsg_context();
  AGLContext get_context();
  OSStatus build_gl(bool full_screen);
  bool set_icon_filename(const Filename &icon_filename);

  void set_pointer_in_window(int x, int y);
  void set_pointer_out_of_window();

private:
  UInt32 _last_key_modifiers;
  UInt32 _last_buttons;
  WindowRef _osx_window;
  bool _is_fullscreen;

  CGImageRef _pending_icon;
  CGImageRef _current_icon;
  
  int _ID;
  static osxGraphicsWindow *full_screen_window; 
  
#ifdef HACK_SCREEN_HASH_CONTEXT
  AGLContext _holder_aglcontext;
#endif
  CFDictionaryRef _originalMode;

  // True if _properties.get_cursor_hidden() is true.
  bool _cursor_hidden;

  // True if the cursor is actually hidden right now via system calls.
  bool _display_hide_cursor;

  SInt32 _wheel_hdelta;
  SInt32 _wheel_vdelta;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GraphicsWindow::init_type();
    register_type(_type_handle, "osxGraphicsWindow",
                  GraphicsWindow::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "osxGraphicsWindow.I"

#endif

