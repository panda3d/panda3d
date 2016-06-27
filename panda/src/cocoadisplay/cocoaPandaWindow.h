/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cocoaPandaWindow.h
 * @author rdb
 * @date 2012-05-25
 */

#import <AppKit/NSWindow.h>

class CocoaGraphicsWindow;

@interface CocoaPandaWindow : NSWindow {
  @private
    CocoaGraphicsWindow *_graphicsWindow;
}

- (id) initWithContentRect:(NSRect)rect styleMask:(NSUInteger)styleMask screen:(NSScreen*)screen window:(CocoaGraphicsWindow*)window;
- (BOOL) canBecomeKeyWindow;
- (BOOL) canBecomeMainWindow;
- (BOOL) acceptsFirstResponder;
- (BOOL) becomeFirstResponder;
- (BOOL) resignFirstResponder;

@end
