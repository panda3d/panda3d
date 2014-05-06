// Filename: cocoaGraphicsWindow.mm
// Created by:  rdb (14May12)
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

#include "cocoaGraphicsWindow.h"
#include "cocoaGraphicsStateGuardian.h"
#include "config_cocoadisplay.h"
#include "cocoaGraphicsPipe.h"

#include "graphicsPipe.h"
#include "keyboardButton.h"
#include "mouseButton.h"
#include "clockObject.h"
#include "pStatTimer.h"
#include "textEncoder.h"
#include "throw_event.h"
#include "lightReMutexHolder.h"
#include "nativeWindowHandle.h"
#include "virtualFileSystem.h"

#import "cocoaPandaView.h"
#import "cocoaPandaWindow.h"

#import <ApplicationServices/ApplicationServices.h>
#import <Foundation/NSAutoreleasePool.h>
#import <AppKit/NSApplication.h>
#import <AppKit/NSCursor.h>
#import <AppKit/NSEvent.h>
#import <AppKit/NSImage.h>
#import <AppKit/NSScreen.h>
#import <OpenGL/OpenGL.h>
#import <Carbon/Carbon.h>

TypeHandle CocoaGraphicsWindow::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: CocoaGraphicsWindow::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CocoaGraphicsWindow::
CocoaGraphicsWindow(GraphicsEngine *engine, GraphicsPipe *pipe,
                    const string &name,
                    const FrameBufferProperties &fb_prop,
                    const WindowProperties &win_prop,
                    int flags,
                    GraphicsStateGuardian *gsg,
                    GraphicsOutput *host) :
  GraphicsWindow(engine, pipe, name, fb_prop, win_prop, flags, gsg, host)
{
  _window = nil;
  _view = nil;
  _cursor = nil;
  _modifier_keys = 0;
  _mouse_hidden = false;
  _context_needs_update = true;
  _fullscreen_mode = NULL;
  _windowed_mode = NULL;

  GraphicsWindowInputDevice device =
    GraphicsWindowInputDevice::pointer_and_keyboard(this, "keyboard_mouse");
  add_input_device(device);

  CocoaGraphicsPipe *cocoa_pipe;
  DCAST_INTO_V(cocoa_pipe, _pipe);
  _display = cocoa_pipe->_display;
}

////////////////////////////////////////////////////////////////////
//     Function: CocoaGraphicsWindow::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CocoaGraphicsWindow::
~CocoaGraphicsWindow() {
}

////////////////////////////////////////////////////////////////////
//     Function: CocoaGraphicsWindow::move_pointer
//       Access: Published, Virtual
//  Description: Forces the pointer to the indicated position within
//               the window, if possible.
//
//               Returns true if successful, false on failure.  This
//               may fail if the mouse is not currently within the
//               window, or if the API doesn't support this operation.
////////////////////////////////////////////////////////////////////
bool CocoaGraphicsWindow::
move_pointer(int device, int x, int y) {
  //Hack!  Will go away when we have floating-point mouse pos.
  MouseData md = get_pointer(device);
  if (md.get_x() == x && md.get_y() == y) {
    return true;
  }

  if (device == 0) {
    CGPoint point;
    if (_properties.get_fullscreen()) {
      point = CGPointMake(x, y + 1);
    } else {
      point = CGPointMake(x + _properties.get_x_origin(),
                          y + _properties.get_y_origin() + 1);
    }

    // I don't know what the difference between these two methods is.
    //if (CGWarpMouseCursorPosition(point) == kCGErrorSuccess) {
    if (CGDisplayMoveCursorToPoint(_display, point) == kCGErrorSuccess) {
      // Generate a mouse event.
      NSPoint pos = [_window mouseLocationOutsideOfEventStream];
      NSPoint loc = [_view convertPoint:pos fromView:nil];
      BOOL inside = [_view mouse:loc inRect:[_view bounds]];
      handle_mouse_moved_event(inside, loc.x, loc.y, true);
      return true;
    }
  } else {
    // No support for raw mice at the moment.
  }
  return false;
}


////////////////////////////////////////////////////////////////////
//     Function: CocoaGraphicsWindow::begin_frame
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               before beginning rendering for a given frame.  It
//               should do whatever setup is required, and return true
//               if the frame should be rendered, or false if it
//               should be skipped.
////////////////////////////////////////////////////////////////////
bool CocoaGraphicsWindow::
begin_frame(FrameMode mode, Thread *current_thread) {
  PStatTimer timer(_make_current_pcollector, current_thread);

  begin_frame_spam(mode);
  if (_gsg == (GraphicsStateGuardian *)NULL) {
    return false;
  }

  CocoaGraphicsStateGuardian *cocoagsg;
  DCAST_INTO_R(cocoagsg, _gsg, false);
  nassertr(cocoagsg->_context != nil, false);
  nassertr(_view != nil, false);

  // Place a lock on the context.
  CGLLockContext((CGLContextObj) [cocoagsg->_context CGLContextObj]);

  // Set the drawable.
  if (_properties.get_fullscreen()) {
    // Fullscreen.
    [cocoagsg->_context setFullScreen];
  } else {
    // Although not recommended, it is technically possible to
    // use the same context with multiple different-sized windows.
    // If that happens, the context needs to be updated accordingly.
    if ([cocoagsg->_context view] != _view) {
      //XXX I'm not 100% sure that changing the view requires it to update.
      _context_needs_update = true;
      [cocoagsg->_context setView:_view];

      cocoadisplay_cat.spam()
        << "Switching context to view " << _view << "\n";
    }
  }

  // Update the context if necessary, to make it reallocate buffers etc.
  if (_context_needs_update) {
    [cocoagsg->_context update];
    _context_needs_update = false;
  }

  // Lock the view for drawing.
  if (!_properties.get_fullscreen()) {
    nassertr([_view lockFocusIfCanDraw], false);
  }

  // Make the context current.
  [cocoagsg->_context makeCurrentContext];

  // Now that we have made the context current to a window, we can
  // reset the GSG state if this is the first time it has been used.
  // (We can't just call reset() when we construct the GSG, because
  // reset() requires having a current context.)
  cocoagsg->reset_if_new();

  if (mode == FM_render) {
    // begin_render_texture();
    clear_cube_map_selection();
  }

  _gsg->set_current_properties(&get_fb_properties());
  return _gsg->begin_frame(current_thread);
}

////////////////////////////////////////////////////////////////////
//     Function: CocoaGraphicsWindow::end_frame
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               after rendering is completed for a given frame.  It
//               should do whatever finalization is required.
////////////////////////////////////////////////////////////////////
void CocoaGraphicsWindow::
end_frame(FrameMode mode, Thread *current_thread) {
  end_frame_spam(mode);
  nassertv(_gsg != (GraphicsStateGuardian *)NULL);

  if (!_properties.get_fullscreen()) {
    [_view unlockFocus];
  }
  // Release the context.
  CocoaGraphicsStateGuardian *cocoagsg;
  DCAST_INTO_V(cocoagsg, _gsg);
  
  CGLUnlockContext((CGLContextObj) [cocoagsg->_context CGLContextObj]);

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

////////////////////////////////////////////////////////////////////
//     Function: CocoaGraphicsWindow::end_flip
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               after begin_flip() has been called on all windows, to
//               finish the exchange of the front and back buffers.
//
//               This should cause the window to wait for the flip, if
//               necessary.
////////////////////////////////////////////////////////////////////
void CocoaGraphicsWindow::
end_flip() {
  if (_gsg != (GraphicsStateGuardian *)NULL && _flip_ready) {

    CocoaGraphicsStateGuardian *cocoagsg;
    DCAST_INTO_V(cocoagsg, _gsg);

    CGLLockContext((CGLContextObj) [cocoagsg->_context CGLContextObj]);

    // Swap the front and back buffer.
    [cocoagsg->_context flushBuffer];

    // Flush the window
    [[_view window] flushWindow];

    CGLUnlockContext((CGLContextObj) [cocoagsg->_context CGLContextObj]);
  }
  GraphicsWindow::end_flip();
}

////////////////////////////////////////////////////////////////////
//     Function: CocoaGraphicsWindow::process_events
//       Access: Public, Virtual
//  Description: Do whatever processing is necessary to ensure that
//               the window responds to user events.  Also, honor any
//               requests recently made via request_properties()
//
//               This function is called only within the window
//               thread.
////////////////////////////////////////////////////////////////////
void CocoaGraphicsWindow::
process_events() {
  GraphicsWindow::process_events();

  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
  NSEvent *event = nil;

  while (true) {
    event = [NSApp
      nextEventMatchingMask:NSAnyEventMask
      untilDate:nil
      inMode:NSDefaultRunLoopMode
      dequeue:YES];

    if (event == nil) {
      break;
    }

    [NSApp sendEvent: event];
  }

  if (_window != nil) {
    [_window update];
  }

  [pool release];
}

////////////////////////////////////////////////////////////////////
//     Function: CocoaGraphicsWindow::open_window
//       Access: Protected, Virtual
//  Description: Opens the window right now.  Called from the window
//               thread.  Returns true if the window is successfully
//               opened, or false if there was a problem.
////////////////////////////////////////////////////////////////////
bool CocoaGraphicsWindow::
open_window() {
  CocoaGraphicsPipe *cocoa_pipe;
  DCAST_INTO_R(cocoa_pipe, _pipe, false);

  // GSG Creation/Initialization
  CocoaGraphicsStateGuardian *cocoagsg;
  if (_gsg == 0) {
    // There is no old gsg.  Create a new one.
    cocoagsg = new CocoaGraphicsStateGuardian(_engine, _pipe, NULL);
    cocoagsg->choose_pixel_format(_fb_properties, cocoa_pipe->_display, false);
    _gsg = cocoagsg;
  } else {
    // If the old gsg has the wrong pixel format, create a
    // new one that shares with the old gsg.
    DCAST_INTO_R(cocoagsg, _gsg, false);
    if (!cocoagsg->get_fb_properties().subsumes(_fb_properties)) {
      cocoagsg = new CocoaGraphicsStateGuardian(_engine, _pipe, cocoagsg);
      cocoagsg->choose_pixel_format(_fb_properties, cocoa_pipe->_display, false);
      _gsg = cocoagsg;
    }
  }

  // Fill in the blanks.
  if (!_properties.has_origin()) {
    _properties.set_origin(-2, -2);
  }
  if (!_properties.has_size()) {
    _properties.set_size(100, 100);
  }
  if (!_properties.has_fullscreen()) {
    _properties.set_fullscreen(false);
  }
  if (!_properties.has_foreground()) {
    _properties.set_foreground(true);
  }
  if (!_properties.has_undecorated()) {
    _properties.set_undecorated(false);
  }
  if (!_properties.has_fixed_size()) {
    _properties.set_fixed_size(false);
  }
  if (!_properties.has_minimized()) {
    _properties.set_minimized(false);
  }
  if (!_properties.has_z_order()) {
    _properties.set_z_order(WindowProperties::Z_normal);
  }
  if (!_properties.has_cursor_hidden()) {
    _properties.set_cursor_hidden(false);
  }

  // Check if we have a parent view.
  NSView *parent_nsview = nil;
  _parent_window_handle = NULL;

  WindowHandle *window_handle = _properties.get_parent_window();
  if (window_handle != NULL) {
    cocoadisplay_cat.info()
      << "Got parent_window " << *window_handle << "\n";

    WindowHandle::OSHandle *os_handle = window_handle->get_os_handle();
    if (os_handle != NULL) {
      cocoadisplay_cat.info()
        << "os_handle type " << os_handle->get_type() << "\n";

      void *ptr_handle;

      // Depending on whether the window handle comes from a Carbon or a Cocoa
      // application, it could be either a HIViewRef or an NSView or NSWindow.
      // Currently, we only support a Cocoa NSView, but we could in the future
      // add support for Carbon parents using HICocoaView.

      if (os_handle->is_of_type(NativeWindowHandle::IntHandle::get_class_type())) {
        NativeWindowHandle::IntHandle *int_handle = DCAST(NativeWindowHandle::IntHandle, os_handle);
        ptr_handle = (void*) int_handle->get_handle();
      }

      if (ptr_handle != NULL) {
        NSObject *nsobj = (NSObject *)ptr_handle;
        if ([nsobj isKindOfClass:[NSView class]]) {
          parent_nsview = (NSView *)nsobj;
          _parent_window_handle = window_handle;
          cocoadisplay_cat.info()
            << "Parent window handle is a valid NSView pointer\n";
        } else {
          cocoadisplay_cat.error()
            << "Parent window handle is not a valid NSView pointer!\n";
          return false;
        }
      }
    }
  }

  // Center the window if coordinates were set to -1 or -2
  //TODO: perhaps in future, in the case of -1, it should use the origin
  // used in a previous run of Panda
  NSRect container;
  if (parent_nsview != NULL) {
    container = [parent_nsview bounds];
  } else {
    container = [cocoa_pipe->_screen frame];
    container.origin = NSMakePoint(0, 0);
  }
  int x = _properties.get_x_origin();
  int y = _properties.get_y_origin();

  if (x < 0) {
    x = floor(container.size.width / 2 - _properties.get_x_size() / 2);
  }
  if (y < 0) {
    y = floor(container.size.height / 2 - _properties.get_y_size() / 2);
  }
  _properties.set_origin(x, y);

  if (_parent_window_handle == (WindowHandle *)NULL) {
    // Content rectangle
    NSRect rect;
    if (_properties.get_fullscreen()) {
      rect = container;
    } else {
      rect = NSMakeRect(x, container.size.height - _properties.get_y_size() - y,
                        _properties.get_x_size(), _properties.get_y_size());
    }

    // Configure the window decorations
    NSUInteger windowStyle;
    if (_properties.get_undecorated() || _properties.get_fullscreen()) {
      windowStyle = NSBorderlessWindowMask;
    } else if (_properties.get_fixed_size()) {
      // Fixed size windows should not show the resize button.
      windowStyle = NSTitledWindowMask | NSClosableWindowMask |
                    NSMiniaturizableWindowMask;
    } else {
      windowStyle = NSTitledWindowMask | NSClosableWindowMask |
                    NSMiniaturizableWindowMask | NSResizableWindowMask;
    }

    // Create the window.
    if (cocoadisplay_cat.is_debug()) {
      NSString *str = NSStringFromRect(rect);
      cocoadisplay_cat.debug()
        << "Creating NSWindow with content rect " << [str UTF8String] << "\n";
    }

    _window = [[CocoaPandaWindow alloc]
               initWithContentRect: rect
               styleMask:windowStyle
               screen:cocoa_pipe->_screen
               window:this];

    if (_window == nil) {
      cocoadisplay_cat.error()
        << "Failed to create Cocoa window.\n";
      return false;
    }
  }

  // Lock the context, so we can safely operate on it.
  CGLLockContext((CGLContextObj) [cocoagsg->_context CGLContextObj]);

  // Create the NSView to render to.
  NSRect rect = NSMakeRect(0, 0, _properties.get_x_size(), _properties.get_y_size());
  _view = [[CocoaPandaView alloc] initWithFrame:rect context:cocoagsg->_context window:this];
  if (_parent_window_handle == (WindowHandle *)NULL) {
    [_window setContentView:_view];
    [_window makeFirstResponder:_view];
  }

  // Check if we have an NSView to attach our NSView to.
  if (parent_nsview != NULL) {
    [parent_nsview addSubview:_view];
  }

  // Create a WindowHandle for ourselves.
  // wxWidgets seems to use the NSView pointer approach,
  // so let's do the same here.
  _window_handle = NativeWindowHandle::make_int((size_t) _view);

  // And tell our parent window that we're now its child.
  if (_parent_window_handle != (WindowHandle *)NULL) {
    _parent_window_handle->attach_child(_window_handle);
  }

  if (_properties.has_icon_filename()) {
    NSImage *image = load_image(_properties.get_icon_filename());
    if (image != nil) {
      // We're technically changing the application icon,
      // but this is most likely what the developer means.
      // There isn't really a "window icon" in Mac OS X.
      [NSApp setApplicationIconImage:image];
    } else {
      _properties.clear_icon_filename();
    }
  }

  if (_properties.has_cursor_filename()) {
    NSImage *image = load_image(_properties.get_cursor_filename());
    NSCursor *cursor = nil;
    //TODO: allow setting the hotspot, read it from file when loading .cur.
    if (image != nil) {
      cursor = [[NSCursor alloc] initWithImage:image hotSpot:NSMakePoint(0, 0)];
    }
    if (cursor != nil) {
      _cursor = cursor;
    } else {
      _properties.clear_cursor_filename();
    }
    // This will ensure that NSView's resetCursorRects gets
    // called, which sets the appropriate cursor rects.
    [[_view window] invalidateCursorRectsForView:_view];
  }

  // Set the properties
  if (_window != nil) {
    if (_properties.has_title()) {
      [_window setTitle: [NSString stringWithUTF8String: _properties.get_title().c_str()]];
    }

    [_window setShowsResizeIndicator: !_properties.get_fixed_size()];

    if (_properties.get_fullscreen()) {
      [_window makeKeyAndOrderFront:nil];
     } else if (_properties.get_minimized()) {
      [_window makeKeyAndOrderFront:nil];
      [_window miniaturize:nil];
    } else if (_properties.get_foreground()) {
      [_window makeKeyAndOrderFront:nil];
    } else {
      [_window orderBack:nil];
    }

    if (_properties.get_fullscreen()) {
      [_window setLevel: NSMainMenuWindowLevel + 1];
    } else {
      switch (_properties.get_z_order()) {
      case WindowProperties::Z_bottom:
        // Seems to work!
        [_window setLevel: NSNormalWindowLevel - 1];
        break;

      case WindowProperties::Z_normal:
        [_window setLevel: NSNormalWindowLevel];
        break;

      case WindowProperties::Z_top:
        [_window setLevel: NSPopUpMenuWindowLevel];
        break;
      }
    }
  }

  if (_properties.get_fullscreen()) {
    // Change the display mode.
#if __MAC_OS_X_VERSION_MAX_ALLOWED >= 1060
    CGDisplayModeRef mode;
#else
    CFDictionaryRef mode;
#endif

    mode = find_display_mode(_properties.get_x_size(),
                             _properties.get_y_size());

    if (mode == NULL) {
      cocoadisplay_cat.error()
        << "Could not find a suitable display mode!\n";
      return false;

    } else if (!do_switch_fullscreen(mode)) {
      cocoadisplay_cat.error()
        << "Failed to change display mode.\n";
      return false;
    }
  }

  // Make the context current.
  _context_needs_update = false;
  [cocoagsg->_context makeCurrentContext];
  [cocoagsg->_context setView:_view];
  [cocoagsg->_context update];

  cocoagsg->reset_if_new();

  // Release the context.
  CGLUnlockContext((CGLContextObj) [cocoagsg->_context CGLContextObj]);

  if (!cocoagsg->is_valid()) {
    close_window();
    return false;
  }

  if (!cocoagsg->get_fb_properties().verify_hardware_software
      (_fb_properties, cocoagsg->get_gl_renderer())) {
    close_window();
    return false;
  }
  _fb_properties = cocoagsg->get_fb_properties();

  // Get the initial mouse position.
  NSPoint pos = [_window mouseLocationOutsideOfEventStream];
  NSPoint loc = [_view convertPoint:pos fromView:nil];
  BOOL inside = [_view mouse:loc inRect:[_view bounds]];
  handle_mouse_moved_event(inside, loc.x, loc.y, true);

  // Enable relative mouse mode, if this was requested.
  if (_properties.has_mouse_mode() &&
      _properties.get_mouse_mode() == WindowProperties::M_relative) {
    mouse_mode_relative();
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: CocoaGraphicsWindow::close_window
//       Access: Protected, Virtual
//  Description: Closes the window right now.  Called from the window
//               thread.
////////////////////////////////////////////////////////////////////
void CocoaGraphicsWindow::
close_window() {
  if (_mouse_hidden) {
    [NSCursor unhide];
    _mouse_hidden = false;
  }

  if (_cursor != nil) {
    [_cursor release];
    _cursor = nil;
  }

  if (_gsg != (GraphicsStateGuardian *)NULL) {
    CocoaGraphicsStateGuardian *cocoagsg;
    cocoagsg = DCAST(CocoaGraphicsStateGuardian, _gsg);

    if (cocoagsg != NULL && cocoagsg->_context != nil) {
      CGLLockContext((CGLContextObj) [cocoagsg->_context CGLContextObj]);
      [cocoagsg->_context clearDrawable];
      CGLUnlockContext((CGLContextObj) [cocoagsg->_context CGLContextObj]);
    }
    _gsg.clear();
  }

  if (_window != nil) {
    [_window setReleasedWhenClosed: YES];
    [_window close];
    _window = nil;
  }

  if (_view != nil) {
    [_view release];
    _view = nil;
  }

  GraphicsWindow::close_window();
}

////////////////////////////////////////////////////////////////////
//     Function: CocoaGraphicsWindow::mouse_mode_relative
//       Access: Protected, Virtual
//  Description: Overridden from GraphicsWindow.
////////////////////////////////////////////////////////////////////
void CocoaGraphicsWindow::
mouse_mode_absolute() {
  CGAssociateMouseAndMouseCursorPosition(YES);
}

////////////////////////////////////////////////////////////////////
//     Function: CocoaGraphicsWindow::mouse_mode_relative
//       Access: Protected, Virtual
//  Description: Overridden from GraphicsWindow.
////////////////////////////////////////////////////////////////////
void CocoaGraphicsWindow::
mouse_mode_relative() {
  CGAssociateMouseAndMouseCursorPosition(NO);
}

////////////////////////////////////////////////////////////////////
//     Function: CocoaGraphicsWindow::set_properties_now
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
//               The return value is true if the properties are set,
//               false if they are ignored.  This is mainly useful for
//               derived classes to implement extensions to this
//               function.
////////////////////////////////////////////////////////////////////
void CocoaGraphicsWindow::
set_properties_now(WindowProperties &properties) {
  if (_pipe == (GraphicsPipe *)NULL) {
    // If the pipe is null, we're probably closing down.
    GraphicsWindow::set_properties_now(properties);
    return;
  }

  GraphicsWindow::set_properties_now(properties);
  if (!properties.is_any_specified()) {
    // The base class has already handled this case.
    return;
  }

  if (properties.has_fullscreen()) {
    if (_properties.get_fullscreen() != properties.get_fullscreen()) {
      if (properties.get_fullscreen()) {
        int width, height;
        if (properties.has_size()) {
          width = properties.get_x_size();
          height = properties.get_y_size();
        } else {
          width = _properties.get_x_size();
          height = _properties.get_y_size();
        }

#if __MAC_OS_X_VERSION_MAX_ALLOWED >= 1060
        CGDisplayModeRef mode;
#else
        CFDictionaryRef mode;
#endif

        mode = find_display_mode(width, height);

        if (mode == NULL) {
          cocoadisplay_cat.error()
            << "Could not find a suitable display mode with size " << width
            << "x" << height << "!\n";

        } else if (do_switch_fullscreen(mode)) {
          if (_window != nil) {
            // For some reason, setting the style mask
            // makes it give up its first-responder status.
            if ([_window respondsToSelector:@selector(setStyleMask:)]) {
              [_window setStyleMask:NSBorderlessWindowMask];
            }
            [_window makeFirstResponder:_view];
            [_window setLevel:NSMainMenuWindowLevel+1];
            [_window makeKeyAndOrderFront:nil];
          }

          // We've already set the size property this way; clear it.
          properties.clear_size();
          _properties.set_size(width, height);
          properties.clear_origin();
          _properties.set_origin(0, 0);
          properties.clear_fullscreen();
          _properties.set_fullscreen(true);

        } else {
          cocoadisplay_cat.error()
            << "Failed to change display mode.\n";
        }

      } else {
        do_switch_fullscreen(NULL);
        _properties.set_fullscreen(false);

        // Force properties to be reset to their actual values
        properties.set_undecorated(_properties.get_undecorated());
        properties.set_z_order(_properties.get_z_order());
        properties.clear_fullscreen();
      }
    }
    _context_needs_update = true;
  }

  if (properties.has_minimized() && !_properties.get_fullscreen() && _window != nil) {
    _properties.set_minimized(properties.get_minimized());
    if (properties.get_minimized()) {
      [_window miniaturize:nil];
    } else {
      [_window deminiaturize:nil];
    }
    properties.clear_minimized();
  }

  if (properties.has_size()) {
    int width = properties.get_x_size();
    int height = properties.get_y_size();

    if (!_properties.get_fullscreen()) {
      _properties.set_size(width, height);
      if (_window != nil) {
        [_window setContentSize:NSMakeSize(width, height)];
      }
      [_view setFrameSize:NSMakeSize(width, height)];

      cocoadisplay_cat.debug()
        << "Setting size to " << width << ", " << height << "\n";

      _context_needs_update = true;
      properties.clear_size();

    } else {
#if __MAC_OS_X_VERSION_MAX_ALLOWED >= 1060
      CGDisplayModeRef mode = find_display_mode(width, height);
#else
      CFDictionaryRef mode = find_display_mode(width, height);
#endif

      if (mode == NULL) {
        cocoadisplay_cat.error()
          << "Could not find a suitable display mode with size " << width
          << "x" << height << "!\n";

      } else if (do_switch_fullscreen(mode)) {
        // Yay!  Our resolution has changed.
        _properties.set_size(width, height);
        properties.clear_size();

      } else {
        cocoadisplay_cat.error()
          << "Failed to change display mode.\n";
      }
    }
  }

  if (properties.has_origin() && !_properties.get_fullscreen()) {
    int x = properties.get_x_origin();
    int y = properties.get_y_origin();

    // Get the frame for the screen
    NSRect frame;
    NSRect container;
    if (_window != nil) {
      frame = [_window contentRectForFrameRect:[_window frame]];
      NSScreen *screen = [_window screen];
      nassertv(screen != nil);
      container = [screen frame];
    } else {
      frame = [_view frame];
      container = [[_view superview] frame];
    }

    if (x < 0) {
      x = floor(container.size.width / 2 - frame.size.width / 2);
    }
    if (y < 0) {
      y = floor(container.size.height / 2 - frame.size.height / 2);
    }
    _properties.set_origin(x, y);

    if (!_properties.get_fullscreen()) {
      // Remember, Mac OS X coordinates are flipped in the vertical axis.
      frame.origin.x = x;
      frame.origin.y = container.size.height - y - frame.size.height;

      cocoadisplay_cat.debug()
        << "Setting window content origin to " << frame.origin.x << ", " << frame.origin.y << "\n";

      if (_window != nil) {
        [_window setFrame:[_window frameRectForContentRect:frame] display:NO];
      } else {
        [_view setFrame:frame];
      }
    }
    properties.clear_origin();
  }

  if (properties.has_title() && _window != nil) {
    _properties.set_title(properties.get_title());
    [_window setTitle:[NSString stringWithUTF8String:properties.get_title().c_str()]];
    properties.clear_title();
  }

  if (properties.has_fixed_size() && _window != nil) {
    _properties.set_fixed_size(properties.get_fixed_size());
    [_window setShowsResizeIndicator:!properties.get_fixed_size()];
    
    if (!_properties.get_fullscreen()) {
      // If our window is decorated, change the style mask
      // to show or hide the resize button appropriately.
      // However, if we're specifying the 'undecorated' property also,
      // then we'll be setting the style mask about 25 LOC further down,
      // so we won't need to bother setting it here.
      if (!properties.has_undecorated() && !_properties.get_undecorated() &&
          [_window respondsToSelector:@selector(setStyleMask:)]) {
        if (properties.get_fixed_size()) {
          [_window setStyleMask:NSTitledWindowMask | NSClosableWindowMask |
                                NSMiniaturizableWindowMask ];
        } else {
          [_window setStyleMask:NSTitledWindowMask | NSClosableWindowMask |
                                NSMiniaturizableWindowMask | NSResizableWindowMask ];
        }
        [_window makeFirstResponder:_view];
      }
    }

    properties.clear_fixed_size();
  }

  if (properties.has_undecorated() && _window != nil && [_window respondsToSelector:@selector(setStyleMask:)]) {
    _properties.set_undecorated(properties.get_undecorated());

    if (!_properties.get_fullscreen()) {
      if (properties.get_undecorated()) {
        [_window setStyleMask: NSBorderlessWindowMask];
      } else if (_properties.get_fixed_size()) {
        // Fixed size windows should not show the resize button.
        [_window setStyleMask: NSTitledWindowMask | NSClosableWindowMask |
                               NSMiniaturizableWindowMask ];
      } else {
        [_window setStyleMask: NSTitledWindowMask | NSClosableWindowMask |
                               NSMiniaturizableWindowMask | NSResizableWindowMask ];
      }
      [_window makeFirstResponder:_view];
    }

    properties.clear_undecorated();
  }

  if (properties.has_foreground() && !_properties.get_fullscreen() && _window != nil) {
    _properties.set_foreground(properties.get_foreground());
    if (!_properties.get_minimized()) {
      if (properties.get_foreground()) {
        [_window makeKeyAndOrderFront: nil];
      } else {
        [_window orderBack: nil];
      }
    }
    properties.clear_foreground();
  }

  //TODO: support raw mice.

  if (properties.has_cursor_hidden()) {
    if (properties.get_cursor_hidden() != _properties.get_cursor_hidden()) {
      if (properties.get_cursor_hidden() && _input_devices[0].get_pointer().get_in_window()) {
        [NSCursor hide];
        _mouse_hidden = true;
      } else if (_mouse_hidden) {
        [NSCursor unhide];
        _mouse_hidden = false;
      }
      _properties.set_cursor_hidden(properties.get_cursor_hidden());
    }
    properties.clear_cursor_hidden();
  }

  if (properties.has_icon_filename()) {
    Filename icon_filename = properties.get_icon_filename();
    NSImage *image = load_image(icon_filename);

    if (image != nil || icon_filename.empty()) {
      // We're technically changing the application icon,
      // but this is most likely what the developer means.
      // There isn't really a "window icon" in Mac OS X.
      [NSApp setApplicationIconImage:image];
      _properties.set_icon_filename(icon_filename);
      properties.clear_icon_filename();
    }
  }

  if (properties.has_cursor_filename()) {
    Filename cursor_filename = properties.get_cursor_filename();

    if (cursor_filename.empty()) {
      // Release the existing cursor.
      if (_cursor != nil) {
        [_cursor release];
        _cursor = nil;
      }
      properties.set_cursor_filename(cursor_filename);
      properties.clear_cursor_filename();
    } else {
      NSImage *image = load_image(cursor_filename);
      if (image != nil) {
        NSCursor *cursor;
        cursor = [[NSCursor alloc] initWithImage:image hotSpot:NSMakePoint(0, 0)];
        if (cursor != nil) {
          // Replace the existing cursor.
          if (_cursor != nil) {
            [_cursor release];
          }
          _cursor = cursor;
          _properties.set_cursor_filename(cursor_filename);
          properties.clear_cursor_filename();
        }
      }
    }
    // This will ensure that NSView's resetCursorRects gets
    // called, which sets the appropriate cursor rects.
    [[_view window] invalidateCursorRectsForView:_view];
  }

  if (properties.has_z_order() && _window != nil) {
    _properties.set_z_order(properties.get_z_order());
    
    if (!_properties.get_fullscreen()) {
      switch (properties.get_z_order()) {
      case WindowProperties::Z_bottom:
        [_window setLevel: NSNormalWindowLevel - 1];
        break;

      case WindowProperties::Z_normal:
        [_window setLevel: NSNormalWindowLevel];
        break;

      case WindowProperties::Z_top:
        [_window setLevel: NSPopUpMenuWindowLevel];
        break;
      }
    }
    properties.clear_z_order();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CocoaGraphicsWindow::find_display_mode
//       Access: Protected
//  Description: Returns an appropriate CGDisplayModeRef for the
//               given width and height, or NULL if none was found.
////////////////////////////////////////////////////////////////////
#if __MAC_OS_X_VERSION_MAX_ALLOWED >= 1060
CGDisplayModeRef CocoaGraphicsWindow::
find_display_mode(int width, int height) {
  CFArrayRef modes = CGDisplayCopyAllDisplayModes(_display, NULL);
  size_t num_modes = CFArrayGetCount(modes);
  CGDisplayModeRef mode;

  // Get the current refresh rate and pixel encoding.
  CFStringRef current_pixel_encoding;
  int refresh_rate;
  mode = CGDisplayCopyDisplayMode(_display);
  
  // First check if the current mode is adequate.
  if (CGDisplayModeGetWidth(mode) == width &&
      CGDisplayModeGetHeight(mode) == height) {
    return mode;
  }
  
  current_pixel_encoding = CGDisplayModeCopyPixelEncoding(mode);
  refresh_rate = CGDisplayModeGetRefreshRate(mode);
  CGDisplayModeRelease(mode);

  for (size_t i = 0; i < num_modes; ++i) {
    mode = (CGDisplayModeRef) CFArrayGetValueAtIndex(modes, i);

    CFStringRef pixel_encoding = CGDisplayModeCopyPixelEncoding(mode);

    if (CGDisplayModeGetWidth(mode) == width &&
        CGDisplayModeGetHeight(mode) == height &&
        CGDisplayModeGetRefreshRate(mode) == refresh_rate &&
        CFStringCompare(pixel_encoding, current_pixel_encoding, 0) == kCFCompareEqualTo) {

      CFRetain(mode);
      CFRelease(pixel_encoding);
      CFRelease(current_pixel_encoding);
      CFRelease(modes);
      return mode;
    }
  }

  CFRelease(current_pixel_encoding);
  CFRelease(modes);
  return NULL;
}
#else // Version for pre-10.6.
CFDictionaryRef CocoaGraphicsWindow::
find_display_mode(int width, int height) {
  // Get the current mode and extract its properties.
  CFDictionaryRef current_mode = CGDisplayCurrentMode(_display);
  int current_width, current_height, current_bpp, current_refresh_rate;

  CFNumberGetValue((CFNumberRef) CFDictionaryGetValue(current_mode, kCGDisplayWidth),
    kCFNumberIntType, &current_width);

  CFNumberGetValue((CFNumberRef) CFDictionaryGetValue(current_mode, kCGDisplayHeight),
    kCFNumberIntType, &current_height);

  CFNumberGetValue((CFNumberRef) CFDictionaryGetValue(current_mode, kCGDisplayBitsPerPixel),
    kCFNumberIntType, &current_bpp);

  CFNumberGetValue((CFNumberRef) CFDictionaryGetValue(current_mode, kCGDisplayRefreshRate),
    kCFNumberIntType, &current_refresh_rate);

  // Check if it is suitable and if so, return it.
  if (current_width == width && current_height == height) {
    return current_mode;
  }

  // Iterate over the modes to find a suitable one.
  CFArrayRef modes = CGDisplayAvailableModes(_display);
  size_t num_modes = CFArrayGetCount(modes);
  int mode_width, mode_height, mode_bpp, mode_refresh_rate;

  for (size_t i = 0; i < num_modes; ++i) {
    CFDictionaryRef mode = (CFDictionaryRef) CFArrayGetValueAtIndex(modes, i);

    CFNumberGetValue((CFNumberRef) CFDictionaryGetValue(mode, kCGDisplayWidth),
      kCFNumberIntType, &mode_width);

    CFNumberGetValue((CFNumberRef) CFDictionaryGetValue(mode, kCGDisplayHeight),
      kCFNumberIntType, &mode_height);

    CFNumberGetValue((CFNumberRef) CFDictionaryGetValue(mode, kCGDisplayBitsPerPixel),
      kCFNumberIntType, &mode_bpp);

    CFNumberGetValue((CFNumberRef) CFDictionaryGetValue(mode, kCGDisplayRefreshRate),
      kCFNumberIntType, &mode_refresh_rate);

    if (mode_width == width && mode_height == height &&
        mode_refresh_rate == current_refresh_rate &&
        mode_bpp == current_bpp) {
      return mode;
    }
  }

  return NULL;
}
#endif

////////////////////////////////////////////////////////////////////
//     Function: CocoaGraphicsWindow::do_switch_fullscreen
//       Access: Protected
//  Description: Switches to the indicated fullscreen mode, or
//               back to windowed if NULL was given.  Returns true
//               on success, false on failure.
////////////////////////////////////////////////////////////////////
#if __MAC_OS_X_VERSION_MAX_ALLOWED >= 1060
bool CocoaGraphicsWindow::
do_switch_fullscreen(CGDisplayModeRef mode) {
#else
bool CocoaGraphicsWindow::
do_switch_fullscreen(CFDictionaryRef mode) {
#endif
  if (mode == NULL) {
    if (_windowed_mode == NULL) {
      // Already windowed.
      return true;
    }

    // Switch back to the mode we were in when we were still windowed.
#if __MAC_OS_X_VERSION_MAX_ALLOWED >= 1060
    CGDisplaySetDisplayMode(_display, _windowed_mode, NULL);
    CGDisplayModeRelease(_windowed_mode);
#else
    CGDisplaySwitchToMode(_display, _windowed_mode);
#endif
    CGDisplayRelease(_display);
    _windowed_mode = NULL;
    _context_needs_update = true;

  } else {
    if (_windowed_mode != NULL && _fullscreen_mode == mode) {
      // Already fullscreen in that size.
      return true;
    }

    // Store the existing mode under _windowed_mode.
#if __MAC_OS_X_VERSION_MAX_ALLOWED >= 1060
    _windowed_mode = CGDisplayCopyDisplayMode(_display);
#else
    _windowed_mode = CGDisplayCurrentMode(_display);
#endif
    _fullscreen_mode = mode;
    _context_needs_update = true;

    CGError err;
#if __MAC_OS_X_VERSION_MAX_ALLOWED >= 1060
    err = CGDisplaySetDisplayMode(_display, _fullscreen_mode, NULL);
#else
    err = CGDisplaySwitchToMode(_display, _fullscreen_mode);
#endif

    if (err != kCGErrorSuccess) {
      return false;
    }

    CGDisplayCapture(_display);

    NSRect frame = [[[_view window] screen] frame];
    if (cocoadisplay_cat.is_debug()) {
      NSString *str = NSStringFromRect(frame);
      cocoadisplay_cat.debug()
        << "Switched to fullscreen, screen rect is now " << [str UTF8String] << "\n";
    }

    if (_window != nil) {
      [_window setFrame:frame display:YES];
      [_view setFrame:NSMakeRect(0, 0, frame.size.width, frame.size.height)];
      [_window update];
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: CocoaGraphicsWindow::load_image
//       Access: Private
//  Description: Loads the indicated filename and returns an NSImage
//               pointer, or NULL on failure.
//               Must be called from the window thread.
////////////////////////////////////////////////////////////////////
NSImage *CocoaGraphicsWindow::
load_image(const Filename &filename) {
  if (filename.empty()) {
    return nil;
  }

  // Note: perhaps eventually we will need to create
  // an NSImageRep implementation, but for now, Apple
  // seems to support the major image formats.

  // Resolve the filename on the model path.
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  Filename resolved (filename);
  if (!vfs->resolve_filename(resolved, get_model_path())) {
    // The filename doesn't exist.
    cocoadisplay_cat.warning()
      << "Could not find filename " << filename << "\n";
    return 0;
  }

  // Look in our index.
  NSImage *image = nil;
  IconImages::const_iterator it = _images.find(resolved);
  if (it != _images.end()) {
    // Found it.
    return (*it).second;
  }

  cocoadisplay_cat.info()
    << "Loading NSImage from file " << resolved << "\n";

  PT(VirtualFile) vfile = vfs->get_file(filename);
  if (vfile == NULL) {
    return nil;
  }
  istream *str = vfile->open_read_file(true);
  if (str == NULL) {
    cocoadisplay_cat.error()
      << "Could not open file " << filename << " for reading\n";
    return nil;
  }

  size_t size = vfile->get_file_size(str);
  char* buffer = (char*) malloc(size);
  str->read(buffer, size);
  vfile->close_read_file(str);

  NSData *data = [NSData dataWithBytesNoCopy:buffer length:size];
  if (data == nil) {
    return nil;
  }

  image = [[NSImage alloc] initWithData:data];
  [data release];
  if (image == nil) {
    cocoadisplay_cat.error()
      << "Could not load image from file " << filename << "\n";
    return nil;
  }

  _images[resolved] = image;
  return image;
}

////////////////////////////////////////////////////////////////////
//     Function: CocoaGraphicsWindow::handle_move_event
//       Access: Public
//  Description: Called by CocoaPandaView or the window delegate
//               when the frame rect changes.
////////////////////////////////////////////////////////////////////
void CocoaGraphicsWindow::
handle_move_event() {
  // Remember, Mac OS X uses flipped coordinates
  NSRect frame;
  int x, y;
  if (_window == nil) {
    frame = [_view frame];
    x = frame.origin.x;
    y = [[_view superview] bounds].size.height - frame.origin.y - frame.size.height;
  } else {
    frame = [_window contentRectForFrameRect:[_window frame]];
    x = frame.origin.x;
    y = [[_window screen] frame].size.height - frame.origin.y - frame.size.height;
  }

  if (x != _properties.get_x_origin() ||
      y != _properties.get_y_origin()) {

    WindowProperties properties;
    properties.set_origin(x, y);

    if (cocoadisplay_cat.is_spam()) {
      cocoadisplay_cat.spam()
        << "Window changed origin to (" << x << ", " << y << ")\n";
    }
    system_changed_properties(properties);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CocoaGraphicsWindow::handle_resize_event
//       Access: Public
//  Description: Called by CocoaPandaView or the window delegate
//               when the frame rect changes.
////////////////////////////////////////////////////////////////////
void CocoaGraphicsWindow::
handle_resize_event() {
  if (_window != nil) {
    NSRect contentRect = [_window contentRectForFrameRect:[_window frame]];
    [_view setFrameSize:contentRect.size];
  }

  NSRect frame = [_view convertRect:[_view bounds] toView:nil];

  if (frame.size.width != _properties.get_x_size() ||
      frame.size.height != _properties.get_y_size()) {

    WindowProperties properties;
    properties.set_size(frame.size.width, frame.size.height);

    if (cocoadisplay_cat.is_spam()) {
      cocoadisplay_cat.spam()
        << "Window changed size to (" << frame.size.width
       << ", " << frame.size.height << ")\n";
    }
    system_changed_properties(properties);
  }

  _context_needs_update = true;
}

////////////////////////////////////////////////////////////////////
//     Function: CocoaGraphicsWindow::handle_minimize_event
//       Access: Public
//  Description: Called by the window delegate when the window is
//               miniaturized or deminiaturized.
////////////////////////////////////////////////////////////////////
void CocoaGraphicsWindow::
handle_minimize_event(bool minimized) {
  if (minimized == _properties.get_minimized()) {
    return;
  }

  if (cocoadisplay_cat.is_debug()) {
    if (minimized) {
      cocoadisplay_cat.debug() << "Window was miniaturized\n";
    } else {
      cocoadisplay_cat.debug() << "Window was deminiaturized\n";
    }
  }

  WindowProperties properties;
  properties.set_minimized(minimized);
  system_changed_properties(properties);
}

////////////////////////////////////////////////////////////////////
//     Function: CocoaGraphicsWindow::handle_foreground_event
//       Access: Public
//  Description: Called by the window delegate when the window has
//               become the key window or resigned that status.
////////////////////////////////////////////////////////////////////
void CocoaGraphicsWindow::
handle_foreground_event(bool foreground) {
  if (cocoadisplay_cat.is_debug()) {
    if (foreground) {
      cocoadisplay_cat.debug() << "Window became key\n";
    } else {
      cocoadisplay_cat.debug() << "Window resigned key\n";
    }
  }

  WindowProperties properties;
  properties.set_foreground(foreground);
  system_changed_properties(properties);

  if (foreground && _properties.get_mouse_mode() != WindowProperties::M_relative) {
    // The mouse position may have changed during
    // the time that we were not the key window.
    NSPoint pos = [_window mouseLocationOutsideOfEventStream];

    NSPoint loc = [_view convertPoint:pos fromView:nil];
    BOOL inside = [_view mouse:loc inRect:[_view bounds]];

    handle_mouse_moved_event(inside, loc.x, loc.y, true);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CocoaGraphicsWindow::handle_close_request
//       Access: Public
//  Description: Called by the window delegate when the user
//               requests to close the window.  This may not always
//               be called, which is why there is also a
//               handle_close_event.
//               Returns false if the user indicated that he wants
//               to handle the close request himself, true if the
//               operating system should continue closing the window.
////////////////////////////////////////////////////////////////////
bool CocoaGraphicsWindow::
handle_close_request() {
  string close_request_event = get_close_request_event();
  if (!close_request_event.empty()) {
    // In this case, the app has indicated a desire to intercept
    // the request and process it directly.
    throw_event(close_request_event);

    cocoadisplay_cat.debug()
      << "Window requested close.  Rejecting, throwing event "
      << close_request_event << " instead\n";

    // Prevent the operating system from closing the window.
    return false;
  }

  cocoadisplay_cat.debug()
    << "Window requested close, accepting\n";

  // Let the operating system close the window normally.
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: CocoaGraphicsWindow::handle_close_event
//       Access: Public
//  Description: Called by the window delegate when the window closes.
////////////////////////////////////////////////////////////////////
void CocoaGraphicsWindow::
handle_close_event() {
  cocoadisplay_cat.debug() << "Window is about to close\n";

  _window = nil;

  // Get rid of the GSG
  if (_gsg != (GraphicsStateGuardian *)NULL) {
    CocoaGraphicsStateGuardian *cocoagsg;
    cocoagsg = DCAST(CocoaGraphicsStateGuardian, _gsg);

    if (cocoagsg != NULL && cocoagsg->_context != nil) {
      CGLLockContext((CGLContextObj) [cocoagsg->_context CGLContextObj]);
      [cocoagsg->_context clearDrawable];
      CGLUnlockContext((CGLContextObj) [cocoagsg->_context CGLContextObj]);
    }
    _gsg.clear();
  }

  // Dump the view, too
  if (_view != nil) {
    [_view release];
    _view = nil;
  }

  // Unhide the mouse cursor
  if (_mouse_hidden) {
    [NSCursor unhide];
    _mouse_hidden = false;
  }

  // Release the cursor.
  if (_cursor != nil) {
    [_cursor release];
    _cursor = nil;
  }

  WindowProperties properties;
  properties.set_open(false);
  properties.set_cursor_hidden(false);
  system_changed_properties(properties);

  GraphicsWindow::close_window();
}

////////////////////////////////////////////////////////////////////
//     Function: CocoaGraphicsWindow::handle_key_event
//       Access: Public
//  Description: This method processes the NSEvent of type NSKeyUp,
//               NSKeyDown or NSFlagsChanged and passes the
//               information on to Panda.
//               Should only be called by CocoaPandaView.
////////////////////////////////////////////////////////////////////
void CocoaGraphicsWindow::
handle_key_event(NSEvent *event) {
  NSUInteger modifierFlags = [event modifierFlags];

  //NB.  This is actually a on-off toggle, not up-down.
  // Should we instead rapidly fire two successive up-down events?
  handle_modifier(modifierFlags, NSAlphaShiftKeyMask, KeyboardButton::caps_lock());

  // Check if any of the modifier keys have changed.
  handle_modifier(modifierFlags, NSShiftKeyMask, KeyboardButton::shift());
  handle_modifier(modifierFlags, NSControlKeyMask, KeyboardButton::control());
  handle_modifier(modifierFlags, NSAlternateKeyMask, KeyboardButton::alt());
  handle_modifier(modifierFlags, NSCommandKeyMask, KeyboardButton::meta());

  // These are not documented, but they seem to be a reliable indicator
  // of the status of the left/right modifier keys.
  handle_modifier(modifierFlags, 0x0002, KeyboardButton::lshift());
  handle_modifier(modifierFlags, 0x0004, KeyboardButton::rshift());
  handle_modifier(modifierFlags, 0x0001, KeyboardButton::lcontrol());
  handle_modifier(modifierFlags, 0x2000, KeyboardButton::rcontrol());
  handle_modifier(modifierFlags, 0x0020, KeyboardButton::lalt());
  handle_modifier(modifierFlags, 0x0040, KeyboardButton::ralt());
  handle_modifier(modifierFlags, 0x0008, KeyboardButton::lmeta());
  handle_modifier(modifierFlags, 0x0010, KeyboardButton::rmeta());

  _modifier_keys = modifierFlags;

  // Get the raw button and send it.
  ButtonHandle raw_button = map_raw_key([event keyCode]);
  if (raw_button != ButtonHandle::none()) {
    // This is not perfect.  Eventually, this whole thing should
    // probably be replaced with something that uses IOKit or so.
    // In particular, the flaws are:
    // - OS eats unmodified F11, F12, scroll lock, pause
    // - no up events for caps lock
    // - no robust way to distinguish up/down for modkeys
    if ([event type] == NSKeyUp) {
      _input_devices[0].raw_button_up(raw_button);

    } else if ([event type] == NSFlagsChanged) {
      bool down = false;
      if (raw_button == KeyboardButton::lshift()) {
        down = (modifierFlags & 0x0002);
      } else if (raw_button == KeyboardButton::rshift()) {
        down = (modifierFlags & 0x0004);
      } else if (raw_button == KeyboardButton::lcontrol()) {
        down = (modifierFlags & 0x0001);
      } else if (raw_button == KeyboardButton::rcontrol()) {
        down = (modifierFlags & 0x2000);
      } else if (raw_button == KeyboardButton::lalt()) {
        down = (modifierFlags & 0x0020);
      } else if (raw_button == KeyboardButton::ralt()) {
        down = (modifierFlags & 0x0040);
      } else if (raw_button == KeyboardButton::lmeta()) {
        down = (modifierFlags & 0x0008);
      } else if (raw_button == KeyboardButton::rmeta()) {
        down = (modifierFlags & 0x0010);
      } else if (raw_button == KeyboardButton::caps_lock()) {
        // Emulate down-up, annoying hack!
        _input_devices[0].raw_button_down(raw_button);
      }
      if (down) {
        _input_devices[0].raw_button_down(raw_button);
      } else {
        _input_devices[0].raw_button_up(raw_button);
      }
    } else if (![event isARepeat]) {
      _input_devices[0].raw_button_down(raw_button);
    }
  }

  // FlagsChanged events only carry modifier key information.
  if ([event type] == NSFlagsChanged) {
    return;
  }

  NSString *str = [event charactersIgnoringModifiers];
  if (str == nil || [str length] == 0) {
    return;
  }
  nassertv([str length] == 1);
  unichar c = [str characterAtIndex: 0];

  ButtonHandle button = map_key(c);

  if (c < 0xF700 || c >= 0xF900) {
    // If a down event and not a special function key,
    // process it as keystroke as well.
    if ([event type] == NSKeyDown) {
      NSString *origstr = [event characters];
      c = [str characterAtIndex: 0];
      _input_devices[0].keystroke(c);
    }
  }

  if (button == ButtonHandle::none()) {
    // That done, continue trying to find out the button handle.
    if ([str canBeConvertedToEncoding: NSASCIIStringEncoding]) {
      // Nhm, ascii character perhaps?
      str = [str lowercaseString];
      button = KeyboardButton::ascii_key([str cStringUsingEncoding: NSASCIIStringEncoding]);
    }
  }

  if (button == ButtonHandle::none()) {
    cocoadisplay_cat.warning()
      << "Unhandled keypress, character " << (int) c << ", keyCode " << [event keyCode]
      << ", type " << [event type] << ", flags " << [event modifierFlags] << "\n";
    return;
  }

  if (cocoadisplay_cat.is_spam()) {
    cocoadisplay_cat.spam()
      << "Handled keypress, character " << (int) c << ", keyCode " << [event keyCode]
      << ", type " << [event type] << ", flags " << [event modifierFlags] << "\n";
  }

  // Let's get it off our chest.
  if ([event type] == NSKeyUp) {
    _input_devices[0].button_up(button);
  } else {
    _input_devices[0].button_down(button);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CocoaGraphicsWindow::handle_modifier
//       Access: Private
//  Description: Called by handle_key_event to read the state of
//               a modifier key.
////////////////////////////////////////////////////////////////////
void CocoaGraphicsWindow::
handle_modifier(NSUInteger modifierFlags, NSUInteger mask, ButtonHandle button) {
  if ((modifierFlags ^ _modifier_keys) & mask) {
    if (modifierFlags & mask) {
      _input_devices[0].button_down(button);
    } else {
      _input_devices[0].button_up(button);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CocoaGraphicsWindow::handle_mouse_button_event
//       Access: Public
//  Description: This method processes the NSEvents related to
//               mouse button presses.
//               Should only be called by CocoaPandaView.
////////////////////////////////////////////////////////////////////
void CocoaGraphicsWindow::
handle_mouse_button_event(int button, bool down) {
  if (down) {
    _input_devices[0].button_down(MouseButton::button(button));

#ifndef NDEBUG
    cocoadisplay_cat.spam()
      << "Mouse button " << button << " down\n";
#endif
  } else {
    _input_devices[0].button_up(MouseButton::button(button));

#ifndef NDEBUG
    cocoadisplay_cat.spam()
      << "Mouse button " << button << " up\n";
#endif
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CocoaGraphicsWindow::handle_mouse_moved_event
//       Access: Public
//  Description: This method processes the NSEvents of the
//               mouseMoved and mouseDragged types.
//               Should only be called by CocoaPandaView.
////////////////////////////////////////////////////////////////////
void CocoaGraphicsWindow::
handle_mouse_moved_event(bool in_window, double x, double y, bool absolute) {
  if (absolute) {
    if (cocoadisplay_cat.is_spam()) {
      if (in_window != _input_devices[0].get_pointer().get_in_window()) {
        if (in_window) {
          cocoadisplay_cat.spam() << "Mouse pointer entered window\n";
        } else {
          cocoadisplay_cat.spam() << "Mouse pointer exited window\n";
        }
      }
    }

    // Strangely enough, in Cocoa, mouse Y coordinates are 1-based.
    _input_devices[0].set_pointer(in_window, x, y - 1,
      ClockObject::get_global_clock()->get_frame_time());

  } else {
    // We received deltas, so add it to the current mouse position.
    MouseData md = _input_devices[0].get_pointer();
    _input_devices[0].set_pointer_in_window(md.get_x() + x, md.get_y() + y);
  }

  if (in_window != _mouse_hidden && _properties.get_cursor_hidden()) {
    // Hide the cursor if the mouse enters the window,
    // and unhide it when the mouse leaves the window.
    if (in_window) {
      [NSCursor hide];
    } else {
      [NSCursor unhide];
    }
    _mouse_hidden = in_window;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CocoaGraphicsWindow::handle_wheel_event
//       Access: Public
//  Description: Called by CocoaPandaView to inform that the scroll
//               wheel has been used.
////////////////////////////////////////////////////////////////////
void CocoaGraphicsWindow::
handle_wheel_event(double x, double y) {
  cocoadisplay_cat.spam()
    << "Wheel delta " << x << ", " << y << "\n";

  if (y > 0.0) {
    _input_devices[0].button_down(MouseButton::wheel_up());
    _input_devices[0].button_up(MouseButton::wheel_up());
  } else if (y < 0.0) {
    _input_devices[0].button_down(MouseButton::wheel_down());
    _input_devices[0].button_up(MouseButton::wheel_down());
  }

  //TODO: check if this is correct, I don't own a MacBook
  if (x > 0.0) {
    _input_devices[0].button_down(MouseButton::wheel_right());
    _input_devices[0].button_up(MouseButton::wheel_right());
  } else if (x < 0.0) {
    _input_devices[0].button_down(MouseButton::wheel_left());
    _input_devices[0].button_up(MouseButton::wheel_left());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CocoaGraphicsWindow::get_keyboard_map
//       Access: Published, Virtual
//  Description: Returns a ButtonMap containing the association
//               between raw buttons and virtual buttons.
////////////////////////////////////////////////////////////////////
ButtonMap *CocoaGraphicsWindow::
get_keyboard_map() const {
  TISInputSourceRef input_source;
  CFDataRef layout_data;
  const UCKeyboardLayout *layout;

  // Get the current keyboard layout data.
  input_source = TISCopyCurrentKeyboardInputSource();
  layout_data = (CFDataRef) TISGetInputSourceProperty(input_source, kTISPropertyUnicodeKeyLayoutData);
  layout = (const UCKeyboardLayout *)CFDataGetBytePtr(layout_data);

  ButtonMap *map = new ButtonMap;

  UniChar chars[4];
  UniCharCount num_chars;

  // Iterate through the known scancode range and see what
  // every scan code is mapped to.
  for (int k = 0; k <= 0x7E; ++k) {
    ButtonHandle raw_button = map_raw_key(k);
    if (raw_button == ButtonHandle::none()) {
      continue;
    }

    UInt32 dead_keys = 0;
    if (UCKeyTranslate(layout, k, kUCKeyActionDisplay, 0, LMGetKbdType(),
                       kUCKeyTranslateNoDeadKeysMask, &dead_keys, 4,
                       &num_chars, chars) == noErr) {
      if (num_chars > 0 && chars[0] != 0x10) {
        ButtonHandle button = ButtonHandle::none();

        if (chars[0] > 0 && chars[0] <= 0x7f) {
          button = KeyboardButton::ascii_key(chars[0]);
        }
        if (button == ButtonHandle::none()) {
          button = map_key(chars[0]);
        }
        if (button != ButtonHandle::none()) {
          map->map_button(raw_button, button);
        }
      } else {
        // A special function key or modifier key, which isn't remapped by the OS.
        map->map_button(raw_button, raw_button);
      }
    }
  }

  CFRelease(input_source);
  return map;
}

////////////////////////////////////////////////////////////////////
//     Function: CocoaGraphicsWindow::map_key
//       Access: Private
//  Description: Maps a unicode key character to a ButtonHandle.
////////////////////////////////////////////////////////////////////
ButtonHandle CocoaGraphicsWindow::
map_key(unsigned short c) const {
  switch (c) {
  case NSEnterCharacter:
    return KeyboardButton::enter();
  case NSBackspaceCharacter:
  case NSDeleteCharacter:
    // NSDeleteCharacter fires when I press backspace.
    return KeyboardButton::backspace();
  case NSTabCharacter:
  case NSBackTabCharacter:
    // BackTabCharacter is sent when shift-tab is used.
    return KeyboardButton::tab();

  case 0x10:
    // No idea where this constant comes from, but it
    // is sent whenever the menu key is pressed.
    return KeyboardButton::menu();

  case 0x1e:
  case NSUpArrowFunctionKey:
    return KeyboardButton::up();
  case 0x1f:
  case NSDownArrowFunctionKey:
    return KeyboardButton::down();
  case 0x1c:
  case NSLeftArrowFunctionKey:
    return KeyboardButton::left();
  case 0x1d:
  case NSRightArrowFunctionKey:
    return KeyboardButton::right();
  case NSF1FunctionKey:
    return KeyboardButton::f1();
  case NSF2FunctionKey:
    return KeyboardButton::f2();
  case NSF3FunctionKey:
    return KeyboardButton::f3();
  case NSF4FunctionKey:
    return KeyboardButton::f4();
  case NSF5FunctionKey:
    return KeyboardButton::f5();
  case NSF6FunctionKey:
    return KeyboardButton::f6();
  case NSF7FunctionKey:
    return KeyboardButton::f7();
  case NSF8FunctionKey:
    return KeyboardButton::f8();
  case NSF9FunctionKey:
    return KeyboardButton::f9();
  case NSF10FunctionKey:
    return KeyboardButton::f10();
  case NSF11FunctionKey:
    return KeyboardButton::f11();
  case NSF12FunctionKey:
    return KeyboardButton::f12();
  case NSF13FunctionKey:
    return KeyboardButton::f13();
  case NSF14FunctionKey:
    return KeyboardButton::f14();
  case NSF15FunctionKey:
    return KeyboardButton::f15();
  case NSF16FunctionKey:
    return KeyboardButton::f16();
  case NSF17FunctionKey:
  case NSF18FunctionKey:
  case NSF19FunctionKey:
  case NSF20FunctionKey:
  case NSF21FunctionKey:
  case NSF22FunctionKey:
  case NSF23FunctionKey:
  case NSF24FunctionKey:
  case NSF25FunctionKey:
  case NSF26FunctionKey:
  case NSF27FunctionKey:
  case NSF28FunctionKey:
  case NSF29FunctionKey:
  case NSF30FunctionKey:
  case NSF31FunctionKey:
  case NSF32FunctionKey:
  case NSF33FunctionKey:
  case NSF34FunctionKey:
  case NSF35FunctionKey:
    break;
  case NSInsertFunctionKey:
    return KeyboardButton::insert();
  case NSDeleteFunctionKey:
    return KeyboardButton::del();
  case 0x01:
  case NSHomeFunctionKey:
    return KeyboardButton::home();
  case NSBeginFunctionKey:
    break;
  case 0x04:
  case NSEndFunctionKey:
    return KeyboardButton::end();
  case 0x0b:
  case NSPageUpFunctionKey:
    return KeyboardButton::page_up();
  case 0x0c:
  case NSPageDownFunctionKey:
    return KeyboardButton::page_down();
  case NSPrintScreenFunctionKey:
    return KeyboardButton::print_screen();
  case NSScrollLockFunctionKey:
    return KeyboardButton::scroll_lock();
  case NSPauseFunctionKey:
    return KeyboardButton::pause();
  case NSSysReqFunctionKey:
  case NSBreakFunctionKey:
  case NSResetFunctionKey:
  case NSStopFunctionKey:
  case NSMenuFunctionKey:
  case NSUserFunctionKey:
  case NSSystemFunctionKey:
  case NSPrintFunctionKey:
    break;
  case NSClearLineFunctionKey:
    return KeyboardButton::num_lock();
  case NSClearDisplayFunctionKey:
  case NSInsertLineFunctionKey:
  case NSDeleteLineFunctionKey:
  case NSInsertCharFunctionKey:
  case NSDeleteCharFunctionKey:
  case NSPrevFunctionKey:
  case NSNextFunctionKey:
  case NSSelectFunctionKey:
  case NSExecuteFunctionKey:
  case NSUndoFunctionKey:
  case NSRedoFunctionKey:
  case NSFindFunctionKey:
    break;
  case 0x05:
  case NSHelpFunctionKey:
    return KeyboardButton::help();
  case NSModeSwitchFunctionKey:
    break;
  }
  return ButtonHandle::none();
}

////////////////////////////////////////////////////////////////////
//     Function: CocoaGraphicsWindow::map_raw_key
//       Access: Private
//  Description: Maps a keycode to a ButtonHandle.
////////////////////////////////////////////////////////////////////
ButtonHandle CocoaGraphicsWindow::
map_raw_key(unsigned short keycode) const {
  if (keycode > 0x7f) {
    return ButtonHandle::none();
  }
  switch ((unsigned char) keycode) {
  /* See HIToolBox/Events.h */
  case 0x00: return KeyboardButton::ascii_key('a');
  case 0x01: return KeyboardButton::ascii_key('s');
  case 0x02: return KeyboardButton::ascii_key('d');
  case 0x03: return KeyboardButton::ascii_key('f');
  case 0x04: return KeyboardButton::ascii_key('h');
  case 0x05: return KeyboardButton::ascii_key('g');
  case 0x06: return KeyboardButton::ascii_key('z');
  case 0x07: return KeyboardButton::ascii_key('x');
  case 0x08: return KeyboardButton::ascii_key('c');
  case 0x09: return KeyboardButton::ascii_key('v');
  case 0x0B: return KeyboardButton::ascii_key('b');
  case 0x0C: return KeyboardButton::ascii_key('q');
  case 0x0D: return KeyboardButton::ascii_key('w');
  case 0x0E: return KeyboardButton::ascii_key('e');
  case 0x0F: return KeyboardButton::ascii_key('r');
  case 0x10: return KeyboardButton::ascii_key('y');
  case 0x11: return KeyboardButton::ascii_key('t');
  case 0x12: return KeyboardButton::ascii_key('1');
  case 0x13: return KeyboardButton::ascii_key('2');
  case 0x14: return KeyboardButton::ascii_key('3');
  case 0x15: return KeyboardButton::ascii_key('4');
  case 0x16: return KeyboardButton::ascii_key('6');
  case 0x17: return KeyboardButton::ascii_key('5');
  case 0x18: return KeyboardButton::ascii_key('=');
  case 0x19: return KeyboardButton::ascii_key('9');
  case 0x1A: return KeyboardButton::ascii_key('7');
  case 0x1B: return KeyboardButton::ascii_key('-');
  case 0x1C: return KeyboardButton::ascii_key('8');
  case 0x1D: return KeyboardButton::ascii_key('0');
  case 0x1E: return KeyboardButton::ascii_key(']');
  case 0x1F: return KeyboardButton::ascii_key('o');
  case 0x20: return KeyboardButton::ascii_key('u');
  case 0x21: return KeyboardButton::ascii_key('[');
  case 0x22: return KeyboardButton::ascii_key('i');
  case 0x23: return KeyboardButton::ascii_key('p');
  case 0x24: return KeyboardButton::enter();
  case 0x25: return KeyboardButton::ascii_key('l');
  case 0x26: return KeyboardButton::ascii_key('j');
  case 0x27: return KeyboardButton::ascii_key('\'');
  case 0x28: return KeyboardButton::ascii_key('k');
  case 0x29: return KeyboardButton::ascii_key(';');
  case 0x2A: return KeyboardButton::ascii_key('\\');
  case 0x2B: return KeyboardButton::ascii_key(',');
  case 0x2C: return KeyboardButton::ascii_key('/');
  case 0x2D: return KeyboardButton::ascii_key('n');
  case 0x2E: return KeyboardButton::ascii_key('m');
  case 0x2F: return KeyboardButton::ascii_key('.');
  case 0x30: return KeyboardButton::tab();
  case 0x31: return KeyboardButton::ascii_key(' ');
  case 0x32: return KeyboardButton::ascii_key('`');
  case 0x33: return KeyboardButton::backspace();
  case 0x35: return KeyboardButton::escape();
  case 0x36: return KeyboardButton::rmeta();
  case 0x37: return KeyboardButton::lmeta();
  case 0x38: return KeyboardButton::lshift();
  case 0x39: return KeyboardButton::caps_lock();
  case 0x3A: return KeyboardButton::lalt();
  case 0x3B: return KeyboardButton::lcontrol();
  case 0x3C: return KeyboardButton::rshift();
  case 0x3D: return KeyboardButton::ralt();
  case 0x3E: return KeyboardButton::rcontrol();
  case 0x41: return KeyboardButton::ascii_key('.');
  case 0x43: return KeyboardButton::ascii_key('*');
  case 0x45: return KeyboardButton::ascii_key('+');
  case 0x47: return KeyboardButton::num_lock();
  case 0x4B: return KeyboardButton::ascii_key('/');
  case 0x4C: return KeyboardButton::enter();
  case 0x4E: return KeyboardButton::ascii_key('-');
  case 0x51: return KeyboardButton::ascii_key('=');
  case 0x52: return KeyboardButton::ascii_key('0');
  case 0x53: return KeyboardButton::ascii_key('1');
  case 0x54: return KeyboardButton::ascii_key('2');
  case 0x55: return KeyboardButton::ascii_key('3');
  case 0x56: return KeyboardButton::ascii_key('4');
  case 0x57: return KeyboardButton::ascii_key('5');
  case 0x58: return KeyboardButton::ascii_key('6');
  case 0x59: return KeyboardButton::ascii_key('7');
  case 0x5B: return KeyboardButton::ascii_key('8');
  case 0x5C: return KeyboardButton::ascii_key('9');
  case 0x60: return KeyboardButton::f5();
  case 0x61: return KeyboardButton::f6();
  case 0x62: return KeyboardButton::f7();
  case 0x63: return KeyboardButton::f3();
  case 0x64: return KeyboardButton::f8();
  case 0x65: return KeyboardButton::f9();
  case 0x67: return KeyboardButton::f11();
  case 0x69: return KeyboardButton::print_screen();
  case 0x6B: return KeyboardButton::scroll_lock();
  case 0x6D: return KeyboardButton::f10();
  case 0x6E: return KeyboardButton::menu();
  case 0x6F: return KeyboardButton::f12();
  case 0x71: return KeyboardButton::pause();
  case 0x72: return KeyboardButton::insert();
  case 0x73: return KeyboardButton::home();
  case 0x74: return KeyboardButton::page_up();
  case 0x75: return KeyboardButton::del();
  case 0x76: return KeyboardButton::f4();
  case 0x77: return KeyboardButton::end();
  case 0x78: return KeyboardButton::f2();
  case 0x79: return KeyboardButton::page_down();
  case 0x7A: return KeyboardButton::f1();
  case 0x7B: return KeyboardButton::left();
  case 0x7C: return KeyboardButton::right();
  case 0x7D: return KeyboardButton::down();
  case 0x7E: return KeyboardButton::up();
  default:   return ButtonHandle::none();
  }
}
