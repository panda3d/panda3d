/**
* PANDA 3D SOFTWARE
* Copyright (c) Carnegie Mellon University.  All rights reserved.
*
* All use of this software is subject to the terms of the revised BSD
* license.  You should have received a copy of this license along
* with this source code in a file named "LICENSE."
*
* @file cocoaPandaAppDelegate.mm
* @author Donny Lawrence
* @date 2018-02-25
*/

#import "cocoaPandaAppDelegate.h"
#include "graphicsEngine.h"
#include "config_cocoadisplay.h"

@implementation CocoaPandaAppDelegate

- (id) initWithEngine:(GraphicsEngine *)engine {

  if (self = [super init]) {
    _engine = engine;
  }

  return self;
}

- (BOOL)applicationSupportsSecureRestorableState:(NSApplication *)app {
  // Squelches an annoying warning.
  return YES;
}

- (void)applicationDidFinishLaunching:(NSNotification *)notification {
  // This only seems to work when called here.
  [NSApp activateIgnoringOtherApps:YES];
}

- (BOOL)applicationShouldTerminate:(NSApplication *)app {
  if (cocoadisplay_cat.is_debug()) {
    cocoadisplay_cat.debug()
      << "Received applicationShouldTerminate, requesting to close all Cocoa windows\n";
  }
  // Ask all the windows whether they are OK to be closed.
  bool should_close = true;
  for (NSWindow *window in [app windows]) {
    id<NSWindowDelegate> delegate = [window delegate];
    if (delegate != nil && ![delegate windowShouldClose:window]) {
      should_close = false;
    }
  }
  if (should_close) {
    if (cocoadisplay_cat.is_debug()) {
      cocoadisplay_cat.debug()
        << "No window objected to close request, closing all windows\n";
    }
    // If so (none of them fired a close request event), close them now.
    for (NSWindow *window in [app windows]) {
      [window close];
    }
  }
  // Give the application a chance to run its own cleanup functions.
  return FALSE;
}

- (void)applicationWillTerminate:(NSNotification *)notification {
  // The application is about to be closed, tell the graphics engine to close
  // all the windows.
  if (cocoadisplay_cat.is_debug()) {
    cocoadisplay_cat.debug()
      << "Received applicationWillTerminate, removing all windows\n";
  }
  _engine->remove_all_windows();
}

@end
