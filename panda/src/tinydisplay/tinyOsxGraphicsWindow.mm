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

#include "pandabase.h"

#if defined(IS_OSX) && !defined(BUILD_IPHONE) && defined(HAVE_CARBON) && !__LP64__

#include <Carbon/Carbon.h>
#include <Cocoa/Cocoa.h>

#include "tinyOsxGraphicsWindow.h"
#include "config_tinydisplay.h"
#include "tinyOsxGraphicsPipe.h"
#include "pStatTimer.h"
#include "keyboardButton.h"
#include "mouseButton.h"
#include "tinyGraphicsStateGuardian.h"
#include "throw_event.h"
#include "pnmImage.h"
#include "virtualFileSystem.h"
#include "config_util.h"

#include <ApplicationServices/ApplicationServices.h>

#include "pmutex.h"
//#include "mutexHolder.h"

////////////////////////////////////

\
Mutex &  OSXGloablMutex() {
    static   Mutex  m("OSXWIN_Mutex");
    return m;
}


struct work1
{
    volatile bool work_done;
};

#define PANDA_CREATE_WINDOW  101
static void Post_Event_Wait(unsigned short type, unsigned int data1 , unsigned int data2 , int target_window ) {
    work1 w;
    w.work_done = false;
    NSEvent *ev = [NSEvent otherEventWithType:NSApplicationDefined
                                     location:NSZeroPoint
                                modifierFlags:0
                                    timestamp:0.0f
                                 windowNumber:target_window
                                      context:nil
                                      subtype:type
                                        data1:data1
                                        data2:(int)&w];

    [NSApp postEvent:ev atStart:NO];
    while (!w.work_done)
        usleep(10);

}



////////////////////////// Global Objects .....

TypeHandle TinyOsxGraphicsWindow::_type_handle;
TinyOsxGraphicsWindow  * TinyOsxGraphicsWindow::FullScreenWindow = NULL;


#define USER_CONTAINER

#include <set>

#ifdef USER_CONTAINER

std::set< WindowRef >     MyWindows;
void AddAWindow( WindowRef window) {
     MyWindows.insert(window);
}
bool checkmywindow(WindowRef window) {
   return MyWindows.find(window) != MyWindows.end();
}
#else

void AddAWindow( WindowRef window) {
}

bool checkmywindow(WindowRef window) {
    return true;
}

#endif



////////////////////////////////////////////////////////////////////
//     Function: GetCurrentOSxWindow
//       Access: Static,
//  Description: How to find the active window for events  on osx..
//
////////////////////////////////////////////////////////////////////
TinyOsxGraphicsWindow* TinyOsxGraphicsWindow::GetCurrentOSxWindow(WindowRef window) {
    if (FullScreenWindow != NULL)
       return FullScreenWindow;

    if (NULL == window)  // HID use this path
    {
        //    Assume first we are a child window. If we cant find a window of that class, then we
        //    are standalone and can jsut grab the front window.
        window    =    GetFrontWindowOfClass(kSimpleWindowClass, TRUE);
        if (NULL == window)
            window    =    FrontNonFloatingWindow();
    }

    if (window  && checkmywindow(window)) {
      return (TinyOsxGraphicsWindow *)GetWRefCon (window);
    } else {
      return NULL;
    }
}

////////////////////////////////////////////////////////////////////
//     Function: TinyOsxGraphicsWindow::event_handler
//       Access: Public
//  Description: The standard window event handler for non-fullscreen
//               windows.
////////////////////////////////////////////////////////////////////
OSStatus TinyOsxGraphicsWindow::
event_handler(EventHandlerCallRef myHandler, EventRef event) {

  OSStatus result = eventNotHandledErr;
  UInt32 the_class = GetEventClass(event);
  UInt32 kind = GetEventKind(event);

  WindowRef window = NULL;
  GetEventParameter(event, kEventParamDirectObject, typeWindowRef, NULL,
                    sizeof(window), NULL, &window);

  UInt32 attributes = 0;
  GetEventParameter(event, kEventParamAttributes, typeUInt32, NULL,
                    sizeof(attributes), NULL, &attributes);

  if (tinydisplay_cat.is_spam()) {
    tinydisplay_cat.spam() << ClockObject::get_global_clock()->get_real_time() << " event_handler: " << (void *)this << ", " << window << ", " << the_class << ", " << kind << "\n";
  }

  switch (the_class) {
  case kEventClassMouse:
    result  = handleWindowMouseEvents (myHandler, event);
    break;
    
  case kEventClassWindow:
    switch (kind) {
        case kEventWindowCollapsing:
          /*
          Rect r;
          GetWindowPortBounds (window, &r);
          CompositeGLBufferIntoWindow( get_context(), &r, GetWindowPort (window));
          UpdateCollapsedWindowDockTile (window);
          */
          SystemSetWindowForground(false);
          break;
        case kEventWindowActivated: // called on click activation and initially
          SystemSetWindowForground(true);
          DoResize();
          break;
        case kEventWindowDeactivated:
          SystemSetWindowForground(false);
          break;
        case kEventWindowClose: // called when window is being closed (close box)
          // This is a message from the window manager indicating that
          // the user has requested to close the window.
          user_close_request();
          result = noErr;
          break;
        case kEventWindowShown: // called on initial show (not on un-minimize)
          if (window == FrontNonFloatingWindow ())
            SetUserFocusWindow (window);
          break;

      case kEventWindowBoundsChanging:
        // Gives us a chance to intercept resize attempts
        if (attributes & kWindowBoundsChangeSizeChanged) {
          // If the window is supposed to be fixed-size, enforce this.
          if (_properties.get_fixed_size()) {
            Rect bounds;
            GetEventParameter(event, kEventParamCurrentBounds,
                              typeQDRectangle, NULL, sizeof(bounds), NULL, &bounds);
            bounds.right = bounds.left + _properties.get_x_size();
            bounds.bottom = bounds.top + _properties.get_y_size();
            SetEventParameter(event, kEventParamCurrentBounds,
                              typeQDRectangle, sizeof(bounds), &bounds);                   result = noErr;
          }
        }
        break;
        
    case kEventWindowBoundsChanged: // called for resize and moves (drag)
      DoResize();
      break;
    case kEventWindowZoomed:
      break;
    case kEventWindowCollapsed:
      {
        WindowProperties properties;
        properties.set_minimized(true);
        system_changed_properties(properties);
      }
      break;
    case kEventWindowExpanded:
      {
        WindowProperties properties;
        properties.set_minimized(false);
        system_changed_properties(properties);
      }
      break;
    }
    break;
  }

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: TinyOsxGraphicsWindow::user_close_request
//       Access: Private
//  Description: The user has requested to close the window, for
//               instance with Cmd-W, or by clicking on the close
//               button.
////////////////////////////////////////////////////////////////////
void TinyOsxGraphicsWindow::user_close_request() {
  string close_request_event = get_close_request_event();
  if (!close_request_event.empty()) {
    // In this case, the app has indicated a desire to intercept the request and process it directly.
    throw_event(close_request_event);
  } else {
    // In this case, the default case, the app does not intend to service the request, so we do by closing the window.
    close_window();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TinyOsxGraphicsWindow::SystemCloseWindow
//       Access: private
//  Description: The Windows is closed by a OS resource not by a internal request
//
////////////////////////////////////////////////////////////////////
void TinyOsxGraphicsWindow::SystemCloseWindow() {
    if (tinydisplay_cat.is_debug())
        tinydisplay_cat.debug() << "System Closing Window \n";
    ReleaseSystemResources();
};

////////////////////////////////////////////////////////////////////
//     Function: windowEvtHndlr
//       Access: file scope static
//  Description: The C callback for Window Events ..
//
//  We only hook this up for non fullscreen window... so we only
//  handle system window events..
//
////////////////////////////////////////////////////////////////////
static pascal OSStatus    windowEvtHndlr(EventHandlerCallRef myHandler, EventRef event, void *userData) {
#pragma unused (userData)

//  volatile().lock();

  WindowRef window = NULL;
  GetEventParameter(event, kEventParamDirectObject, typeWindowRef, NULL, sizeof(WindowRef), NULL, &window);

  if (window != NULL) {
    TinyOsxGraphicsWindow *osx_win = TinyOsxGraphicsWindow::GetCurrentOSxWindow(window);
    if (osx_win != (TinyOsxGraphicsWindow *)NULL) {
      //OSXGloablMutex().release();
      return osx_win->event_handler(myHandler, event);
    }
  }

  //OSXGloablMutex().release();
  return eventNotHandledErr;
}

///////////////////////////////////////////////////////////////////
//     Function: TinyOsxGraphicsWindow::DoResize
//       Access:
//  Description: The C callback for Window Events ..
//
//   We only hook this up for none fullscreen window... so we only handle system window events..
//
////////////////////////////////////////////////////////////////////
void TinyOsxGraphicsWindow::DoResize(void) {
  tinydisplay_cat.info() << "In Resize....." << _properties << "\n";

  // only in window mode .. not full screen
  if (_osx_window != NULL && !_is_fullscreen && _properties.has_size()) {
    Rect                rectPort = {0,0,0,0};
    CGRect                 viewRect = {{0.0f, 0.0f}, {0.0f, 0.0f}};

    GetWindowPortBounds (_osx_window, &rectPort);
    viewRect.size.width = (PN_stdfloat) (rectPort.right - rectPort.left);
    viewRect.size.height = (PN_stdfloat) (rectPort.bottom - rectPort.top);
    // tell panda
    WindowProperties properties;
    properties.set_size((int)viewRect.size.width,(int)viewRect.size.height);
    properties.set_origin((int) rectPort.left,(int)rectPort.top);
    system_changed_properties(properties);
    ZB_resize(_frame_buffer, NULL, _properties.get_x_size(), _properties.get_y_size());

    if (tinydisplay_cat.is_debug()) {
      tinydisplay_cat.debug() << " Resizing Window " << viewRect.size.width << " " << viewRect.size.height << "\n";
    }
  }
};

///////////////////////////////////////////////////////////////////
//     Function: appEvtHndlr
//       Access:
//  Description: The C callback for APlication Events..
//
//    Hooked  once for application
//
////////////////////////////////////////////////////////////////////
static pascal OSStatus appEvtHndlr (EventHandlerCallRef myHandler, EventRef event, void* userData) {
#pragma unused (myHandler)
  OSStatus result = eventNotHandledErr;
  {

  //OSXGloablMutex().lock();

  TinyOsxGraphicsWindow *osx_win = NULL;
  WindowRef window = NULL;
  UInt32 the_class = GetEventClass (event);
  UInt32 kind = GetEventKind (event);

  GetEventParameter(event, kEventParamWindowRef, typeWindowRef, NULL, sizeof(WindowRef), NULL, (void*) &window);
  osx_win = TinyOsxGraphicsWindow::GetCurrentOSxWindow(window);
  if (osx_win == NULL) {
    //OSXGloablMutex().release();
    return eventNotHandledErr;
  }

  switch (the_class) {
    case kEventClassTextInput:
      if (kind == kEventTextInputUnicodeForKeyEvent) {
	osx_win->handleTextInput(myHandler, event);
      }
      //result = noErr;
      //
      // can not report handled .. the os will not sent the raw key strokes then
      //    if (osx_win->handleTextInput(myHandler, event) == noErr)
      //          result = noErr;
      break;
    case kEventClassKeyboard:
      {
      switch (kind) {
	case kEventRawKeyRepeat:
	case kEventRawKeyDown:
	    result = osx_win->handleKeyInput  (myHandler, event, true);
	    break;
	case kEventRawKeyUp:
	    result = osx_win->handleKeyInput  (myHandler, event, false);
	    break;
        case kEventRawKeyModifiersChanged:
	  {
	    UInt32 newModifiers;
	    OSStatus error = GetEventParameter(event, kEventParamKeyModifiers,typeUInt32, NULL,sizeof(UInt32), NULL, &newModifiers);
	    if (error == noErr) {
	      osx_win->HandleModifireDeleta(newModifiers);
	      result = noErr;
	    }
	  }
	  break;
        }
      }
      break;

    case kEventClassMouse:
	    // tinydisplay_cat.info() << "Mouse movement handled by Application handler\n";
      //if (TinyOsxGraphicsWindow::FullScreenWindow != NULL)
	  result = osx_win->handleWindowMouseEvents (myHandler, event);
       //result = noErr;
      break;
  }

  //OSXGloablMutex().release();
  }

  return result;
}

///////////////////////////////////////////////////////////////////
//     Function: TinyOsxGraphicsWindow::handleTextInput
//       Access:
//  Description:  Trap Unicode  Input.
//
//
////////////////////////////////////////////////////////////////////
OSStatus TinyOsxGraphicsWindow::handleTextInput (EventHandlerCallRef myHandler, EventRef theTextEvent) {
  UniChar      *text = NULL;
  UInt32       actualSize = 0;

  OSStatus ret = GetEventParameter (theTextEvent, kEventParamTextInputSendText, typeUnicodeText, NULL, 0, &actualSize, NULL);
  if (ret != noErr) {
    return ret;
  }

  text = (UniChar*)NewPtr(actualSize);
  if (text!= NULL) {
    ret = GetEventParameter (theTextEvent, kEventParamTextInputSendText,typeUnicodeText, NULL, actualSize, NULL, text);
    if (ret != noErr) {
      return ret;
    }

    for (unsigned int x = 0; x < actualSize/sizeof(UniChar); ++x) {
      _input_devices[0].keystroke(text[x]);
    }
    DisposePtr((char *)text);
  }

  return ret;
}
///////////////////////////////////////////////////////////////////
//     Function: TinyOsxGraphicsWindow::ReleaseSystemResources
//       Access: private..
//  Description: Clean up the OS level messes..
////////////////////////////////////////////////////////////////////
void TinyOsxGraphicsWindow::ReleaseSystemResources() {
  if (_is_fullscreen) {
    _is_fullscreen = false;
    FullScreenWindow = NULL;

    if (_originalMode != NULL)
      CGDisplaySwitchToMode( kCGDirectMainDisplay, _originalMode );

    CGDisplayRelease( kCGDirectMainDisplay );

    _originalMode = NULL;
  }

  if (_osx_window != NULL) {
    SetWRefCon (_osx_window, (long int) NULL);
    HideWindow (_osx_window);
    DisposeWindow(_osx_window);
    _osx_window = NULL;
  }

  if (_pending_icon != NULL) {
    CGImageRelease(_pending_icon);
    _pending_icon = NULL;
  }
  if (_current_icon != NULL) {
    CGImageRelease(_current_icon);
    _current_icon = NULL;
  }

  WindowProperties properties;
  properties.set_foreground(false);
  properties.set_open(false);
  properties.set_cursor_filename(Filename());
  system_changed_properties(properties);

  _is_fullscreen = false;
  _osx_window = NULL;
}


static int id_seed = 100;

////////////////////////////////////////////////////////////////////
//     Function: TinyOsxGraphicsWindow::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
TinyOsxGraphicsWindow::TinyOsxGraphicsWindow(GraphicsEngine *engine, GraphicsPipe *pipe,
                                             const string &name,
                                             const FrameBufferProperties &fb_prop,
                                             const WindowProperties &win_prop,
                                             int flags,
                                             GraphicsStateGuardian *gsg,
                                             GraphicsOutput *host) :
  GraphicsWindow(engine, pipe, name, fb_prop, win_prop, flags, gsg, host),
  _osx_window(NULL),
  _is_fullscreen(false),
  _pending_icon(NULL),
  _current_icon(NULL),
  _ID(id_seed++),
  _originalMode(NULL) {
 GraphicsWindowInputDevice device =
    GraphicsWindowInputDevice::pointer_and_keyboard(this, "keyboard/mouse");
  _input_devices.push_back(device);
  _input_devices[0].set_pointer_in_window(0, 0);
  _last_key_modifiers = 0;
  _last_buttons = 0;

  _cursor_hidden = false;
  _display_hide_cursor = false;
  _wheel_delta = 0;

  _frame_buffer = NULL;
  update_pixel_factor();

  if (tinydisplay_cat.is_debug())
    tinydisplay_cat.debug() << "TinyOsxGraphicsWindow::TinyOsxGraphicsWindow() -" <<_ID << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: TinyOsxGraphicsWindow::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
TinyOsxGraphicsWindow::~TinyOsxGraphicsWindow() {
  if (tinydisplay_cat.is_debug())
    tinydisplay_cat.debug() << "TinyOsxGraphicsWindow::~TinyOsxGraphicsWindow() -" <<_ID << "\n";

  // Make sure the window callback won't come back to this
  // (destructed) object any more.
  if (_osx_window) {
    SetWRefCon (_osx_window, (long) NULL);
  }

  ReleaseSystemResources();
}

////////////////////////////////////////////////////////////////////
//     Function: TinyOsxGraphicsWindow::set_icon_filename
//       Access: Private
//  Description: Called internally to load up an icon file that should
//               be applied to the window.  Returns true on success,
//               false on failure.
////////////////////////////////////////////////////////////////////
bool TinyOsxGraphicsWindow::set_icon_filename(const Filename &icon_filename) {
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

  Filename icon_pathname = icon_filename;
  if (!vfs->resolve_filename(icon_pathname, get_model_path())) {
    // The filename doesn't exist along the search path.
    if (icon_pathname.is_fully_qualified() && vfs->exists(icon_pathname)) {
      // But it does exist locally, so accept it.
    } else {
      tinydisplay_cat.warning()
        << "Could not find icon filename " << icon_filename << "\n";
      return false;
    }
  }


  PNMImage pnmimage;
  if (!pnmimage.read(icon_pathname)) {
    tinydisplay_cat.warning()
      << "Could not read icon filename " << icon_pathname << "\n";
    return false;
  }

  CGImageRef icon_image = TinyOsxGraphicsPipe::create_cg_image(pnmimage);
  if (icon_image == NULL) {
    return false;
  }

  if (_pending_icon != NULL) {
    CGImageRelease(_pending_icon);
    _pending_icon = NULL;
  }
  _pending_icon = icon_image;
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: TinyOsxGraphicsWindow::set_pointer_in_window
//       Access: Private
//  Description: Indicates the mouse pointer is seen within the
//               window.
////////////////////////////////////////////////////////////////////
void TinyOsxGraphicsWindow::
set_pointer_in_window(int x, int y) {
  _input_devices[0].set_pointer_in_window(x, y);

  if (_cursor_hidden != _display_hide_cursor) {
    if (_cursor_hidden) {
      CGDisplayHideCursor(kCGDirectMainDisplay);
      _display_hide_cursor = true;
    } else {
      CGDisplayShowCursor(kCGDirectMainDisplay);
      _display_hide_cursor = false;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TinyOsxGraphicsWindow::set_pointer_out_of_window
//       Access: Private
//  Description: Indicates the mouse pointer is no longer within the
//               window.
////////////////////////////////////////////////////////////////////
void TinyOsxGraphicsWindow::
set_pointer_out_of_window() {
  _input_devices[0].set_pointer_out_of_window();

  if (_display_hide_cursor) {
    CGDisplayShowCursor(kCGDirectMainDisplay);
    _display_hide_cursor = false;
  }
}


////////////////////////////////////////////////////////////////////
//     Function: TinyOsxGraphicsWindow::begin_frame
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               before beginning rendering for a given frame.  It
//               should do whatever setup is required, and return true
//               if the frame should be rendered, or false if it
//               should be skipped.
////////////////////////////////////////////////////////////////////
bool TinyOsxGraphicsWindow::begin_frame(FrameMode mode, Thread *current_thread) {
  PStatTimer timer(_make_current_pcollector);

  begin_frame_spam(mode);
  if (_gsg == (GraphicsStateGuardian *)NULL ||
      (_osx_window == NULL && !_is_fullscreen)) {
    // not powered up .. just abort..
    return false;
  }

  // Now is a good time to apply the icon change that may have
  // recently been requested.  By this point, we should be able to get
  // a handle to the dock context.
  if (_pending_icon != NULL) {
    CGContextRef context = BeginCGContextForApplicationDockTile();
    if (context != NULL) {
      SetApplicationDockTileImage(_pending_icon);
      EndCGContextForApplicationDockTile(context);

      if (_current_icon != NULL) {
        CGImageRelease(_current_icon);
        _current_icon = NULL;
      }
      _current_icon = _pending_icon;
      _pending_icon = NULL;
    }
  }

  TinyGraphicsStateGuardian *tinygsg;
  DCAST_INTO_R(tinygsg, _gsg, false);

  tinygsg->_current_frame_buffer = _frame_buffer;
  tinygsg->reset_if_new();

  _gsg->set_current_properties(&get_fb_properties());
  return _gsg->begin_frame(current_thread);
}

////////////////////////////////////////////////////////////////////
//     Function: TinyOsxGraphicsWindow::end_frame
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               after rendering is completed for a given frame.  It
//               should do whatever finalization is required.
////////////////////////////////////////////////////////////////////
void TinyOsxGraphicsWindow::end_frame(FrameMode mode, Thread *current_thread) {
  end_frame_spam(mode);

  _gsg->end_frame(current_thread);

  if (mode == FM_render) {

    copy_to_textures();

    trigger_flip();
   clear_cube_map_selection();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TinyOsxGraphicsWindow::begin_flip
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               after end_frame() has been called on all windows, to
//               initiate the exchange of the front and back buffers.
//
//               This should instruct the window to prepare for the
//               flip at the next video sync, but it should not wait.
//
//               We have the two separate functions, begin_flip() and
//               end_flip(), to make it easier to flip all of the
//               windows at the same time.
////////////////////////////////////////////////////////////////////
void TinyOsxGraphicsWindow::begin_flip() {
  if (_osx_window == NULL) {
    return;
  }

  QDErr err;

  // blit rendered framebuffer into window backing store
  Rect src_rect = {0, 0, _frame_buffer->ysize, _frame_buffer->xsize};
  Rect ddrc_rect = {0, 0, _frame_buffer->ysize, _frame_buffer->xsize};
  if (get_pixel_factor() != 1.0) {
    src_rect.right = get_fb_x_size();
    src_rect.bottom = get_fb_y_size();
  }

  // create a GWorld containing our image
  GWorldPtr pGWorld;
  err = NewGWorldFromPtr(&pGWorld, k32BGRAPixelFormat, &src_rect, 0, 0, 0,
                         (char *)_frame_buffer->pbuf, _frame_buffer->linesize);
  if (err != noErr) {
    tinydisplay_cat.error()
      << " error in NewGWorldFromPtr, called from begin_flip()\n";
    return;
  }

  GrafPtr out_port = GetWindowPort(_osx_window);
  GrafPtr portSave = NULL;
  Boolean portChanged = QDSwapPort(out_port, &portSave);

  {
    static PStatCollector b2("Wait:Flip:Begin:CopyBits");
    PStatTimer t2(b2);
    CopyBits(GetPortBitMapForCopyBits(pGWorld),
             GetPortBitMapForCopyBits(out_port),
             &src_rect, &ddrc_rect, srcCopy, 0);
  }

  if (portChanged) {
    QDSwapPort(portSave, NULL);
  }

  DisposeGWorld(pGWorld);
}

////////////////////////////////////////////////////////////////////
//     Function: TinyOsxGraphicsWindow::close_window
//       Access: Protected, Virtual
//  Description: Closes the window right now.  Called from the window
//               thread.
////////////////////////////////////////////////////////////////////
void TinyOsxGraphicsWindow::close_window() {
  SystemCloseWindow();

  WindowProperties properties;
  properties.set_open(false);
  system_changed_properties(properties);

  ReleaseSystemResources();
  _gsg.clear();
 GraphicsWindow::close_window();
}

//////////////////////////////////////////////////////////
// HACK ALLERT ************ Undocumented OSX calls...
// I can not find any other way to get the mouse focus to a window in OSX..
//
//extern "C" {
//    struct  CPSProcessSerNum
//    {
//        UInt32 lo;
//        UInt32 hi;
//    };
///
//extern OSErr CPSGetCurrentProcess(CPSProcessSerNum *psn);
//extern OSErr CPSEnableForegroundOperation( struct CPSProcessSerNum *psn);
//extern OSErr CPSSetProcessName ( struct CPSProcessSerNum *psn, char *processname);
//extern OSErr CPSSetFrontProcess( struct CPSProcessSerNum *psn);
//};

////////////////////////////////////////////////////////////////////
//     Function: TinyOsxGraphicsWindow::open_window
//       Access: Protected, Virtual
//  Description: Opens the window right now.  Called from the window
//               thread.  Returns true if the window is successfully
//               opened, or false if there was a problem.
////////////////////////////////////////////////////////////////////
bool TinyOsxGraphicsWindow::open_window() {
  // GSG Creation/Initialization
  TinyGraphicsStateGuardian *tinygsg;
  if (_gsg == 0) {
    // There is no old gsg.  Create a new one.
    tinygsg = new TinyGraphicsStateGuardian(_engine, _pipe, NULL);
    _gsg = tinygsg;
  } else {
    DCAST_INTO_R(tinygsg, _gsg, false);
  }

  create_frame_buffer();
  if (_frame_buffer == NULL) {
    tinydisplay_cat.error()
      << "Could not create frame buffer.\n";
    return false;
  }

  tinygsg->_current_frame_buffer = _frame_buffer;

  tinygsg->reset_if_new();
  if (!tinygsg->is_valid()) {
    close_window();
    return false;
  }

  WindowProperties req_properties  = _properties;
  //OSXGloablMutex().lock();
  bool answer =  OSOpenWindow(req_properties);
  //OSXGloablMutex().release();
  return answer;
}


bool TinyOsxGraphicsWindow::OSOpenWindow(WindowProperties &req_properties) {
  OSErr err = noErr;

  if (_current_icon != NULL && _pending_icon == NULL) {
      // If we already have an icon specified, we'll need to reapply it
      // when the window is successfully created.
      _pending_icon = _current_icon;
      _current_icon = NULL;
  }

  static bool GlobalInits = false;
  if (!GlobalInits) {
    //
    // one time aplication inits.. to get a window open from a standalone aplication..

    EventHandlerRef    application_event_ref_ref1;
    EventTypeSpec    list1[] =
      {
        //{ kEventClassCommand,  kEventProcessCommand },
        //{ kEventClassCommand,  kEventCommandUpdateStatus },
        { kEventClassMouse, kEventMouseDown },// handle trackball functionality globaly because there is only a single user
        { kEventClassMouse, kEventMouseUp },
        { kEventClassMouse, kEventMouseMoved },
        { kEventClassMouse, kEventMouseDragged },
        { kEventClassMouse, kEventMouseWheelMoved } ,
        { kEventClassKeyboard, kEventRawKeyDown },
        { kEventClassKeyboard, kEventRawKeyUp } ,
        { kEventClassKeyboard, kEventRawKeyRepeat },
        { kEventClassKeyboard, kEventRawKeyModifiersChanged }    ,
        { kEventClassTextInput,    kEventTextInputUnicodeForKeyEvent},
      };

    EventHandlerUPP    gEvtHandler = NewEventHandlerUPP(appEvtHndlr);
    err = InstallApplicationEventHandler (gEvtHandler, GetEventTypeCount (list1) , list1, this, &application_event_ref_ref1 );
    GlobalInits = true;

    ProcessSerialNumber psn = { 0, kCurrentProcess };

    // Determine if we're running from a bundle.
    CFDictionaryRef dref =
      ProcessInformationCopyDictionary(&psn, kProcessDictionaryIncludeAllInformationMask);
    // If the dictionary doesn't have "BundlePath" (or the BundlePath
    // is the same as the executable path), then we're not running
    // from a bundle, and we need to call TransformProcessType to make
    // the process a "foreground" application, with its own icon in
    // the dock and such.

    bool has_bundle = false;

    CFStringRef bundle_path = (CFStringRef)CFDictionaryGetValue(dref, CFSTR("BundlePath"));
    if (bundle_path != NULL) {
      // OK, we have a bundle path.  We're probably running in a
      // bundle . . .
      has_bundle = true;

      // . . . unless it turns out it's the same as the executable
      // path.
      CFStringRef exe_path = (CFStringRef)CFDictionaryGetValue(dref, kCFBundleExecutableKey);
      if (exe_path != NULL) {
        if (CFStringCompare(bundle_path, exe_path, kCFCompareCaseInsensitive) == kCFCompareEqualTo) {
          has_bundle = false;
        }
        CFRelease(exe_path);
      }

      CFRelease(bundle_path);
    }

    if (!has_bundle) {
      TransformProcessType(&psn, kProcessTransformToForegroundApplication);
    }
    SetFrontProcess(&psn);
  }


  bool wants_fullscreen = req_properties.has_fullscreen() && req_properties.get_fullscreen();
  if (req_properties.get_minimized()) {
    // A minimized window can't be fullscreen.
    wants_fullscreen = false;
  }
  
  if (wants_fullscreen) {
   tinydisplay_cat.info() << "Creating full screen\n";

    // capture the main display
    CGDisplayCapture(kCGDirectMainDisplay);
    // if sized try and switch it..
    if (req_properties.has_size()) {
      _originalMode = CGDisplayCurrentMode(kCGDirectMainDisplay);
      CFDictionaryRef newMode = CGDisplayBestModeForParameters(kCGDirectMainDisplay, 32, req_properties.get_x_size(), req_properties.get_y_size(), 0);
      if (newMode == NULL) {
        tinydisplay_cat.error()
          << "Invalid fullscreen size: " << req_properties.get_x_size()
          << ", " << req_properties.get_y_size()
          << "\n";
      } else {
        CGDisplaySwitchToMode(kCGDirectMainDisplay, newMode);

        // Set our new window size according to the size we actually got.
        SInt32 width, height;
        CFNumberGetValue((CFNumberRef)CFDictionaryGetValue(newMode, kCGDisplayWidth), kCFNumberSInt32Type, &width);
        CFNumberGetValue((CFNumberRef)CFDictionaryGetValue(newMode, kCGDisplayHeight), kCFNumberSInt32Type, &height);

        _properties.set_size(width, height);
      }
    }

    _properties.set_fullscreen(true);
    _properties.set_minimized(false);
    _properties.set_foreground(true);
 
    _is_fullscreen = true; 
    FullScreenWindow = this;
    req_properties.clear_fullscreen();
  } else {
    int x_origin = 10;
    int y_origin = 50;
    if (req_properties.has_origin()) { 
      y_origin  = req_properties.get_y_origin();
      x_origin = req_properties.get_x_origin();
    }

    int x_size = 512;
    int y_size = 512;
    if (req_properties.has_size()) {
      x_size = req_properties.get_x_size();
      y_size = req_properties.get_y_size();
    }
      
    // A coordinate of -2 means to center the window on screen.
    if (y_origin == -2 || x_origin == -2) {
      if (y_origin == -2) {
        y_origin = (_pipe->get_display_height() - y_size) / 2;
      }
      if (x_origin == -2) {
        x_origin = (_pipe->get_display_width() - x_size) / 2;
      }
    }

    // A coordinate of -1 means a default location.
    if (y_origin == -1) {
      y_origin = 50;
    }
    if (x_origin == -1) {
      x_origin = 10;
    }

    _properties.set_origin(x_origin, y_origin);
    _properties.set_size(x_size, y_size);

    Rect r;
    r.top = y_origin;
    r.left = x_origin;
    r.right = r.left + x_size;
    r.bottom = r.top + y_size;

            /*
    if (req_properties.has_parent_window()) {
        tinydisplay_cat.info() << "Creating child window\n";

        CreateNewWindow(kSimpleWindowClass, kWindowNoAttributes, &r, &_osx_window);
        AddAWindow(_osx_window);

        _properties.set_fixed_size(true);
        tinydisplay_cat.info() << "Child window created\n";
    }
    else */
    {
      int attributes = kWindowStandardDocumentAttributes | kWindowStandardHandlerAttribute;
      if (req_properties.has_fixed_size() && req_properties.get_fixed_size()) {
        attributes &= ~kWindowResizableAttribute;
      }

      if (req_properties.has_undecorated() && req_properties.get_undecorated()) { // create a unmovable .. no edge window..
	tinydisplay_cat.info() << "Creating undecorated window\n";

        // We don't want a resize box either.
        attributes &= ~kWindowResizableAttribute;
        attributes |= kWindowNoTitleBarAttribute;
	CreateNewWindow(kDocumentWindowClass, attributes, &r, &_osx_window);
      }
      else
      { // create a window with crome and sizing and sucj
	// In this case, we want to constrain the window to the
	// available size.
	Rect bounds;
	GetAvailableWindowPositioningBounds(GetMainDevice(), &bounds);

	r.left = max(r.left, bounds.left);
	r.right = min(r.right, bounds.right);
	r.top = max(r.top, bounds.top);
	r.bottom = min(r.bottom, bounds.bottom);

	tinydisplay_cat.info() << "Creating standard window\n";
	CreateNewWindow(kDocumentWindowClass, attributes, &r, &_osx_window);
	AddAWindow(_osx_window);
      }
    }


    if (_osx_window) {

      EventHandlerUPP gWinEvtHandler;            // window event handler
      EventTypeSpec list[] =
      {
          { kEventClassWindow, kEventWindowCollapsing },
          { kEventClassWindow, kEventWindowShown },
          { kEventClassWindow, kEventWindowActivated },
          { kEventClassWindow, kEventWindowDeactivated },
          { kEventClassWindow, kEventWindowClose },
          { kEventClassWindow, kEventWindowBoundsChanging },
          { kEventClassWindow, kEventWindowBoundsChanged },

          { kEventClassWindow, kEventWindowCollapsed },
          { kEventClassWindow, kEventWindowExpanded },
          { kEventClassWindow, kEventWindowZoomed },
          { kEventClassWindow, kEventWindowClosed },
      };

      SetWRefCon (_osx_window, (long) this); // point to the window record in the ref con of the window
      gWinEvtHandler = NewEventHandlerUPP(windowEvtHndlr);
      InstallWindowEventHandler(_osx_window, gWinEvtHandler, GetEventTypeCount(list), list, (void*)this, NULL); // add event handler

                  /*if (!req_properties.has_parent_window()) */
      {
          ShowWindow (_osx_window);
      }
                  /*
      else
      {

          NSWindow*    parentWindow        =    (NSWindow *)req_properties.get_parent_window();
      //    NSView*        aView                =    [[parentWindow contentView] viewWithTag:378];
      //    NSRect        aRect                =    [aView frame];
      //    NSPoint        origin                =    [parentWindow convertBaseToScreen:aRect.origin];

      //    NSWindow*    childWindow            =    [[NSWindow alloc] initWithWindowRef:_osx_window];


          Post_Event_Wait(PANDA_CREATE_WINDOW,(unsigned long) _osx_window,1,[parentWindow windowNumber]);

      //    [childWindow setFrameOrigin:origin];
      //    [childWindow setAcceptsMouseMovedEvents:YES];
      //    [childWindow setBackgroundColor:[NSColor blackColor]];
           // this seems to block till the parent accepts the connection ?
//                [parentWindow addChildWindow:childWindow ordered:NSWindowAbove];
//                [childWindow orderFront:nil];


          _properties.set_parent_window(req_properties.get_parent_window());
          req_properties.clear_parent_window();

                          }    */

      if (req_properties.has_fullscreen()) {
        _properties.set_fullscreen(false);
        req_properties.clear_fullscreen();
      }

      if (req_properties.has_undecorated()) {
        _properties.set_undecorated(req_properties.get_undecorated());
        req_properties.clear_undecorated();
      }

      _properties.set_minimized(false);
      _properties.set_foreground(true);

      if (req_properties.has_minimized()) {
        CollapseWindow(_osx_window, req_properties.get_minimized());
        _properties.set_minimized(req_properties.get_minimized());
        _properties.set_foreground(!req_properties.get_minimized());
        req_properties.clear_minimized();
      }
    }

    // Now measure the size and placement of the window we
    // actually ended up with.
    Rect rectPort = {0,0,0,0};
    GetWindowPortBounds (_osx_window, &rectPort);
    _properties.set_origin((int) rectPort.left, (int) rectPort.top);
    _properties.set_size((int) (rectPort.right - rectPort.left), (int) (rectPort.bottom - rectPort.top));
    req_properties.clear_size();
    req_properties.clear_origin();
  }

  if (req_properties.has_icon_filename()) {
    set_icon_filename(req_properties.get_icon_filename());
  }

  _properties.set_open(true);

  if (_properties.has_size())
      set_size_and_recalc(_properties.get_x_size(), _properties.get_y_size());


  return (err == noErr);
}

////////////////////////////////////////////////////////////////////
//     Function: TinyOsxGraphicsWindow::process_events()
//       Access: virtual, protected
//  Description: Required Event upcall . Used to dispatch Window and Aplication Events
//               back into panda
//
////////////////////////////////////////////////////////////////////
void TinyOsxGraphicsWindow::process_events() {
  GraphicsWindow::process_events();

  if (!osx_disable_event_loop) {
    EventRef          theEvent;
    EventTargetRef theTarget    =    GetEventDispatcherTarget();

    /*if (!_properties.has_parent_window())*/
    {
      while  (ReceiveNextEvent(0, NULL, kEventDurationNoWait, true, &theEvent)== noErr) {
	SendEventToEventTarget (theEvent, theTarget);
	ReleaseEvent(theEvent);
      }
    }
  }
};

////////////////////////////////////////////////////////////////////
//     Function: TinyOsxGraphicsWindow::supports_pixel_zoom
//       Access: Published, Virtual
//  Description: Returns true if a call to set_pixel_zoom() will be
//               respected, false if it will be ignored.  If this
//               returns false, then get_pixel_factor() will always
//               return 1.0, regardless of what value you specify for
//               set_pixel_zoom().
//
//               This may return false if the underlying renderer
//               doesn't support pixel zooming, or if you have called
//               this on a DisplayRegion that doesn't have both
//               set_clear_color() and set_clear_depth() enabled.
////////////////////////////////////////////////////////////////////
bool TinyOsxGraphicsWindow::
supports_pixel_zoom() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: TinyOsxGraphicsWindow::handleKeyInput()
//       Access: virtual, protected
//  Description: Required Event upcall . Used to dispatch Window and Aplication Events
//               back into panda
//
////////////////////////////////////////////////////////////////////
// key input handler
OSStatus TinyOsxGraphicsWindow::handleKeyInput (EventHandlerCallRef myHandler, EventRef event, Boolean keyDown) {

 if (tinydisplay_cat.is_debug()) {
    UInt32 keyCode;
    GetEventParameter (event, kEventParamKeyCode, typeUInt32, NULL, sizeof(UInt32), NULL, &keyCode);

    tinydisplay_cat.debug()
      << ClockObject::get_global_clock()->get_real_time()
      << " handleKeyInput: " << (void *)this << ", " << keyCode
      << ", " << (int)keyDown << "\n";
  }

  //CallNextEventHandler(myHandler, event);

  // We don't check the result of the above function.  In principle,
  // this should return eventNotHandledErr if the key event is not
  // handled by the OS, but in practice, testing this just seems to
  // eat the Escape keypress meaninglessly.  Keypresses like F11 that
  // are already mapped in the desktop seem to not even come into this
  // function in the first place.
  UInt32 newModifiers = 0;
  OSStatus error = GetEventParameter(event, kEventParamKeyModifiers, typeUInt32, NULL, sizeof(UInt32), NULL, &newModifiers);
  if (error == noErr) {
    HandleModifireDeleta(newModifiers);
  }

  UInt32 keyCode;
  GetEventParameter(event, kEventParamKeyCode, typeUInt32, NULL, sizeof(UInt32), NULL, &keyCode);
  ButtonHandle button = OSX_TranslateKey(keyCode, event);

  if (keyDown) {
    if ((newModifiers & cmdKey) != 0) {
      if (button == KeyboardButton::ascii_key('q') || button == KeyboardButton::ascii_key('w')) {
        // Command-Q or Command-W: quit the application or close the
        // window, respectively.  For now, we treat them both the
        // same: close the window.
        user_close_request();
      }
    }
    SendKeyEvent(button, true);
  } else {
    SendKeyEvent(button, false);
  }
  return   CallNextEventHandler(myHandler, event);
//  return noErr;
}

 ////////////////////////////////////////////////////////////////////
 //     Function:
 //       Access:
 //  Description:
 ////////////////////////////////////////////////////////////////////
void TinyOsxGraphicsWindow::SystemSetWindowForground(bool forground) {
  WindowProperties properties;
  properties.set_foreground(forground);
  system_changed_properties(properties);
}

////////////////////////////////////////////////////////////////////
//     Function:
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void TinyOsxGraphicsWindow::SystemPointToLocalPoint(Point &qdGlobalPoint) {
  if (_osx_window != NULL) {
    GrafPtr savePort;
    Boolean    portChanged    =    QDSwapPort(GetWindowPort(_osx_window), &savePort);

    GlobalToLocal(&qdGlobalPoint);

    if (portChanged)
      QDSwapPort(savePort, NULL);
  }
}

 ////////////////////////////////////////////////////////////////////
 //     Function:
 //       Access:
 //  Description:
 ////////////////////////////////////////////////////////////////////
 OSStatus TinyOsxGraphicsWindow::handleWindowMouseEvents (EventHandlerCallRef myHandler, EventRef event) {
  WindowRef            window = NULL;
  OSStatus            result = eventNotHandledErr;
  UInt32                 kind = GetEventKind (event);
 Point qdGlobalPoint = {0, 0};
  UInt32                modifiers = 0;
  Rect                 rectPort;
  SInt32 this_wheel_delta;
  EventMouseWheelAxis wheelAxis;

  //cerr <<" Start Mouse Event " << _ID << "\n";

  // Mac OS X v10.1 and later
  // should this be front window???
  GetEventParameter(event, kEventParamWindowRef, typeWindowRef, NULL, sizeof(WindowRef), NULL, &window);

  if (!_is_fullscreen && (window == NULL || window != _osx_window )) {
    if (kind == kEventMouseMoved) {
      set_pointer_out_of_window();
    }
    return eventNotHandledErr;
  }



  GetWindowPortBounds (window, &rectPort);

  // result = CallNextEventHandler(myHandler, event);
  // if (eventNotHandledErr == result) { // only handle events not already handled (prevents wierd resize interaction)
  switch (kind) {
     // Whenever mouse button state changes, generate the
     // appropriate Panda down/up events to represent the
     // change.

    case kEventMouseDown:
    case kEventMouseUp:
      {
	GetEventParameter(event, kEventParamKeyModifiers, typeUInt32, NULL, sizeof(UInt32), NULL, &modifiers);
	if (_properties.get_mouse_mode() == WindowProperties::M_relative) {
	  GetEventParameter(event, kEventParamMouseDelta,typeQDPoint, NULL, sizeof(Point),NULL    , (void*) &qdGlobalPoint);
	  MouseData currMouse = get_pointer(0);
	  qdGlobalPoint.h += currMouse.get_x();
	  qdGlobalPoint.v += currMouse.get_y();
	}
	else
	{
	  GetEventParameter(event, kEventParamMouseLocation,typeQDPoint, NULL, sizeof(Point),NULL    , (void*) &qdGlobalPoint);
	  SystemPointToLocalPoint(qdGlobalPoint);
	}

	set_pointer_in_window((int)qdGlobalPoint.h, (int)qdGlobalPoint.v);

	UInt32 new_buttons = GetCurrentEventButtonState();
	HandleButtonDelta(new_buttons);
      }
      result = noErr;
      break;

    case kEventMouseMoved:
    case kEventMouseDragged:
      if (_properties.get_mouse_mode()==WindowProperties::M_relative) {
	GetEventParameter(event, kEventParamMouseDelta,typeQDPoint, NULL, sizeof(Point),NULL    , (void*) &qdGlobalPoint);

	MouseData currMouse=get_pointer(0);
	qdGlobalPoint.h+=currMouse.get_x();
	qdGlobalPoint.v+=currMouse.get_y();
      } else {
	GetEventParameter(event, kEventParamMouseLocation,typeQDPoint, NULL, sizeof(Point),NULL    , (void*) &qdGlobalPoint);
	SystemPointToLocalPoint(qdGlobalPoint);
      }
      if (kind == kEventMouseMoved &&
	  (qdGlobalPoint.h < 0 || qdGlobalPoint.v < 0)) {
	// Moving into the titlebar region.
	set_pointer_out_of_window();
      } else {
	// Moving within the window itself (or dragging anywhere).
	set_pointer_in_window((int)qdGlobalPoint.h, (int)qdGlobalPoint.v);
      }
      result = noErr;

      break;

    case kEventMouseWheelMoved:
      GetEventParameter(event, kEventParamMouseWheelDelta, typeLongInteger, NULL, sizeof(this_wheel_delta), NULL, &this_wheel_delta);
      GetEventParameter(event, kEventParamMouseWheelAxis, typeMouseWheelAxis, NULL, sizeof(wheelAxis), NULL, &wheelAxis );
      GetEventParameter(event, kEventParamMouseLocation,typeQDPoint, NULL, sizeof(Point),NULL    , (void*) &qdGlobalPoint);
      SystemPointToLocalPoint(qdGlobalPoint);

      if (wheelAxis == kEventMouseWheelAxisY) {
	set_pointer_in_window((int)qdGlobalPoint.h, (int)qdGlobalPoint.v);
	_wheel_delta += this_wheel_delta;
	SInt32 wheel_scale = osx_mouse_wheel_scale;
	while (_wheel_delta > wheel_scale) {
	  _input_devices[0].button_down(MouseButton::wheel_up());
	  _input_devices[0].button_up(MouseButton::wheel_up());
	  _wheel_delta -= wheel_scale;
	}
	while (_wheel_delta < -wheel_scale) {
	  _input_devices[0].button_down(MouseButton::wheel_down());
	  _input_devices[0].button_up(MouseButton::wheel_down());
	  _wheel_delta += wheel_scale;
	}
      }
      result = noErr;
      break;
    }
    //result = noErr;

   return result;
 }

 ////////////////////////////////////////////////////////////////////
 //     Function: TinyOsxGraphicsWindow::OSX_TranslateKey
 //       Access: Private
 //  Description: MAC Key Codes to Panda Key Codes
 ////////////////////////////////////////////////////////////////////
 ButtonHandle TinyOsxGraphicsWindow::OSX_TranslateKey(UInt32 key, EventRef event) {

   ButtonHandle nk = ButtonHandle::none();
   switch ( key ) {
   case 0:    nk = KeyboardButton::ascii_key('a');       break;
   case 11:   nk = KeyboardButton::ascii_key('b');       break;
   case 8:    nk = KeyboardButton::ascii_key('c');       break;
   case 2:    nk = KeyboardButton::ascii_key('d');       break;
   case 14:   nk = KeyboardButton::ascii_key('e');       break;
   case 3:    nk = KeyboardButton::ascii_key('f');       break;
   case 5:    nk = KeyboardButton::ascii_key('g');       break;
   case 4:    nk = KeyboardButton::ascii_key('h');       break;
   case 34:   nk = KeyboardButton::ascii_key('i');       break;
   case 38:   nk = KeyboardButton::ascii_key('j');       break;
   case 40:   nk = KeyboardButton::ascii_key('k');       break;
   case 37:   nk = KeyboardButton::ascii_key('l');       break;
   case 46:   nk = KeyboardButton::ascii_key('m');       break;
   case 45:   nk = KeyboardButton::ascii_key('n');       break;
   case 31:   nk = KeyboardButton::ascii_key('o');       break;
   case 35:   nk = KeyboardButton::ascii_key('p');       break;
   case 12:   nk = KeyboardButton::ascii_key('q');       break;
   case 15:   nk = KeyboardButton::ascii_key('r');       break;
   case 1:    nk = KeyboardButton::ascii_key('s');       break;
   case 17:   nk = KeyboardButton::ascii_key('t');       break;
   case 32:   nk = KeyboardButton::ascii_key('u');       break;
   case 9:    nk = KeyboardButton::ascii_key('v');       break;
   case 13:   nk = KeyboardButton::ascii_key('w');       break;
   case 7:    nk = KeyboardButton::ascii_key('x');       break;
   case 16:   nk = KeyboardButton::ascii_key('y');       break;
   case 6:    nk = KeyboardButton::ascii_key('z');       break;

       // top row numbers
   case 29:   nk = KeyboardButton::ascii_key('0');       break;
   case 18:   nk = KeyboardButton::ascii_key('1');       break;
   case 19:   nk = KeyboardButton::ascii_key('2');       break;
   case 20:   nk = KeyboardButton::ascii_key('3');       break;
   case 21:   nk = KeyboardButton::ascii_key('4');       break;
   case 23:   nk = KeyboardButton::ascii_key('5');       break;
   case 22:   nk = KeyboardButton::ascii_key('6');       break;
   case 26:   nk = KeyboardButton::ascii_key('7');       break;
   case 28:   nk = KeyboardButton::ascii_key('8');       break;
   case 25:   nk = KeyboardButton::ascii_key('9');       break;

       // key pad ... do they really map to the top number in panda ?
   case 82:   nk = KeyboardButton::ascii_key('0');       break;
   case 83:   nk = KeyboardButton::ascii_key('1');       break;
   case 84:   nk = KeyboardButton::ascii_key('2');       break;
   case 85:   nk = KeyboardButton::ascii_key('3');       break;
   case 86:   nk = KeyboardButton::ascii_key('4');       break;
   case 87:   nk = KeyboardButton::ascii_key('5');       break;
   case 88:   nk = KeyboardButton::ascii_key('6');       break;
   case 89:   nk = KeyboardButton::ascii_key('7');       break;
   case 91:   nk = KeyboardButton::ascii_key('8');       break;
   case 92:   nk = KeyboardButton::ascii_key('9');       break;


       //    case 36:   nk = KeyboardButton::ret();              break;   // no return  in panda ???
   case 49:   nk = KeyboardButton::space();               break;
   case 51:   nk = KeyboardButton::backspace();          break;
   case 48:   nk = KeyboardButton::tab();                  break;
   case 53:   nk = KeyboardButton::escape();              break;
   case 76:   nk = KeyboardButton::enter();              break;
   case 36:   nk = KeyboardButton::enter();              break;

   case 123:  nk = KeyboardButton::left();              break;
   case 124:  nk = KeyboardButton::right();              break;
   case 125:  nk = KeyboardButton::down();              break;
   case 126:  nk = KeyboardButton::up();                  break;
   case 116:  nk = KeyboardButton::page_up();              break;
   case 121:  nk = KeyboardButton::page_down();          break;
   case 115:  nk = KeyboardButton::home();              break;
   case 119:  nk = KeyboardButton::end();                  break;
   case 114:  nk = KeyboardButton::help();              break;
   case 117:  nk = KeyboardButton::del();                  break;

       //    case  71:  nk = KeyboardButton::num_lock()        break;

   case 122:  nk = KeyboardButton::f1();                  break;
   case 120:  nk = KeyboardButton::f2();                  break;
   case  99:  nk = KeyboardButton::f3();                  break;
   case 118:  nk = KeyboardButton::f4();                  break;
   case  96:  nk = KeyboardButton::f5();                  break;
   case  97:  nk = KeyboardButton::f6();                  break;
   case  98:  nk = KeyboardButton::f7();                  break;
   case 100:  nk = KeyboardButton::f8();                  break;
   case 101:  nk = KeyboardButton::f9();                  break;
   case 109:  nk = KeyboardButton::f10();                  break;
   case 103:  nk = KeyboardButton::f11();                  break;
   case 111:  nk = KeyboardButton::f12();                  break;

   case 105:  nk = KeyboardButton::f13();                  break;
   case 107:  nk = KeyboardButton::f14();                  break;
   case 113:  nk = KeyboardButton::f15();                  break;
   case 106:  nk = KeyboardButton::f16();                  break;

       // shiftable chartablet
   case  50:  nk = KeyboardButton::ascii_key('`');      break;
   case  27:  nk = KeyboardButton::ascii_key('-');      break;
   case  24:  nk = KeyboardButton::ascii_key('=');      break;
   case  33:  nk = KeyboardButton::ascii_key('[');      break;
   case  30:  nk = KeyboardButton::ascii_key(']');      break;
   case  42:  nk = KeyboardButton::ascii_key('\\');      break;
   case  41:  nk = KeyboardButton::ascii_key(';');      break;
   case  39:  nk = KeyboardButton::ascii_key('\'');      break;
   case  43:  nk = KeyboardButton::ascii_key(',');      break;
   case  47:  nk = KeyboardButton::ascii_key('.');      break;
   case  44:  nk = KeyboardButton::ascii_key('/');      break;

   default:
     if (tinydisplay_cat.is_debug()) {
       tinydisplay_cat.debug()
	 << " Untranslated KeyCode: " << key
	 << " (0x" << hex << key << dec << ")\n";
     }

     // not sure this is right .. but no mapping for keypad and such
     // this at least does a best gess..

     char charCode =  0;
     if (GetEventParameter( event, kEventParamKeyMacCharCodes, typeChar, nil, sizeof( charCode ), nil, &charCode ) == noErr)
       nk = KeyboardButton::ascii_key(charCode);
   }
   return nk;
 }
 ////////////////////////////////////////////////////////////////////
 //     Function: TinyOsxGraphicsWindow::HandleModifireDeleta
 //       Access: Private
 //  Description: Used to emulate key events for the MAC key Modifiers..
 ////////////////////////////////////////////////////////////////////
 void TinyOsxGraphicsWindow::HandleModifireDeleta(UInt32 newModifiers) {
  UInt32 changed = _last_key_modifiers ^ newModifiers;

  if ((changed & (shiftKey | rightShiftKey)) != 0)
    SendKeyEvent(KeyboardButton::shift(),(newModifiers & (shiftKey | rightShiftKey)) != 0) ;

  if ((changed & (optionKey | rightOptionKey)) != 0)
    SendKeyEvent(KeyboardButton::alt(),(newModifiers & (optionKey | rightOptionKey)) != 0);


  if ((changed & (controlKey | rightControlKey)) != 0)
    SendKeyEvent(KeyboardButton::control(),(newModifiers & (controlKey | rightControlKey)) != 0);

  if ((changed & cmdKey) != 0)
    SendKeyEvent(KeyboardButton::meta(),(newModifiers & cmdKey) != 0);

  if ((changed & alphaLock) != 0)
    SendKeyEvent(KeyboardButton::caps_lock(),(newModifiers & alphaLock) != 0);

  // save current state
  _last_key_modifiers = newModifiers;
 };

////////////////////////////////////////////////////////////////////
//     Function: TinyOsxGraphicsWindow::HandleButtonDelta
//       Access: Private
//  Description: Used to emulate buttons events/
////////////////////////////////////////////////////////////////////
void TinyOsxGraphicsWindow::
HandleButtonDelta(UInt32 new_buttons) {
  UInt32 changed = _last_buttons ^ new_buttons;

  if (changed & 0x01) {
    if (new_buttons & 0x01) {
      _input_devices[0].button_down(MouseButton::one());
    } else {
      _input_devices[0].button_up(MouseButton::one());
    }
  }

  if (changed & 0x04) {
    if (new_buttons & 0x04) {
      _input_devices[0].button_down(MouseButton::two());
    } else {
      _input_devices[0].button_up(MouseButton::two());
    }
  }

  if (changed & 0x02) {
    if (new_buttons & 0x02) {
      _input_devices[0].button_down(MouseButton::three());
    } else {
      _input_devices[0].button_up(MouseButton::three());
    }
  }

  _last_buttons = new_buttons;
}

 ////////////////////////////////////////////////////////////////////
//     Function: TinyOsxGraphicsWindow::move_pointer
//       Access: Published, Virtual
//  Description: Forces the pointer to the indicated position within
//               the window, if possible.
//
//               Returns true if successful, false on failure.  This
//               may fail if the mouse is not currently within the
//               window, or if the API doesn't support this operation.
////////////////////////////////////////////////////////////////////
bool TinyOsxGraphicsWindow::move_pointer(int device, int x, int y) {
  if (_osx_window == NULL) {
    return false;
  }

  if (tinydisplay_cat.is_debug()) {
    tinydisplay_cat.debug() << "move_pointer " << device <<" "<<  x <<" "<< y <<"\n";
  }

  Point pt = {0, 0};
  pt.h = x;
  pt.v = y;
  set_pointer_in_window(x, y);

  if (_properties.get_mouse_mode() == WindowProperties::M_absolute) {
     LocalPointToSystemPoint(pt);
     CGPoint newCursorPosition = {0, 0};
     newCursorPosition.x = pt.h;
     newCursorPosition.y = pt.v;
     mouse_mode_relative();
     CGWarpMouseCursorPosition(newCursorPosition);
     mouse_mode_absolute();
  }

  return true;
};

bool TinyOsxGraphicsWindow::do_reshape_request(int x_origin, int y_origin, bool has_origin, int x_size, int y_size) {
  tinydisplay_cat.info() << "Do Reshape\n";

  if (_properties.get_fullscreen()) {
    return false;
  }

  // A coordinate of -2 means to center the window on screen.
  if (x_origin == -2 || y_origin == -2 || x_origin == -1 || y_origin == -1) {
    if (y_origin == -2) {
      y_origin = (_pipe->get_display_height() - y_size) / 2;
    }
    if (x_origin == -2) {
      x_origin = (_pipe->get_display_width() - x_size) / 2;
    }
    if (y_origin == -1) {
      y_origin = 50;
    }
    if (x_origin == -1) {
      x_origin = 10;
    }
    _properties.set_origin(x_origin, y_origin);
    system_changed_properties(_properties);
  }

  /*
  if (_properties.has_parent_window()) {
    if (has_origin) {
      NSWindow*    parentWindow        =    (NSWindow *)_properties.get_parent_window();
      NSRect        parentFrame            =    [parentWindow frame];

      MoveWindow(_osx_window, x_origin+parentFrame.origin.x, y_origin+parentFrame.origin.y, false);
     }
  }
  else*/
  {
    // We sometimes get a bogus origin of (0, 0).  As a special hack,
    // treat this as a special case, and ignore it.
    if (has_origin) {
      if (x_origin != 0 || y_origin != 0) {
        MoveWindow(_osx_window, x_origin, y_origin, false);
      }
    }
  }

  if (!_properties.get_undecorated()) {
    // Constrain the window to the available desktop size.
    Rect bounds;
    GetAvailableWindowPositioningBounds(GetMainDevice(), &bounds);

    x_size = min(x_size, bounds.right - bounds.left);
    y_size = min(y_size, bounds.bottom - bounds.top);
  }

  SizeWindow(_osx_window, x_size, y_size, false);

  system_changed_size(x_size, y_size);
  ZB_resize(_frame_buffer, NULL, _properties.get_x_size(), _properties.get_y_size());
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: TinyOsxGraphicsWindow::set_properties_now
//       Access: Public, Virtual
//  Description: Applies the requested set of properties to the
//               window, if possible, for instance to request a change
//               in size or minimization status.
//
//               The window properties are applied immediately, rather
//               than waiting until the next frame.  This implies that
//               this method may *only* be called from within the
//               window thread.
//
//               The properties that have been applied are cleared
//               from the structure by this function; so on return,
//               whatever remains in the properties structure are
//               those that were unchanged for some reason (probably
//               because the underlying interface does not support
//               changing that property on an open window).
////////////////////////////////////////////////////////////////////
void TinyOsxGraphicsWindow::set_properties_now(WindowProperties &properties) {
  if (tinydisplay_cat.is_debug()) {
    tinydisplay_cat.debug()
      << "------------------------------------------------------\n";
    tinydisplay_cat.debug()
      << "set_properties_now " << properties << "\n";
  }

  GraphicsWindow::set_properties_now(properties);

  if (tinydisplay_cat.is_debug()) {
    tinydisplay_cat.debug()
      << "set_properties_now After Base Class" << properties << "\n";
  }

  // for some changes .. a full rebuild is required for the OS layer Window.
  //    I think it is the crome atribute and full screen behaviour.
  bool need_full_rebuild = false;

  // if we are not full and transitioning to full
  if (properties.has_fullscreen() &&
      properties.get_fullscreen() != _properties.get_fullscreen()) {
    need_full_rebuild = true;
  }

  // If we are fullscreen and requesting a size change
  if (_properties.get_fullscreen() &&
      (properties.has_size() &&
       (properties.get_x_size() != _properties.get_x_size() ||
        properties.get_y_size() != _properties.get_y_size()))) {
    need_full_rebuild = true;
  }
 
  // If we are fullscreen and requesting a minimize change
  if (_properties.get_fullscreen() && 
      (properties.has_minimized() && 
       (properties.get_minimized() != _properties.get_minimized()))) {
    need_full_rebuild = true;
  }

  if (need_full_rebuild) {
    // Logic here is ..  take a union of the properties .. with the
    // new allowed to overwrite the old states.  and start a bootstrap
    // of a new window ..

    // get a copy of my properties..
    WindowProperties req_properties(_properties);
    ReleaseSystemResources();
    req_properties.add_properties(properties);

    OSOpenWindow(req_properties);

    // Now we've handled all of the requested properties.
    properties.clear();
  }

  if (properties.has_title()) {
    _properties.set_title(properties.get_title());
    if (_osx_window) {
      SetWindowTitleWithCFString(_osx_window,
                                 CFStringCreateWithCString(NULL,properties.get_title().c_str(),
                                                           kCFStringEncodingMacRoman));
    }
    properties.clear_title();
  }

  // An icon filename means to load up the icon and save it.  We can't
  // necessarily apply it immediately; it will get applied later, in
  // the window event handler.
  if (properties.has_icon_filename()) {
    if (set_icon_filename(properties.get_icon_filename())) {
      properties.clear_icon_filename();
    }
  }

  // decorated .. if this changes it reqires a new window
  if (properties.has_undecorated()) {
    _properties.set_undecorated(properties.get_undecorated());
    properties.clear_undecorated();
  }

  if (properties.has_cursor_hidden()) {
    _properties.set_cursor_hidden(properties.get_cursor_hidden());
    _cursor_hidden = properties.get_cursor_hidden();
    if (_cursor_hidden && _input_devices[0].has_pointer()) {
      if (!_display_hide_cursor) {
        CGDisplayHideCursor(kCGDirectMainDisplay);
        _display_hide_cursor = true;
      }
    } else {
      if (_display_hide_cursor) {
        CGDisplayShowCursor(kCGDirectMainDisplay);
        _display_hide_cursor = false;
      }
    }
    properties.clear_cursor_hidden();
  }

  if (properties.has_minimized()) {
    if (_properties.get_minimized() != properties.get_minimized()) {
      CollapseWindow(_osx_window, properties.get_minimized());
      _properties.set_minimized(properties.get_minimized());
      _properties.set_foreground(!properties.get_minimized());
    }
    properties.clear_minimized();
  }

  if (tinydisplay_cat.is_debug()) {
    tinydisplay_cat.debug()
      << "set_properties_now  Out....." << _properties << "\n";
  }

  return;
}

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
void TinyOsxGraphicsWindow::LocalPointToSystemPoint(Point &qdLocalPoint) {
  if (_osx_window != NULL) {
    GrafPtr savePort;
    Boolean portChanged = QDSwapPort(GetWindowPort(_osx_window), &savePort);

    LocalToGlobal( &qdLocalPoint );

    if (portChanged) {
      QDSwapPort(savePort, NULL);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TinyOsxGraphicsWindow::mouse_mode_relative
//       Access: Protected, Virtual
//  Description: Detaches mouse. Only mouse delta from now on.
////////////////////////////////////////////////////////////////////
void TinyOsxGraphicsWindow::mouse_mode_relative() {
  CGAssociateMouseAndMouseCursorPosition(false);
}

////////////////////////////////////////////////////////////////////
//     Function: TinyOsxGraphicsWindow::mouse_mode_absolute
//       Access: Protected, Virtual
//  Description: Reattaches mouse to location
////////////////////////////////////////////////////////////////////
void TinyOsxGraphicsWindow::mouse_mode_absolute() {
  CGAssociateMouseAndMouseCursorPosition(true);
}

////////////////////////////////////////////////////////////////////
//     Function: TinyOsxGraphicsWindow::create_frame_buffer
//       Access: Private
//  Description: Creates a suitable frame buffer for the current
//               window size.
////////////////////////////////////////////////////////////////////
void TinyOsxGraphicsWindow::
create_frame_buffer() {
  if (_frame_buffer != NULL) {
    ZB_close(_frame_buffer);
    _frame_buffer = NULL;
  }

  _frame_buffer = ZB_open(_properties.get_x_size(), _properties.get_y_size(), ZB_MODE_RGBA, 0, 0, 0, 0);
}

#endif  // IS_OSX
