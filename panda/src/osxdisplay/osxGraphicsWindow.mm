////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University. All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license. You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

// We include these system header files first, because there is a
// namescope conflict between them and some other header file that
// gets included later (in particular, TCP_NODELAY must not be a
// #define symbol for these headers to be included properly).

#include <Carbon/Carbon.h>
#include <Cocoa/Cocoa.h>
#include <ApplicationServices/ApplicationServices.h>
#include <OpenGL/gl.h>
#include <AGL/agl.h>

// We have to include this before we include the system OpenGL/gl.h
// file, but after we include all of the above header files.  Deal
// with this contradiction later.
#include "glgsg.h"

#include "osxGraphicsWindow.h"
#include "config_osxdisplay.h"
#include "osxGraphicsPipe.h"
#include "pStatTimer.h"
#include "keyboardButton.h"
#include "mouseButton.h"
#include "osxGraphicsStateGuardian.h"
#include "osxGraphicsPipe.h"
#include "throw_event.h"
#include "pnmImage.h"
#include "virtualFileSystem.h"
#include "config_util.h"
#include "pset.h"
#include "pmutex.h"


////////////////////////////////////

static Mutex &
osx_global_mutex() {
  static Mutex m("osx_global_mutex");
  return m;
}


////////////////////////// Global Objects .....

TypeHandle osxGraphicsWindow::_type_handle;
osxGraphicsWindow *osxGraphicsWindow::full_screen_window = NULL;

#define USER_CONTAINER

#ifdef USER_CONTAINER

pset<WindowRef> my_windows;
static void
add_a_window(WindowRef window) {
  my_windows.insert(window);
}

static bool
check_my_window(WindowRef window) {
  return my_windows.find(window) != my_windows.end();
}

#else // USER_CONTAINER

static void
add_a_window(WindowRef window) {
}

static bool
check_my_window(WindowRef window) {
  return true;
}

#endif // USER_CONTAINER



////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsWindow::get_current_osx_window
//       Access: Public, Static
//  Description: Returns the active window for the purpose of
//               recording events.
////////////////////////////////////////////////////////////////////
osxGraphicsWindow *osxGraphicsWindow::
get_current_osx_window(WindowRef window) {
  if (full_screen_window != NULL) {
    return full_screen_window;
  }
 
  if (window == NULL) { 
    // HID use this path

    // Assume first we are a child window. If we cant find a window
    // of that class, then we are standalone and can jsut grab the
    // front window.
    window = GetFrontWindowOfClass(kSimpleWindowClass, TRUE);
    if (window == NULL) {
      window = FrontNonFloatingWindow();
    }
  }
 
  if (window && check_my_window(window)) {
    return (osxGraphicsWindow *)GetWRefCon (window);
  } else {
    return NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: report_agl_error
//  Description: Convenience function to report the current AGL error
//               code as a formatted error message.
////////////////////////////////////////////////////////////////////
OSStatus 
report_agl_error(const string &comment) {
  GLenum err = aglGetError();
  if (err != AGL_NO_ERROR) {
    osxdisplay_cat.error()
      << "AGL Error " << aglErrorString(err) << " [" <<comment << "]\n";
  }

  if (err == AGL_NO_ERROR) {
    return noErr;
  } else {
    return (OSStatus)err;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: invert_gl_image
//  Description: Vertically inverts a rendered image.
////////////////////////////////////////////////////////////////////
static void
invert_gl_image(char *imageData, size_t imageSize, size_t rowBytes) {
  char *buffer = (char*)alloca(rowBytes);
  nassertv(buffer != (char *)NULL);

  // Copy by rows through temp buffer
  for (size_t i = 0, j = imageSize - rowBytes; 
       i < imageSize >> 1; 
       i += rowBytes, j -= rowBytes) {
    memcpy(buffer, &imageData[i], rowBytes);
    memcpy(&imageData[i], &imageData[j], rowBytes);
    memcpy(&imageData[j], buffer, rowBytes);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: composite_gl_buffer_into_window
//  Description: Drop a GL overlay onto a carbon window.. 
////////////////////////////////////////////////////////////////////
static void 
composite_gl_buffer_into_window(AGLContext ctx, Rect *bufferRect, 
                                GrafPtr out_port) {
  GWorldPtr world;
  QDErr err;
 
  // blit OpenGL content into window backing store
  // allocate buffer to hold pane image
  long width = (bufferRect->right - bufferRect->left);
  long height = (bufferRect->bottom - bufferRect->top);
 
  Rect src_rect = {0, 0, height, width};
  Rect ddrc_rect = {0, 0, height, width};
  long row_bytes = width * 4;
  long image_size = row_bytes * height;
  char *image = (char *)NewPtr(image_size);

  if (!image) {
    osxdisplay_cat.error()
      << "Out of memory in composite_gl_buffer_into_window()!\n";
    return; // no harm in continuing
  }
 
  // pull GL content down to our image buffer
  aglSetCurrentContext(ctx);
  glReadPixels(0, 0, width, height, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, 
               image);

  // GL buffers are upside-down relative to QD buffers, so we need to flip it
  invert_gl_image(image, image_size, row_bytes);

  // create a GWorld containing our image
  err = NewGWorldFromPtr(&world, k32ARGBPixelFormat, &src_rect, 0, 0, 0, 
                         image, row_bytes);
  if (err != noErr) {
    osxdisplay_cat.error() 
      << " error in NewGWorldFromPtr, called from composite_gl_buffer_into_window()\n";
    DisposePtr(image);
    return;
  }

  GrafPtr port_save = NULL;
  Boolean port_changed = QDSwapPort(out_port, &port_save);

  CopyBits(GetPortBitMapForCopyBits(world), 
           GetPortBitMapForCopyBits(out_port), 
           &src_rect, &ddrc_rect, srcCopy, 0);
 
  if (port_changed) {
    QDSwapPort(port_save, NULL);
  }

  DisposeGWorld(world);
  DisposePtr(image);
}

////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsWindow::event_handler
//       Access: Public
//  Description: The standard window event handler for non-fullscreen
//               windows.
////////////////////////////////////////////////////////////////////
OSStatus osxGraphicsWindow::
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

  if (osxdisplay_cat.is_spam()) {
    osxdisplay_cat.spam()
      << ClockObject::get_global_clock()->get_real_time() 
      << " event_handler: " << (void *)this << ", " << window
      << ", " << the_class << ", " << kind << "\n";
  }

  switch (the_class) {
  case kEventClassMouse:
    result = handle_window_mouse_events (myHandler, event);
    break;
 
  case kEventClassWindow: 
    switch (kind) {
    case kEventWindowCollapsing:
      /*
        Rect r;
        GetWindowPortBounds (window, &r); 
        composite_gl_buffer_into_window(get_context(), &r, GetWindowPort (window));
        UpdateCollapsedWindowDockTile (window);
      */
      system_set_window_foreground(false);
      break;

    case kEventWindowActivated: // called on click activation and initially
      system_set_window_foreground(true);
      do_resize();
      break;

    case kEventWindowDeactivated:
      system_set_window_foreground(false);
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
                            typeQDRectangle, sizeof(bounds), &bounds);        
          result = noErr;
        }
      }
      break;

    case kEventWindowBoundsChanged: // called for resize and moves (drag)
      do_resize();
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
//     Function: osxGraphicsWindow::user_close_request
//       Access: Private
//  Description: The user has requested to close the window, for
//               instance with Cmd-W, or by clicking on the close
//               button.
////////////////////////////////////////////////////////////////////
void osxGraphicsWindow::
user_close_request() {
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
//     Function: osxGraphicsWindow::system_close_window
//       Access: Private
//  Description: The window has been closed by an OS resource, not by
//               an internal request
////////////////////////////////////////////////////////////////////
void osxGraphicsWindow::
system_close_window() { 
  if (osxdisplay_cat.is_debug()) {
    osxdisplay_cat.debug()
      << "System Closing Window \n";
  }
  release_system_resources(false); 
}

////////////////////////////////////////////////////////////////////
//     Function: window_event_handler
//  Description: The C callback for Window Events
//
//               We only hook this up for non-fullscreen windows, so
//               we only handle system window events.
////////////////////////////////////////////////////////////////////
static pascal OSStatus
window_event_handler(EventHandlerCallRef my_handler, EventRef event, void *) {
  // volatile().lock();
 
  WindowRef window = NULL; 
  GetEventParameter(event, kEventParamDirectObject, typeWindowRef, NULL, 
                    sizeof(WindowRef), NULL, &window);

  if (window != NULL) {
    osxGraphicsWindow *osx_win = osxGraphicsWindow::get_current_osx_window(window);
    if (osx_win != (osxGraphicsWindow *)NULL) {
      //osx_global_mutex().release();
      return osx_win->event_handler(my_handler, event);
    }
  }
 
  //osx_global_mutex().release();
  return eventNotHandledErr;
}

///////////////////////////////////////////////////////////////////
//     Function: osxGraphicsWindow::do_resize
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void osxGraphicsWindow::
do_resize() {
  if (osxdisplay_cat.is_debug()) {
    osxdisplay_cat.debug()
      << "In Resize....." << _properties << "\n";
  }

  // only in window mode .. not full screen
  if (_osx_window != NULL && !_is_fullscreen && _properties.has_size()) {
    Rect rectPort = { 
      0, 0, 0, 0 
    };
    CGRect viewRect = { 
      { 0.0f, 0.0f }, { 0.0f, 0.0f } 
    };

    GetWindowPortBounds(_osx_window, &rectPort); 
    viewRect.size.width = (PN_stdfloat)(rectPort.right - rectPort.left);
    viewRect.size.height = (PN_stdfloat)(rectPort.bottom - rectPort.top);

    // tell panda
    WindowProperties properties;
    properties.set_size((int)viewRect.size.width,(int)viewRect.size.height);
    properties.set_origin((int) rectPort.left,(int)rectPort.top);
    system_changed_properties(properties);
 
    if (osxdisplay_cat.is_debug()) {
      osxdisplay_cat.debug()
        << " Resizing Window " << viewRect.size.width 
        << " " << viewRect.size.height << "\n";
    }

    AGLContext context = get_gsg_context();
    if (context != (AGLContext)NULL) {
      // ping gl
      if (!aglSetCurrentContext(context)) {
        report_agl_error("aglSetCurrentContext");
      }
      aglUpdateContext(context);
      report_agl_error("aglUpdateContext .. This is a Resize..");
    }

    if (osxdisplay_cat.is_debug()) {
      osxdisplay_cat.debug() 
        << "Resize Complete.....\n";
    }
  } 
}

///////////////////////////////////////////////////////////////////
//     Function: app_event_handler
//  Description: The C callback for Application events.
//
//               Hooked once per application.
////////////////////////////////////////////////////////////////////
static pascal OSStatus
app_event_handler(EventHandlerCallRef my_handler, EventRef event, 
                  void *user_data) {
  OSStatus result = eventNotHandledErr;
  {
    //osx_global_mutex().lock();

    osxGraphicsWindow *osx_win = NULL;
    WindowRef window = NULL; 
    UInt32 the_class = GetEventClass (event);
    UInt32 kind = GetEventKind (event);

    GetEventParameter(event, kEventParamWindowRef, typeWindowRef, NULL, 
                      sizeof(WindowRef), NULL, (void*) &window);
    osx_win = osxGraphicsWindow::get_current_osx_window(window);
    if (osx_win == NULL) {
      //osx_global_mutex().release();
      return eventNotHandledErr;
    }

    switch (the_class) {
    case kEventClassTextInput:
      if (kind == kEventTextInputUnicodeForKeyEvent) {
        osx_win->handle_text_input(my_handler, event);
      }
      //result = noErr; 
      //
      // can not report handled .. the os will not sent the raw key strokes then 
      // if(osx_win->handle_text_input(my_handler, event) == noErr)
      // result = noErr;
      break;

    case kEventClassKeyboard:
      {
        switch (kind) {
        case kEventRawKeyRepeat:
        case kEventRawKeyDown:
          result = osx_win->handle_key_input (my_handler, event, true);
          break;
        case kEventRawKeyUp:
          result = osx_win->handle_key_input (my_handler, event, false);
          break;
        case kEventRawKeyModifiersChanged:
          {
            UInt32 newModifiers;
            OSStatus error = GetEventParameter(event, kEventParamKeyModifiers,typeUInt32, NULL,sizeof(UInt32), NULL, &newModifiers);
            if (error == noErr) { 
              osx_win->handle_modifier_delta(newModifiers);
              result = noErr; 
            }
          }
          break;
        }
      }
      break;

    case kEventClassMouse:
      // osxdisplay_cat.info() << "Mouse movement handled by Application handler\n";
      //if(osxGraphicsWindow::full_screen_window != NULL) 
      result = osx_win->handle_window_mouse_events(my_handler, event);
      //result = noErr; 
      break;
    }
    
    //osx_global_mutex().release(); 
  }
 
  return result;
}

///////////////////////////////////////////////////////////////////
//     Function: osxGraphicsWindow::handle_text_input
//       Access: Public
//  Description: Trap Unicode Input.
////////////////////////////////////////////////////////////////////
OSStatus osxGraphicsWindow::
handle_text_input(EventHandlerCallRef my_handler, EventRef text_event) {
  UniChar *text = NULL;
  UInt32 actual_size = 0; 
 
  OSStatus ret = GetEventParameter(text_event, kEventParamTextInputSendText, 
                                   typeUnicodeText, NULL, 0, &actual_size, NULL);
  if (ret != noErr) {
    return ret;
  }
 
  text = (UniChar*)NewPtr(actual_size);
  if (text!= NULL) {
    ret = GetEventParameter (text_event, kEventParamTextInputSendText,typeUnicodeText, NULL, actual_size, NULL, text);
    if (ret != noErr) {
      return ret;
    }

    for (unsigned int x = 0; x < actual_size/sizeof(UniChar); ++x) {
      _input_devices[0].keystroke(text[x]); 
    }
    DisposePtr((char *)text);
  }
 
  return ret;
}

///////////////////////////////////////////////////////////////////
//     Function: osxGraphicsWindow::release_system_resources
//       Access: Private
//  Description: Clean up the OS level messes.
////////////////////////////////////////////////////////////////////
void osxGraphicsWindow::
release_system_resources(bool destructing) {
  if (_is_fullscreen) {
    _is_fullscreen = false;
    full_screen_window = NULL;
    
    if (_originalMode != NULL) {
      CGDisplaySwitchToMode(kCGDirectMainDisplay, _originalMode);
    }

    CGDisplayRelease(kCGDirectMainDisplay);
    aglSetDrawable(get_gsg_context(), NULL);
    
    _originalMode = NULL;
  }
 
  // if the gsg context is assigned to this window
  // clear it..
  if (_osx_window != NULL && GetWindowPort (_osx_window) == (GrafPtr)aglGetDrawable(get_gsg_context())) {
    aglSetDrawable(get_gsg_context(),NULL);
  }
 
  // if we are the active gl context clear it.. 
  if (aglGetCurrentContext() == get_gsg_context()) {
    aglSetCurrentContext(NULL);
  }

  if (_osx_window != NULL) {
    SetWRefCon(_osx_window, (long int) NULL);
    HideWindow(_osx_window);
    DisposeWindow(_osx_window);
    _osx_window = NULL;
  }
 
  if (_holder_aglcontext) {
    aglDestroyContext(_holder_aglcontext); 
    _holder_aglcontext = NULL;
  }
  
  if (_pending_icon != NULL) {
    CGImageRelease(_pending_icon);
    _pending_icon = NULL;
  }

  if (_current_icon != NULL) {
    CGImageRelease(_current_icon);
    _current_icon = NULL;
  }

  if (!destructing) {
    WindowProperties properties;
    properties.set_foreground(false);
    properties.set_open(false);
    properties.set_cursor_filename(Filename());
    system_changed_properties(properties);
  }
    
  _is_fullscreen = false; 
  _osx_window = NULL;
}


static int id_seed = 100;

////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsWindow::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
osxGraphicsWindow::
osxGraphicsWindow(GraphicsEngine *engine, GraphicsPipe *pipe, 
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
#ifdef HACK_SCREEN_HASH_CONTEXT 
  _holder_aglcontext(NULL),
#endif 
  _originalMode(NULL),
  _ID(id_seed++)
{
  GraphicsWindowInputDevice device =
    GraphicsWindowInputDevice::pointer_and_keyboard(this, "keyboard/mouse");
  _input_devices.push_back(device);
  _input_devices[0].set_pointer_in_window(0, 0);
  _last_key_modifiers = 0;
  _last_buttons = 0;

  _cursor_hidden = false;
  _display_hide_cursor = false;
  _wheel_hdelta = 0;
  _wheel_vdelta = 0;
 
  if (osxdisplay_cat.is_debug()) {
    osxdisplay_cat.debug()
      << "osxGraphicsWindow::osxGraphicsWindow() -" <<_ID << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsWindow::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
osxGraphicsWindow::
~osxGraphicsWindow() {
  if (osxdisplay_cat.is_debug()) {
    osxdisplay_cat.debug()
      << "osxGraphicsWindow::~osxGraphicsWindow() -" <<_ID << "\n";
  }

  // Make sure the window callback won't come back to this
  // (destructed) object any more.
  if (_osx_window) {
    SetWRefCon(_osx_window, (long) NULL);
  }

  release_system_resources(true); 
}

///////////////////////////////////////////////////////////////////
//     Function: osxGraphicsWindow::get_context
//       Access: Private
//  Description: Helper to decide whitch context to use if any
////////////////////////////////////////////////////////////////////
AGLContext osxGraphicsWindow::
get_context() {
  if (_holder_aglcontext != NULL) {
    return _holder_aglcontext;
  }

  return get_gsg_context();
}

///////////////////////////////////////////////////////////////////
//     Function: osxGraphicsWindow::get_gsg_context
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
AGLContext osxGraphicsWindow::
get_gsg_context() {
  if (_gsg != NULL) {
    osxGraphicsStateGuardian *osxgsg = NULL;
    osxgsg = DCAST(osxGraphicsStateGuardian, _gsg); 
    return osxgsg->get_context();
  }
  return NULL;
}

///////////////////////////////////////////////////////////////////
//     Function: osxGraphicsWindow::build_gl
//       Access: Private
//  Description: Code of the class.. used to control the GL context
//               Allocation.
////////////////////////////////////////////////////////////////////
OSStatus osxGraphicsWindow::
build_gl(bool full_screen) {
  // make sure the gsg is up and runnig..
  osxGraphicsStateGuardian *osxgsg = NULL;
  osxgsg = DCAST(osxGraphicsStateGuardian, _gsg);
  OSStatus stat = osxgsg->build_gl(full_screen, false, _fb_properties);

  if (stat != noErr) {
    return stat;
  }
 
  OSStatus err = noErr;
 
  if (osxgsg->get_agl_pixel_format()) {
    _holder_aglcontext = aglCreateContext(osxgsg->get_agl_pixel_format(), NULL);
 
    err = report_agl_error("aglCreateContext");
    if (_holder_aglcontext == NULL) {
      osxdisplay_cat.error() 
        << "osxGraphicsWindow::build_gl Error aglCreateContext \n";
      if (err ==noErr) {
        err = -1; 
      }
    } else { 
      aglSetInteger(_holder_aglcontext, AGL_BUFFER_NAME, &osxgsg->_shared_buffer); 
      err = report_agl_error ("aglSetInteger AGL_BUFFER_NAME");
    }
  } else {
    osxdisplay_cat.error()
      << "osxGraphicsWindow::build_gl Error Getting PixelFormat \n"; 
    if (err ==noErr) {
      err = -1;
    }
  }
  return err;
}

////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsWindow::set_icon_filename
//       Access: Private
//  Description: Called internally to load up an icon file that should
//               be applied to the window. Returns true on success,
//               false on failure.
////////////////////////////////////////////////////////////////////
bool osxGraphicsWindow::
set_icon_filename(const Filename &icon_filename) {
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

  Filename icon_pathname = icon_filename;
  if (!vfs->resolve_filename(icon_pathname, get_model_path())) {
    // The filename doesn't exist along the search path.
    if (icon_pathname.is_fully_qualified() && vfs->exists(icon_pathname)) {
      // But it does exist locally, so accept it.

    } else {
      osxdisplay_cat.warning()
        << "Could not find icon filename " << icon_filename << "\n";
      return false;
    }
  }


  PNMImage pnmimage;
  if (!pnmimage.read(icon_pathname)) {
    osxdisplay_cat.warning()
      << "Could not read icon filename " << icon_pathname << "\n";
    return false;
  }
  
  CGImageRef icon_image = osxGraphicsPipe::create_cg_image(pnmimage);
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
//     Function: osxGraphicsWindow::set_pointer_in_window
//       Access: Private
//  Description: Indicates the mouse pointer is seen within the
//               window.
////////////////////////////////////////////////////////////////////
void osxGraphicsWindow::
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
//     Function: osxGraphicsWindow::set_pointer_out_of_window
//       Access: Private
//  Description: Indicates the mouse pointer is no longer within the
//               window.
////////////////////////////////////////////////////////////////////
void osxGraphicsWindow::
set_pointer_out_of_window() {
  _input_devices[0].set_pointer_out_of_window();

  if (_display_hide_cursor) {
    CGDisplayShowCursor(kCGDirectMainDisplay); 
    _display_hide_cursor = false;
  }
}


////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsWindow::begin_frame
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               before beginning rendering for a given frame. It
//               should do whatever setup is required, and return true
//               if the frame should be rendered, or false if it
//               should be skipped.
////////////////////////////////////////////////////////////////////
bool osxGraphicsWindow::
begin_frame(FrameMode mode, Thread *current_thread) {
  PStatTimer timer(_make_current_pcollector);
 
  begin_frame_spam(mode);
  if (_gsg == (GraphicsStateGuardian *)NULL || 
      (_osx_window == NULL && !_is_fullscreen)) {
    // not powered up .. just abort..
    return false;
  }
 
  // Now is a good time to apply the icon change that may have
  // recently been requested. By this point, we should be able to get
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
  
  if (_is_fullscreen) {
    aglSetFullScreen(get_gsg_context(),0,0,0,0);
    report_agl_error ("aglSetFullScreen"); 
   
  } else {
    if (full_screen_window != NULL) {
      return false;
    }
    
    if (!aglSetDrawable(get_gsg_context(), GetWindowPort (_osx_window))) {
      report_agl_error("aglSetDrawable");
    }
  }

  if (!aglSetCurrentContext(get_gsg_context())) {
    report_agl_error ("aglSetCurrentContext"); 
  }
 
  _gsg->reset_if_new();
  _gsg->set_current_properties(&get_fb_properties());

  return _gsg->begin_frame(current_thread);
}

////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsWindow::end_frame
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               after rendering is completed for a given frame. It
//               should do whatever finalization is required.
////////////////////////////////////////////////////////////////////
void osxGraphicsWindow::
end_frame(FrameMode mode, Thread *current_thread) {
  end_frame_spam(mode);
 
  if (mode == FM_render) {
    nassertv(_gsg != (GraphicsStateGuardian *)NULL);

    copy_to_textures();

    if (!_properties.get_fixed_size() && 
        !_properties.get_undecorated() && 
        !_properties.get_fullscreen() &&
        show_resize_box) {
      // Draw a kludgey little resize box in the corner of the window,
      // so the user knows he's supposed to be able to drag the window
      // if he wants.
      DisplayRegionPipelineReader dr_reader(_overlay_display_region, current_thread);
      _gsg->prepare_display_region(&dr_reader);
      DCAST(osxGraphicsStateGuardian, _gsg)->draw_resize_box();
    }
    
    aglSwapBuffers (get_gsg_context());
    _gsg->end_frame(current_thread);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsWindow::begin_flip
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
void osxGraphicsWindow::
end_flip() {
  // cerr << " end_flip [" << _ID << "]\n";
}

void osxGraphicsWindow::
begin_flip() {
  // this forces a rip to proper context
  // cerr << " begin_flip [" << _ID << "]\n";
  return;
 
  if (_is_fullscreen) {
    if (!aglSetFullScreen(get_gsg_context(),0,0,0,0)) {
      report_agl_error("aglSetFullScreen"); 
    }
    
    if (!aglSetCurrentContext(get_gsg_context())) {
      report_agl_error("aglSetCurrentContext");
    }
 
    aglSwapBuffers (get_gsg_context());
  } else {
    if (!aglSetDrawable (get_gsg_context(),GetWindowPort (_osx_window))) {
      report_agl_error("aglSetDrawable");
    }
    
    if (!aglSetCurrentContext(get_gsg_context())) {
      report_agl_error("aglSetCurrentContext");
    } 
    
    aglSwapBuffers (get_gsg_context());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsWindow::close_window
//       Access: Protected, Virtual
//  Description: Closes the window right now. Called from the window
//               thread.
////////////////////////////////////////////////////////////////////
void osxGraphicsWindow::
close_window() {
  system_close_window();

  WindowProperties properties;
  properties.set_open(false);
  system_changed_properties(properties);

  release_system_resources(false);
  _gsg.clear();
 GraphicsWindow::close_window();
}

//////////////////////////////////////////////////////////
// HACK ALLERT ************ Undocumented OSX calls...
// I can not find any other way to get the mouse focus to a window in OSX..
//
//extern "C" {
// struct CPSProcessSerNum
// {
// UInt32 lo;
// UInt32 hi;
// };
///
//extern OSErr CPSGetCurrentProcess(CPSProcessSerNum *psn);
//extern OSErr CPSEnableForegroundOperation(struct CPSProcessSerNum *psn);
//extern OSErr CPSSetProcessName (struct CPSProcessSerNum *psn, char *processname);
//extern OSErr CPSSetFrontProcess(struct CPSProcessSerNum *psn);
//};

////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsWindow::open_window
//       Access: Protected, Virtual
//  Description: Opens the window right now. Called from the window
//               thread. Returns true if the window is successfully
//               opened, or false if there was a problem.
////////////////////////////////////////////////////////////////////
bool osxGraphicsWindow::
open_window() {
  WindowProperties req_properties = _properties;
 
  if (_gsg == 0) {
    _gsg = new osxGraphicsStateGuardian(_engine, _pipe, NULL);
  }
 
  //osx_global_mutex().lock();
  bool answer = os_open_window(req_properties);
  //osx_global_mutex().release();
  return answer;
}

////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsWindow::os_open_window
//       Access: Private
//  Description: Actually makes the OS calls to open a window.
////////////////////////////////////////////////////////////////////
bool osxGraphicsWindow::
os_open_window(WindowProperties &req_properties) {
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
    
    EventHandlerRef application_event_ref_ref1;
    EventTypeSpec list1[] = { 
      //{ kEventClassCommand, kEventProcessCommand },
      //{ kEventClassCommand, kEventCommandUpdateStatus },
      { kEventClassMouse, kEventMouseDown },// handle trackball functionality globaly because there is only a single user
      { kEventClassMouse, kEventMouseUp }, 
      { kEventClassMouse, kEventMouseMoved },
      { kEventClassMouse, kEventMouseDragged },
      { kEventClassMouse, kEventMouseWheelMoved } ,
      { kEventClassKeyboard, kEventRawKeyDown },
      { kEventClassKeyboard, kEventRawKeyUp } ,
      { kEventClassKeyboard, kEventRawKeyRepeat },
      { kEventClassKeyboard, kEventRawKeyModifiersChanged } ,
      { kEventClassTextInput, kEventTextInputUnicodeForKeyEvent}, 
    };
    
    EventHandlerUPP gEvtHandler = NewEventHandlerUPP(app_event_handler);
    err = InstallApplicationEventHandler (gEvtHandler, GetEventTypeCount (list1) , list1, this, &application_event_ref_ref1);
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
    if (osxdisplay_cat.is_debug()) {
      osxdisplay_cat.debug()
        << "Creating full screen\n";
    }

    // capture the main display
    CGDisplayCapture(kCGDirectMainDisplay);
    // if sized try and switch it..
    if (req_properties.has_size()) {
      _originalMode = CGDisplayCurrentMode(kCGDirectMainDisplay); 
      CFDictionaryRef newMode = CGDisplayBestModeForParameters(kCGDirectMainDisplay, 32, req_properties.get_x_size(), req_properties.get_y_size(), 0);
      if (newMode == NULL) {
        osxdisplay_cat.error()
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
    
    if (build_gl(true) != noErr) {
      if (_originalMode != NULL) {
        CGDisplaySwitchToMode(kCGDirectMainDisplay, _originalMode);
      }
      _originalMode = NULL;
      
      CGDisplayRelease(kCGDirectMainDisplay);
      return false; 
    } 
    
    _properties.set_fullscreen(true);
    _properties.set_minimized(false);
    _properties.set_foreground(true);

    _is_fullscreen = true; 
    full_screen_window = this;
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
      if (osxdisplay_cat.is_debug()) {
        osxdisplay_cat.debug()
          << "Creating child window\n";
      }
        
      CreateNewWindow(kSimpleWindowClass, kWindowNoAttributes, &r, &_osx_window);
      add_a_window(_osx_window);
      
      _properties.set_fixed_size(true);
      if (osxdisplay_cat.is_debug()) {
        osxdisplay_cat.debug()
          << "Child window created\n";
      }
      } else */
    {
      int attributes = kWindowStandardDocumentAttributes | kWindowStandardHandlerAttribute;
      if (req_properties.has_fixed_size() && req_properties.get_fixed_size()) {
        attributes &= ~kWindowResizableAttribute;
      }

      if (req_properties.has_undecorated() && req_properties.get_undecorated()) { 
        // create a unmovable .. no edge window..
          
        if (osxdisplay_cat.is_debug()) {
          osxdisplay_cat.debug()
            << "Creating undecorated window\n";
        }
 
        // We don't want a resize box either.
        attributes &= ~kWindowResizableAttribute;
        attributes |= kWindowNoTitleBarAttribute;
        CreateNewWindow(kDocumentWindowClass, attributes, &r, &_osx_window);
      } else { 
        // create a window with crome and sizing and sucj
        // In this case, we want to constrain the window to the
        // available size.
        
        Rect bounds;
        GetAvailableWindowPositioningBounds(GetMainDevice(), &bounds);
 
        r.left = max(r.left, bounds.left);
        r.right = min(r.right, bounds.right);
        r.top = max(r.top, bounds.top);
        r.bottom = min(r.bottom, bounds.bottom);
        
        if (osxdisplay_cat.is_debug()) {
          osxdisplay_cat.debug()  
            << "Creating standard window\n";
        }
        CreateNewWindow(kDocumentWindowClass, attributes, &r, &_osx_window);
        add_a_window(_osx_window);
      }
    }
    
    if (_osx_window) {
      EventHandlerUPP gWinEvtHandler; // window event handler
      EventTypeSpec list[] = { 
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

      // point to the window record in the ref con of the window
      SetWRefCon(_osx_window, (long) this);
      gWinEvtHandler = NewEventHandlerUPP(window_event_handler); 
      InstallWindowEventHandler(_osx_window, gWinEvtHandler, GetEventTypeCount(list), list, (void*)this, NULL); // add event handler

      ShowWindow (_osx_window);
 
      if (osxdisplay_cat.is_debug()) {
        osxdisplay_cat.debug()
          << "Event handler installed, now build_gl\n";
      }
      if (build_gl(false) != noErr) {
        osxdisplay_cat.error()
          << "Error in build_gl\n";
 
        HideWindow(_osx_window);
        SetWRefCon(_osx_window, (long int) NULL);
        DisposeWindow(_osx_window);
        _osx_window = NULL;
        return false;
      }
      
      if (osxdisplay_cat.is_debug()) {
        osxdisplay_cat.debug()
          << "build_gl complete, set properties\n";
      }

      //
      // attach the holder context to the window..
      //
      
      if (!aglSetDrawable(_holder_aglcontext, GetWindowPort(_osx_window))) {
        err = report_agl_error("aglSetDrawable");
      }
      
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
    _properties.set_size((int)(rectPort.right - rectPort.left),(int) (rectPort.bottom - rectPort.top));
    req_properties.clear_size();
    req_properties.clear_origin();
  }
  
  if (req_properties.has_icon_filename()) {
    set_icon_filename(req_properties.get_icon_filename());
  }

  if (req_properties.has_cursor_hidden()) { 
    _properties.set_cursor_hidden(req_properties.get_cursor_hidden()); 
    _cursor_hidden = req_properties.get_cursor_hidden();
    if (_cursor_hidden) {
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
    req_properties.clear_cursor_hidden();
  }

  _properties.set_open(true);

  if (_properties.has_size()) {
    set_size_and_recalc(_properties.get_x_size(), _properties.get_y_size());
  }
 
  return (err == noErr);
}

////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsWindow::process_events
//       Access: Protected, Virtual
//  Description: Required event upcall, used to dispatch window and
//               application events back into panda.
////////////////////////////////////////////////////////////////////
void osxGraphicsWindow::
process_events() {
  GraphicsWindow::process_events();

  if (!osx_disable_event_loop) {
    EventRef theEvent;
    EventTargetRef theTarget = GetEventDispatcherTarget();
    
    /*if (!_properties.has_parent_window()) */ {
      while (ReceiveNextEvent(0, NULL, kEventDurationNoWait, true, &theEvent)== noErr) {
        SendEventToEventTarget (theEvent, theTarget);
        ReleaseEvent(theEvent);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsWindow::handle_key_input
//       Access: Protected, Virtual
//  Description: Required event upcall, used to dispatch window and
//               application events back into panda.
////////////////////////////////////////////////////////////////////
OSStatus osxGraphicsWindow::
handle_key_input(EventHandlerCallRef my_handler, EventRef event, 
                 Boolean key_down) {
  if (osxdisplay_cat.is_debug()) {
    UInt32 key_code;
    GetEventParameter(event, kEventParamKeyCode, typeUInt32, NULL, 
                      sizeof(UInt32), NULL, &key_code);

    osxdisplay_cat.debug()
      << ClockObject::get_global_clock()->get_real_time() 
      << " handle_key_input: " << (void *)this << ", " << key_code 
      << ", " << (int)key_down << "\n";
  }
 
  //CallNextEventHandler(my_handler, event);

  // We don't check the result of the above function. In principle,
  // this should return eventNotHandledErr if the key event is not
  // handled by the OS, but in practice, testing this just seems to
  // eat the Escape keypress meaninglessly. Keypresses like F11 that
  // are already mapped in the desktop seem to not even come into this
  // function in the first place.
  UInt32 new_modifiers = 0;
  OSStatus error = GetEventParameter(event, kEventParamKeyModifiers, 
                                     typeUInt32, NULL, sizeof(UInt32), 
                                     NULL, &new_modifiers);
  if (error == noErr) {
    handle_modifier_delta(new_modifiers);
  }
 
  UInt32 key_code;
  GetEventParameter(event, kEventParamKeyCode, typeUInt32, NULL, 
                    sizeof(UInt32), NULL, &key_code);
  ButtonHandle button = osx_translate_key(key_code, event);

  if (key_down) {
    if ((new_modifiers & cmdKey) != 0) {
      if (button == KeyboardButton::ascii_key("q") || 
          button == KeyboardButton::ascii_key("w")) {
        // Command-Q or Command-W: quit the application or close the
        // window, respectively. For now, we treat them both the
        // same: close the window.
        user_close_request();
      }
    }
    send_key_event(button, true);
  } else {
    send_key_event(button, false);
  }

  return CallNextEventHandler(my_handler, event);
  // return noErr;
}

////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsWindow::system_set_window_foreground
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void osxGraphicsWindow::
system_set_window_foreground(bool foreground) {
  WindowProperties properties;
  properties.set_foreground(foreground);
  system_changed_properties(properties);
} 
 
////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsWindow::system_point_to_local_point
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void osxGraphicsWindow::
system_point_to_local_point(Point &global_point) {
  if (_osx_window != NULL) {
    GrafPtr savePort;
    Boolean port_changed = QDSwapPort(GetWindowPort(_osx_window), &savePort);
 
    GlobalToLocal(&global_point);
 
    if (port_changed) {
      QDSwapPort(savePort, NULL);
    }
  } 
}
 
////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsWindow::handle_mouse_window_events
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
OSStatus osxGraphicsWindow::
handle_window_mouse_events(EventHandlerCallRef my_handler, EventRef event) {
  WindowRef window = NULL;
  OSStatus result = eventNotHandledErr;
  UInt32 kind = GetEventKind(event);
  EventMouseButton button = 0;
  Point global_point = {0, 0};
  UInt32 modifiers = 0; 
  Rect rect_port;
  SInt32 this_wheel_delta;
  EventMouseWheelAxis wheelAxis;

  // cerr <<" Start Mouse Event " << _ID << "\n";

  // Mac OS X v10.1 and later
  // should this be front window???
  GetEventParameter(event, kEventParamWindowRef, typeWindowRef, NULL, 
                    sizeof(WindowRef), NULL, &window);

  if (!_is_fullscreen && (window == NULL || window != _osx_window)) {
    if (kind == kEventMouseMoved) {
      set_pointer_out_of_window();
    }
    return eventNotHandledErr;
  }

  GetWindowPortBounds(window, &rect_port);

  // result = CallNextEventHandler(my_handler, event); 
  // if (eventNotHandledErr == result) 
  { // only handle events not already handled (prevents weird resize interaction)
    switch (kind) {
      // Whenever mouse button state changes, generate the
      // appropriate Panda down/up events to represent the
      // change.

    case kEventMouseDown:
    case kEventMouseUp:
      {
        GetEventParameter(event, kEventParamKeyModifiers, typeUInt32, NULL, sizeof(UInt32), NULL, &modifiers);
        if (_properties.get_mouse_mode() == WindowProperties::M_relative) {
          HIPoint delta;
          GetEventParameter(event, kEventParamMouseDelta, typeHIPoint, NULL, sizeof(HIPoint), NULL, (void*) &delta);

          MouseData currMouse = get_pointer(0);
          delta.x += currMouse.get_x();
          delta.y += currMouse.get_y();
          set_pointer_in_window((int)delta.x, (int)delta.y);
        } else {
          GetEventParameter(event, kEventParamMouseLocation, typeQDPoint, NULL, sizeof(Point), NULL , (void*) &global_point); 
          system_point_to_local_point(global_point);
          set_pointer_in_window((int)global_point.h, (int)global_point.v);
        }

        UInt32 new_buttons = GetCurrentEventButtonState();
        handle_button_delta(new_buttons);
      }
      result = noErr;
      break;

    case kEventMouseMoved: 
    case kEventMouseDragged:
      if (_properties.get_mouse_mode() == WindowProperties::M_relative) {
        HIPoint delta;
        GetEventParameter(event, kEventParamMouseDelta, typeHIPoint, NULL, sizeof(HIPoint), NULL, (void*) &delta);
        
        MouseData currMouse = get_pointer(0);
        delta.x += currMouse.get_x();
        delta.y += currMouse.get_y();
        set_pointer_in_window((int)delta.x, (int)delta.y);
      } else {
        GetEventParameter(event, kEventParamMouseLocation, typeQDPoint, NULL, sizeof(Point), NULL, (void*) &global_point);
        system_point_to_local_point(global_point);

        if (kind == kEventMouseMoved && 
            (global_point.h < 0 || global_point.v < 0)) {
          // Moving into the titlebar region.
          set_pointer_out_of_window();
        } else {
          // Moving within the window itself (or dragging anywhere).
          set_pointer_in_window((int)global_point.h, (int)global_point.v);
        }
      }
      result = noErr;
      break;
 
    case kEventMouseWheelMoved: 
      GetEventParameter(event, kEventParamMouseWheelDelta, typeLongInteger, NULL, sizeof(this_wheel_delta), NULL, &this_wheel_delta);
      GetEventParameter(event, kEventParamMouseWheelAxis, typeMouseWheelAxis, NULL, sizeof(wheelAxis), NULL, &wheelAxis);
      GetEventParameter(event, kEventParamMouseLocation,typeQDPoint, NULL, sizeof(Point),NULL , (void*) &global_point);
      system_point_to_local_point(global_point);
 
      if (wheelAxis == kEventMouseWheelAxisX) {
        set_pointer_in_window((int)global_point.h, (int)global_point.v);
        _wheel_hdelta += this_wheel_delta;
        SInt32 wheel_scale = osx_mouse_wheel_scale;
        while (_wheel_hdelta > wheel_scale) {
          _input_devices[0].button_down(MouseButton::wheel_left());
          _input_devices[0].button_up(MouseButton::wheel_left());
          _wheel_hdelta -= wheel_scale;
        }
        while (_wheel_hdelta < -wheel_scale) {
          _input_devices[0].button_down(MouseButton::wheel_right());
          _input_devices[0].button_up(MouseButton::wheel_right());
          _wheel_hdelta += wheel_scale;
        }
      }
      if (wheelAxis == kEventMouseWheelAxisY) {
        set_pointer_in_window((int)global_point.h, (int)global_point.v);
        _wheel_vdelta += this_wheel_delta;
        SInt32 wheel_scale = osx_mouse_wheel_scale;
        while (_wheel_vdelta > wheel_scale) {
          _input_devices[0].button_down(MouseButton::wheel_up());
          _input_devices[0].button_up(MouseButton::wheel_up());
          _wheel_vdelta -= wheel_scale;
        }
        while (_wheel_vdelta < -wheel_scale) {
          _input_devices[0].button_down(MouseButton::wheel_down());
          _input_devices[0].button_up(MouseButton::wheel_down());
          _wheel_vdelta += wheel_scale;
        }
      }
      result = noErr;
      break; 
    }
    // result = noErr;
  } 
 
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsWindow::osx_translate_key
//       Access: Private
//  Description: MAC Key Codes to Panda Key Codes
////////////////////////////////////////////////////////////////////
ButtonHandle osxGraphicsWindow::
osx_translate_key(UInt32 key, EventRef event) {
  ButtonHandle nk = ButtonHandle::none();
  switch (key) {
  case   0: nk = KeyboardButton::ascii_key('a'); break;
  case  11: nk = KeyboardButton::ascii_key('b'); break;
  case   8: nk = KeyboardButton::ascii_key('c'); break;
  case   2: nk = KeyboardButton::ascii_key('d'); break;
  case  14: nk = KeyboardButton::ascii_key('e'); break;
  case   3: nk = KeyboardButton::ascii_key('f'); break;
  case   5: nk = KeyboardButton::ascii_key('g'); break;
  case   4: nk = KeyboardButton::ascii_key('h'); break;
  case  34: nk = KeyboardButton::ascii_key('i'); break;
  case  38: nk = KeyboardButton::ascii_key('j'); break;
  case  40: nk = KeyboardButton::ascii_key('k'); break;
  case  37: nk = KeyboardButton::ascii_key('l'); break;
  case  46: nk = KeyboardButton::ascii_key('m'); break;
  case  45: nk = KeyboardButton::ascii_key('n'); break;
  case  31: nk = KeyboardButton::ascii_key('o'); break;
  case  35: nk = KeyboardButton::ascii_key('p'); break;
  case  12: nk = KeyboardButton::ascii_key('q'); break;
  case  15: nk = KeyboardButton::ascii_key('r'); break;
  case   1: nk = KeyboardButton::ascii_key('s'); break;
  case  17: nk = KeyboardButton::ascii_key('t'); break;
  case  32: nk = KeyboardButton::ascii_key('u'); break;
  case   9: nk = KeyboardButton::ascii_key('v'); break;
  case  13: nk = KeyboardButton::ascii_key('w'); break;
  case   7: nk = KeyboardButton::ascii_key('x'); break;
  case  16: nk = KeyboardButton::ascii_key('y'); break;
  case   6: nk = KeyboardButton::ascii_key('z'); break;

    // top row numbers
  case  29: nk = KeyboardButton::ascii_key('0'); break;
  case  18: nk = KeyboardButton::ascii_key('1'); break;
  case  19: nk = KeyboardButton::ascii_key('2'); break;
  case  20: nk = KeyboardButton::ascii_key('3'); break;
  case  21: nk = KeyboardButton::ascii_key('4'); break;
  case  23: nk = KeyboardButton::ascii_key('5'); break;
  case  22: nk = KeyboardButton::ascii_key('6'); break;
  case  26: nk = KeyboardButton::ascii_key('7'); break;
  case  28: nk = KeyboardButton::ascii_key('8'); break;
  case  25: nk = KeyboardButton::ascii_key('9'); break;

    // key pad ... do they really map to the top number in panda ?
  case  82: nk = KeyboardButton::ascii_key('0'); break;
  case  83: nk = KeyboardButton::ascii_key('1'); break;
  case  84: nk = KeyboardButton::ascii_key('2'); break;
  case  85: nk = KeyboardButton::ascii_key('3'); break;
  case  86: nk = KeyboardButton::ascii_key('4'); break;
  case  87: nk = KeyboardButton::ascii_key('5'); break;
  case  88: nk = KeyboardButton::ascii_key('6'); break;
  case  89: nk = KeyboardButton::ascii_key('7'); break;
  case  91: nk = KeyboardButton::ascii_key('8'); break;
  case  92: nk = KeyboardButton::ascii_key('9'); break;

    // case  36: nk = KeyboardButton::ret(); break; // no return in panda ???
  case  49: nk = KeyboardButton::space(); break;
  case  51: nk = KeyboardButton::backspace(); break;
  case  48: nk = KeyboardButton::tab(); break;
  case  53: nk = KeyboardButton::escape(); break;
  case  76: nk = KeyboardButton::enter(); break; 
  case  36: nk = KeyboardButton::enter(); break; 

  case 123: nk = KeyboardButton::left(); break;
  case 124: nk = KeyboardButton::right(); break;
  case 125: nk = KeyboardButton::down(); break;
  case 126: nk = KeyboardButton::up(); break;
  case 116: nk = KeyboardButton::page_up(); break;
  case 121: nk = KeyboardButton::page_down(); break;
  case 115: nk = KeyboardButton::home(); break;
  case 119: nk = KeyboardButton::end(); break;
  case 114: nk = KeyboardButton::help(); break; 
  case 117: nk = KeyboardButton::del(); break; 

    // case  71: nk = KeyboardButton::num_lock() break; 

  case 122: nk = KeyboardButton::f1(); break;
  case 120: nk = KeyboardButton::f2(); break;
  case  99: nk = KeyboardButton::f3(); break;
  case 118: nk = KeyboardButton::f4(); break;
  case  96: nk = KeyboardButton::f5(); break;
  case  97: nk = KeyboardButton::f6(); break;
  case  98: nk = KeyboardButton::f7(); break;
  case 100: nk = KeyboardButton::f8(); break;
  case 101: nk = KeyboardButton::f9(); break;
  case 109: nk = KeyboardButton::f10(); break;
  case 103: nk = KeyboardButton::f11(); break;
  case 111: nk = KeyboardButton::f12(); break;

  case 105: nk = KeyboardButton::f13(); break;
  case 107: nk = KeyboardButton::f14(); break;
  case 113: nk = KeyboardButton::f15(); break;
  case 106: nk = KeyboardButton::f16(); break;

    // shiftable chartablet 
  case  50: nk = KeyboardButton::ascii_key('`'); break;
  case  27: nk = KeyboardButton::ascii_key('-'); break;
  case  24: nk = KeyboardButton::ascii_key('='); break;
  case  33: nk = KeyboardButton::ascii_key('['); break;
  case  30: nk = KeyboardButton::ascii_key(']'); break;
  case  42: nk = KeyboardButton::ascii_key('\\'); break;
  case  41: nk = KeyboardButton::ascii_key(';'); break;
  case  39: nk = KeyboardButton::ascii_key('\''); break;
  case  43: nk = KeyboardButton::ascii_key(','); break;
  case  47: nk = KeyboardButton::ascii_key('.'); break;
  case  44: nk = KeyboardButton::ascii_key('/'); break;

  default:
    if (osxdisplay_cat.is_debug()) {
      osxdisplay_cat.debug()
        << " Untranslated KeyCode: " << key
        << " (0x" << hex << key << dec << ")\n";
    }

    // not sure this is right .. but no mapping for keypad and such
    // this at least does a best gess..
 
    char charCode = 0; 
    if (GetEventParameter(event, kEventParamKeyMacCharCodes, typeChar, nil, sizeof(charCode), nil, &charCode) == noErr) {
      nk = KeyboardButton::ascii_key(charCode); 
    }
  }
  return nk;
}

////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsWindow::handle_modifier_delta
//       Access: Private
//  Description: Used to emulate key events for the MAC key modifiers.
////////////////////////////////////////////////////////////////////
void osxGraphicsWindow::
handle_modifier_delta(UInt32 new_modifiers) {
  UInt32 changed = _last_key_modifiers ^ new_modifiers;

  if ((changed & (shiftKey | rightShiftKey)) != 0) {
    send_key_event(KeyboardButton::shift(),(new_modifiers & (shiftKey | rightShiftKey)) != 0) ;
  }

  if ((changed & (optionKey | rightOptionKey)) != 0) {
    send_key_event(KeyboardButton::alt(),(new_modifiers & (optionKey | rightOptionKey)) != 0);
  }

  if ((changed & (controlKey | rightControlKey)) != 0) {
    send_key_event(KeyboardButton::control(),(new_modifiers & (controlKey | rightControlKey)) != 0);
  }

  if ((changed & cmdKey) != 0) {
    send_key_event(KeyboardButton::meta(),(new_modifiers & cmdKey) != 0);
  }
 
  if ((changed & alphaLock) != 0) {
    send_key_event(KeyboardButton::caps_lock(),(new_modifiers & alphaLock) != 0);
  }
 
  // save current state
  _last_key_modifiers = new_modifiers; 
}

////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsWindow::handle_button_delta
//       Access: Private
//  Description: Used to emulate button events
////////////////////////////////////////////////////////////////////
void osxGraphicsWindow::
handle_button_delta(UInt32 new_buttons) {
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
//     Function: osxGraphicsWindow::move_pointer
//       Access: Published, Virtual
//  Description: Forces the pointer to the indicated position within
//               the window, if possible. 
//
//               Returns true if successful, false on failure. This
//               may fail if the mouse is not currently within the
//               window, or if the API doesn't support this operation.
////////////////////////////////////////////////////////////////////
bool osxGraphicsWindow::
move_pointer(int device, int x, int y) { 
  if (_osx_window == NULL) {
    return false; 
  }
 
  if (osxdisplay_cat.is_debug()) {
    osxdisplay_cat.debug()
      << "move_pointer " << device <<" "<< x <<" "<< y <<"\n"; 
  }
 
  Point pt = { 0, 0 }; 
  pt.h = x; 
  pt.v = y; 
  set_pointer_in_window(x, y); 

  if (_properties.get_mouse_mode() == WindowProperties::M_absolute) {
    local_point_to_system_point(pt); 
    CGPoint new_position = { 0, 0 }; 
    new_position.x = pt.h; 
    new_position.y = pt.v; 
    mouse_mode_relative();
    CGWarpMouseCursorPosition(new_position); 
    mouse_mode_absolute();
  }
 
  return true; 
} 

////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsWindow::do_reshape_request
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
bool osxGraphicsWindow::
do_reshape_request(int x_origin, int y_origin, bool has_origin,
                   int x_size, int y_size) {
  if (osxdisplay_cat.is_debug()) {
    osxdisplay_cat.debug()
      << "Do Reshape\n";
  }

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
      NSWindow* parentWindow = (NSWindow *)_properties.get_parent_window();
      NSRect parentFrame = [parentWindow frame];
      
      MoveWindow(_osx_window, x_origin+parentFrame.origin.x, y_origin+parentFrame.origin.y, false);
    }
    } else */
  {
    // We sometimes get a bogus origin of (0, 0). As a special hack,
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
  return true;
} 

////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsWindow::set_properties_now
//       Access: Public, Virtual
//  Description: Applies the requested set of properties to the
//               window, if possible, for instance to request a change
//               in size or minimization status.
//
//               The window properties are applied immediately, rather
//               than waiting until the next frame. This implies that
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
void osxGraphicsWindow::
set_properties_now(WindowProperties &properties) {
  if (osxdisplay_cat.is_debug()) {
    osxdisplay_cat.debug()
      << "------------------------------------------------------\n";
    osxdisplay_cat.debug()
      << "set_properties_now " << properties << "\n";
  }
 
  GraphicsWindow::set_properties_now(properties);
  
  if (osxdisplay_cat.is_debug()) {
    osxdisplay_cat.debug()
      << "set_properties_now After Base Class" << properties << "\n";
  }
 
  // for some changes .. a full rebuild is required for the OS layer Window.
  // I think it is the chrome attribute and full screen behaviour.
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
    // Logic here is .. take a union of the properties .. with the
    // new allowed to overwrite the old states. and start a bootstrap
    // of a new window ..
 
    // get a copy of my properties..
    WindowProperties req_properties(_properties); 
    release_system_resources(false);
    req_properties.add_properties(properties); 
 
    os_open_window(req_properties); 
 
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

  // An icon filename means to load up the icon and save it. We can't
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
    if (_cursor_hidden) {
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

  if (osxdisplay_cat.is_debug()) {
    osxdisplay_cat.debug()
      << "set_properties_now Out....." << _properties << "\n";
  }

  return;
}

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsWindow::local_point_to_system_point
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void osxGraphicsWindow::
local_point_to_system_point(Point &local_point) { 
  if (_osx_window != NULL) { 
    GrafPtr save_port;
    Boolean port_changed = QDSwapPort(GetWindowPort(_osx_window), &save_port);
 
    LocalToGlobal(&local_point);
 
    if (port_changed) {
      QDSwapPort(save_port, NULL);
    }
  } 
} 

////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsWindow::mouse_mode_relative
//       Access: Protected, Virtual
//  Description: detaches mouse. Only mouse delta from now on. 
////////////////////////////////////////////////////////////////////
void osxGraphicsWindow::
mouse_mode_relative() {
  CGAssociateMouseAndMouseCursorPosition(false);
}


////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsWindow::mouse_mode_absolute
//       Access: Protected, Virtual
//  Description: reattaches mouse to location
////////////////////////////////////////////////////////////////////
void osxGraphicsWindow::
mouse_mode_absolute() {
  CGAssociateMouseAndMouseCursorPosition(true);
}
