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
#include "config_cocoadisplay.h"

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
  _graphicsWindow->handle_move_event();
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

- (void) windowDidChangeBackingProperties:(NSNotification *)notification {
  _graphicsWindow->handle_backing_change_event();
}

- (BOOL) windowShouldClose:(id)sender {
  if (cocoadisplay_cat.is_debug()) {
    cocoadisplay_cat.debug()
      << "Received windowShouldClose for window " << _graphicsWindow << "\n";
  }
  return _graphicsWindow->handle_close_request();
}

- (void) windowWillClose:(id)sender {
  if (cocoadisplay_cat.is_debug()) {
    cocoadisplay_cat.debug()
      << "Received windowWillClose for window " << _graphicsWindow << "\n";
  }
  _graphicsWindow->handle_close_event();
}

@end
