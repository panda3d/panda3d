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
#include <Carbon/Carbon.h>


#define __glext_h_
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <AGL/agl.h>


#include "pandabase.h"
#include "graphicsWindow.h"
#include "buttonHandle.h"


#define HACK_SCREEN_HASH_CONTEXT true
OSStatus aglReportError (void);

////////////////////////////////////////////////////////////////////
//       Class : osxGraphicsWindow
// Description : An interface to the osx/ system for managing GL
//               windows under X.
////////////////////////////////////////////////////////////////////
class osxGraphicsWindow : public GraphicsWindow {
public:
  osxGraphicsWindow(GraphicsPipe *pipe, GraphicsStateGuardian *gsg,
                    const string &name);
  virtual ~osxGraphicsWindow();

  virtual bool move_pointer(int device, int x, int y);

  virtual void make_current();
  virtual void release_gsg();
  virtual bool begin_frame(FrameMode mode);
  virtual void end_frame(FrameMode mode);
  virtual void begin_flip();
  virtual void process_events();




private:
  void   ReleaseSystemResources();


protected:
  virtual void close_window();
  virtual bool open_window();

private:
    //
    // a singleton .. for the events to find the right pipe to push the event into
    //
    static osxGraphicsWindow  * FullScreenWindow; 

    static osxGraphicsWindow * GetCurrentOSxWindow (WindowRef hint);

public: // do not call direct ..
  OSStatus handleKeyInput (EventHandlerCallRef myHandler, EventRef event, Boolean keyDown);
  OSStatus handleTextInput (EventHandlerCallRef myHandler, EventRef event);
  OSStatus handleWindowMouseEvents (EventHandlerCallRef myHandler, EventRef event);
  ButtonHandle OSX_TranslateKey( UInt32 key,  EventRef event );

  void     HandleModifireDeleta(UInt32 modifiers);
  void     DoResize(void);


  void SystemCloseWindow();	
  void SystemSetWindowForground(bool forground);	
  void SystemPointToLocalPoint(Point &qdGlobalPoint);

  AGLContext  osxGraphicsWindow::get_ggs_context(void);
  AGLContext  get_context(void);
  OSStatus	  buildGL(void);
	
	
  inline void SendKeyEvent( ButtonHandle  key, bool down)
  {
     if(down)
		_input_devices[0].button_down(key);
	else
		_input_devices[0].button_up(key);
  }


//	inline bool IsAlive(void)
//	{
//	    return (_is_fullsreen || _osx_window != NULL);
//	}

	WindowProperties & properties() { return _properties; };
private:
	UInt32			_last_key_modifiers;
     WindowRef		_osx_window;
	bool           _is_fullsreen;

#ifdef HACK_SCREEN_HASH_CONTEXT
  AGLPixelFormat	_aglPixFmt;
  AGLContext		_aglcontext;
#endif
	CFDictionaryRef _originalMode;
	 
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


#endif

