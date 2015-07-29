// Filename: tinyOsxGraphicsWindow.h
// Created by:  drose (12May08)
//
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

#ifndef TINYOSXGRAPHICSWINDOW_H
#define TINYOSXGRAPHICSWINDOW_H

#include "pandabase.h"

#if defined(IS_OSX) && !defined(BUILD_IPHONE) && defined(HAVE_CARBON) && !__LP64__

#include <Carbon/Carbon.h>

#include "graphicsWindow.h"
#include "buttonHandle.h"
#include "tinyGraphicsStateGuardian.h"

////////////////////////////////////////////////////////////////////
//       Class : TinyOsxGraphicsWindow
// Description : Opens a window on OS X to display the TinyPanda
//               software rendering.
////////////////////////////////////////////////////////////////////
class TinyOsxGraphicsWindow : public GraphicsWindow {
public:
  TinyOsxGraphicsWindow(GraphicsEngine *engine, GraphicsPipe *pipe, 
                        const string &name,
                        const FrameBufferProperties &fb_prop,
                        const WindowProperties &win_prop,
                        int flags,
                        GraphicsStateGuardian *gsg,
                        GraphicsOutput *host);
  virtual ~TinyOsxGraphicsWindow();

  virtual bool move_pointer(int device, int x, int y);

  virtual bool begin_frame(FrameMode mode, Thread *current_thread);
  virtual void end_frame(FrameMode mode, Thread *current_thread);
  virtual void begin_flip();
  virtual void process_events();
  virtual bool supports_pixel_zoom() const;
  
  virtual bool do_reshape_request(int x_origin, int y_origin, bool has_origin,
                                  int x_size, int y_size);
  
  virtual void mouse_mode_absolute();
  virtual void mouse_mode_relative();

  virtual void set_properties_now(WindowProperties &properties);

private:
  void   ReleaseSystemResources();
  inline void SendKeyEvent( ButtonHandle  key, bool down);

protected:
  virtual void close_window();
  virtual bool open_window();

private:

  bool OSOpenWindow(WindowProperties &properties);

    //
    // a singleton .. for the events to find the right pipe to push the event into
    //


public: // do not call direct ..
  OSStatus handleKeyInput (EventHandlerCallRef myHandler, EventRef event, Boolean keyDown);
  OSStatus handleTextInput (EventHandlerCallRef myHandler, EventRef event);
  OSStatus handleWindowMouseEvents (EventHandlerCallRef myHandler, EventRef event);
  ButtonHandle OSX_TranslateKey( UInt32 key,  EventRef event );
  static TinyOsxGraphicsWindow * GetCurrentOSxWindow (WindowRef hint);

  void     HandleModifireDeleta(UInt32 modifiers);
  void HandleButtonDelta(UInt32 new_buttons);
  void     DoResize(void);

  OSStatus event_handler(EventHandlerCallRef myHandler, EventRef event);

  virtual void user_close_request();
  void SystemCloseWindow();
  void SystemSetWindowForground(bool forground);
  void SystemPointToLocalPoint(Point &qdGlobalPoint);
  void LocalPointToSystemPoint(Point &qdLocalPoint);
  bool set_icon_filename(const Filename &icon_filename);

  void set_pointer_in_window(int x, int y);
  void set_pointer_out_of_window();

private:
  void create_frame_buffer();

private:
  ZBuffer *_frame_buffer;

private:
  UInt32 _last_key_modifiers;
  UInt32 _last_buttons;
  WindowRef _osx_window;
  bool _is_fullscreen;

  CGImageRef _pending_icon;
  CGImageRef _current_icon;
  
  int _ID;
  static TinyOsxGraphicsWindow  *FullScreenWindow; 
  
  CFDictionaryRef _originalMode;

  // True if _properties.get_cursor_hidden() is true.
  bool _cursor_hidden;

  // True if the cursor is actually hidden right now via system calls.
  bool _display_hide_cursor;

  SInt32 _wheel_delta;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GraphicsWindow::init_type();
    register_type(_type_handle, "TinyOsxGraphicsWindow",
                  GraphicsWindow::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "tinyOsxGraphicsWindow.I"

#endif  // IS_OSX

#endif

