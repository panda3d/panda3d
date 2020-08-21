/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cocoaPandaApp.mm
 * @author rdb
 * @date 2014-03-08
 */

#import "cocoaPandaApp.h"
#include "config_cocoadisplay.h"

@implementation CocoaPandaApp
- (void) sendEvent: (NSEvent *) event {
  // This is a hack that allows us to receive cmd-key-up events correctly.
  // Also prevent it from eating the inserthelp key.
  if (([event type] == NSKeyUp && ([event modifierFlags] & NSCommandKeyMask))
    ||([event type] == NSKeyDown && [event keyCode] == 0x72)) {

    [[self keyWindow] sendEvent: event];
  } else {
    [super sendEvent: event];
  }
}

- (void) _setup: (void *) interp {
  // This is called by Tk when it launches and naively assumes that it is
  // the first to create an NSApplication.  We can't do anything about it
  // at this point except display an error message.

  cocoadisplay_cat.error()
    << "Detected attempt to initialize Tk after creating a Panda window.  "
       "This will likely cause a crash.\n"
       "To fix this, set 'want-tk true' in Config.prc to force "
       "initialization of Tk before opening the Panda window.\n";
}

- (void) _setupEventLoop {
  NSAutoreleasePool *pool = [NSAutoreleasePool new];
  [self finishLaunching];
  [self setWindowsNeedUpdate:YES];
  [pool drain];
}

@end
