/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cocoaPandaView.mm
 * @author rdb
 * @date 2012-05-17
 */

#include "config_cocoadisplay.h"
#import "cocoaPandaView.h"
#import "cocoaGraphicsWindow.h"

#include <OpenGL/gl.h>

@implementation CocoaPandaView
- (id) initWithFrame:(NSRect)frameRect context:(NSOpenGLContext*)context window:(CocoaGraphicsWindow*)window {
  self = [super initWithFrame: frameRect];

  _context = context;
#if __MAC_OS_X_VERSION_MAX_ALLOWED >= 1060
  [self setCanDrawConcurrently:YES];
#endif

  // If a layer ends up becoming attached to the view, tell AppKit we'll manage
  // the redrawing since we're doing things our own way.
  self.layerContentsRedrawPolicy = NSViewLayerContentsRedrawNever;

  if (cocoadisplay_cat.is_debug()) {
    cocoadisplay_cat.debug()
      << "Created CocoaPandaView " << self << " for GraphicsWindow " << window << "\n";
  }
  _graphicsWindow = window;

  return self;
}

- (NSOpenGLContext*) openGLContext {
  return _context;
}

- (GraphicsWindow*) graphicsWindow {
  return _graphicsWindow;
}

- (void) drawRect:(NSRect)dirtyRect {
  // Do nothing.  We draw from another thread.

  if (cocoadisplay_cat.is_spam()) {
    cocoadisplay_cat.spam()
      << "drawRect was called.\n";
  }
}

- (void) finalize {
  _graphicsWindow->handle_close_event();
  [super finalize];
}

- (BOOL) isFlipped {
  // Apple uses a coordinate system where the lower-left corner represents (0,
  // 0).  In Panda, this is the upper-left corner.
  return YES;
}

- (BOOL) needsDisplay {
  // Never, we don't use drawRect.  We draw from elsewhere.
  return NO;
}

- (BOOL) acceptsFirstResponder {
  return YES;
}

- (BOOL) becomeFirstResponder {
  return YES;
}

- (BOOL) resignFirstResponder {
  return YES;
}

- (void) resetCursorRects {
  if (_graphicsWindow->_cursor != nil) {
    [self addCursorRect:[self bounds] cursor:_graphicsWindow->_cursor];
  }
}

- (void) setFrame: (NSRect) frame {
  [super setFrame: frame];
  // _graphicsWindow->handle_resize_event();
}

- (void) keyDown: (NSEvent *) event {
  _graphicsWindow->handle_key_event(event);
}

- (void) keyUp: (NSEvent *) event {
  _graphicsWindow->handle_key_event(event);
}

- (void) flagsChanged: (NSEvent *) event {
  _graphicsWindow->handle_key_event(event);
}

- (void) mouseDown: (NSEvent *) event {
  _graphicsWindow->handle_mouse_button_event(0, true);
}

- (void) mouseDragged: (NSEvent *) event {
  [self mouseMoved: event];
}

- (void) mouseUp: (NSEvent *) event {
  _graphicsWindow->handle_mouse_button_event(0, false);
}

- (void) mouseMoved: (NSEvent *) event {
  NSPoint loc = [self convertPoint:[event locationInWindow] fromView:nil];
  BOOL inside = [self mouse:loc inRect:[self bounds]];

  // the correlation between mouse deltas and location are "debounced"
  // apparently, so send deltas for both relative and confined modes
  if (_graphicsWindow->get_properties().get_mouse_mode() != WindowProperties::M_absolute) {
    _graphicsWindow->handle_mouse_moved_event(inside, [event deltaX], [event deltaY], false);
  } else {
    _graphicsWindow->handle_mouse_moved_event(inside, loc.x, loc.y, true);
  }
}

- (void) rightMouseDown: (NSEvent *) event {
  _graphicsWindow->handle_mouse_button_event(2, true);
}

- (void) rightMouseDragged: (NSEvent *) event {
  [self mouseMoved: event];
}

- (void) rightMouseUp: (NSEvent *) event {
  _graphicsWindow->handle_mouse_button_event(2, false);
}

- (void) otherMouseDown: (NSEvent *) event {
  // 1 and 2 are swapped, for consistency with X11 implementation
  if ([event buttonNumber] == 2) {
    _graphicsWindow->handle_mouse_button_event(1, true);
  } else {
    _graphicsWindow->handle_mouse_button_event([event buttonNumber], true);
  }
}

- (void) otherMouseDragged: (NSEvent *) event {
  [self mouseMoved: event];
}

- (void) otherMouseUp: (NSEvent *) event {
  // 1 and 2 are swapped, for consistency with X11 implementation
  if ([event buttonNumber] == 2) {
    _graphicsWindow->handle_mouse_button_event(1, false);
  } else {
    _graphicsWindow->handle_mouse_button_event([event buttonNumber], false);
  }
}

- (void) scrollWheel: (NSEvent *) event {
  _graphicsWindow->handle_wheel_event([event deltaX], [event deltaY]);
}

- (BOOL) isOpaque {
  return YES;
}

-(void)setLayer:(CALayer*)layer
{
    [super setLayer:layer];

    // Starting in macOS 10.14, a CALayer will still be attached to a view even
    // if `wantsLayer` is false. If we don't update the context now, only a
    // black screen will be rendered until the context is updated some other
    // way (like through a window resize event).
    [_context update];
}
@end
