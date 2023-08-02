/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cocoaPandaView.h
 * @author rdb
 * @date 2012-05-17
 */

#include "graphicsWindow.h"

#import <AppKit/NSView.h>

class CocoaGraphicsWindow;

@interface CocoaPandaView : NSView {
  @private
    CocoaGraphicsWindow *_graphicsWindow;
}

- (id) initWithFrame:(NSRect)frameRect window:(CocoaGraphicsWindow*)window;
- (GraphicsWindow*) graphicsWindow;

- (void) drawRect:(NSRect)dirtyRect;
- (void) finalize;
- (BOOL) isFlipped;
- (BOOL) needsDisplay;
- (BOOL) acceptsFirstResponder;
- (BOOL) becomeFirstResponder;
- (BOOL) resignFirstResponder;
- (void) resetCursorRects;

- (void) setFrame: (NSRect) frame;

// Keyboard events
- (void) keyDown: (NSEvent *) event;
- (void) keyUp: (NSEvent *) event;
- (void) flagsChanged: (NSEvent *) event;

// Mouse events
- (void) mouseDown: (NSEvent *) event;
- (void) mouseDragged: (NSEvent *) event;
- (void) mouseUp: (NSEvent *) event;
- (void) mouseMoved: (NSEvent *) event;
- (void) rightMouseDown: (NSEvent *) event;
- (void) rightMouseDragged: (NSEvent *) event;
- (void) rightMouseUp: (NSEvent *) event;
- (void) otherMouseDown: (NSEvent *) event;
- (void) otherMouseDragged: (NSEvent *) event;
- (void) otherMouseUp: (NSEvent *) event;

- (void) scrollWheel: (NSEvent *) event;

- (BOOL) isOpaque;

@end
