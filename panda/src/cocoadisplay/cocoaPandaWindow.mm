// Filename: cocoaPandaWindow.mm
// Created by:  rdb (25May12)
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

#import "cocoaPandaWindow.h"
#import "cocoaPandaWindowDelegate.h"

@implementation CocoaPandaWindow
- (id) initWithContentRect:(NSRect)rect styleMask:(NSUInteger)styleMask screen:(NSScreen*)screen window:(CocoaGraphicsWindow*)window {

  if (self = [super initWithContentRect:rect
                    styleMask:styleMask
                    backing:NSBackingStoreBuffered
                    defer:YES
                    screen:screen]) {
    _graphicsWindow = window;

    CocoaPandaWindowDelegate *delegate = [[CocoaPandaWindowDelegate alloc] initWithGraphicsWindow:window];
    [self setDelegate:delegate];
    [self setOpaque:YES];
    [self setReleasedWhenClosed:YES];
#if __MAC_OS_X_VERSION_MAX_ALLOWED >= 1060
    [self setAllowsConcurrentViewDrawing:YES];
#endif

    // Necessary to be able to accept mouseMoved in the NSView
    [self setAcceptsMouseMovedEvents:YES];
  }

  return self;
}

- (BOOL) canBecomeKeyWindow {
  // Otherwise borderless windows won't be able to get keyboard events.
  return YES;
}

- (BOOL) canBecomeMainWindow {
  return YES;
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

@end
