/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cocoaPandaWindowDelegate.mm
 * @author rdb
 * @date 2012-05-24
 */

#import "cocoaPandaWindowDelegate.h"

@implementation CocoaPandaWindowDelegate
- (id) initWithGraphicsWindow:(CocoaGraphicsWindow*)window {

  if (self = [super init]) {
    _graphicsWindow = window;
  }

  return self;
}

- (void) windowDidMove:(NSNotification *)notification {
  _graphicsWindow->handle_move_event();
}

- (void) windowDidResize:(NSNotification *)notification {
  // Forcing a move event is unfortunately necessary because Cocoa does not
  // call windowDidMove in case of window zooms.
  _graphicsWindow->handle_resize_event();
}

- (void) windowDidMiniaturize:(NSNotification *)notification {
  _graphicsWindow->handle_minimize_event(true);
}

- (void) windowDidDeminiaturize:(NSNotification *)notification {
  _graphicsWindow->handle_minimize_event(false);
}

- (void) windowDidBecomeKey:(NSNotification *)notification {
  _graphicsWindow->handle_foreground_event(true);
}

- (void) windowDidResignKey:(NSNotification *)notification {
  _graphicsWindow->handle_foreground_event(false);
}

- (BOOL) windowShouldClose:(id)sender {
  bool should_close = _graphicsWindow->handle_close_request();
  if (should_close) {
    _graphicsWindow->handle_close_event();
  }
  return should_close;
}

@end
