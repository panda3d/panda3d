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

@implementation CocoaPandaAppDelegate

- (id) initWithEngine:(GraphicsEngine *)engine {

  if (self = [super init]) {
    _engine = engine;
  }

  return self;
}

- (void)applicationDidFinishLaunching:(NSNotification *)notification {
  // This only seems to work when called here.
  [NSApp activateIgnoringOtherApps:YES];
}

- (BOOL)applicationShouldTerminate:(NSApplication *)app {
  if (cocoadisplay_cat.is_debug()) {
    cocoadisplay_cat.debug()
      << "Received applicationShouldTerminate, closing all Cocoa windows\n";
  }
  // Call performClose on all the windows.  This should make ShowBase shut down.
  for (NSWindow *window in [app windows]) {
    [window performClose:nil];
  }
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
