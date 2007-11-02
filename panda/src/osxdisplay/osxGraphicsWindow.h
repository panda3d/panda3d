////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
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
#include <OpenGL/glu.h>
#include <AGL/agl.h>

#define HACK_SCREEN_HASH_CONTEXT true
OSStatus aglReportError (const std::string &);

////////////////////////////////////////////////////////////////////
//       Class : osxGraphicsWindow
// Description : An interface to the osx/ system for managing GL
//               windows under X.
////////////////////////////////////////////////////////////////////
class osxGraphicsWindow : public GraphicsWindow {
public:
  osxGraphicsWindow(GraphicsPipe *pipe, 
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
  static osxGraphicsWindow * GetCurrentOSxWindow (WindowRef hint);

  void     HandleModifireDeleta(UInt32 modifiers);
  void HandleButtonDelta(UInt32 new_buttons);
  void     DoResize(void);

  OSStatus event_handler(EventHandlerCallRef myHandler, EventRef event);

  virtual void user_close_request();
  void SystemCloseWindow();
  void SystemSetWindowForground(bool forground);	
  void SystemPointToLocalPoint(Point &qdGlobalPoint);
  void LocalPointToSystemPoint(Point &qdLocalPoint);
  AGLContext get_ggs_context(void);
  AGLContext get_context(void);
  OSStatus buildGL(bool full_screen);
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
  static osxGraphicsWindow  *FullScreenWindow; 
  
#ifdef HACK_SCREEN_HASH_CONTEXT
  AGLContext _holder_aglcontext;
#endif
  CFDictionaryRef _originalMode;

  // True if _properties.get_cursor_hidden() is true.
  bool _cursor_hidden;

  // True if the cursor is actually hidden right now via system calls.
  bool _display_hide_cursor;
	 
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




inline void osxGraphicsWindow::SendKeyEvent( ButtonHandle  key, bool down)
{
    if(down)
		_input_devices[0].button_down(key);
	else
		_input_devices[0].button_up(key);
}

#endif

