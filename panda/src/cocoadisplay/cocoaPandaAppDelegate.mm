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

@implementation CocoaPandaAppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)notification {
  // This only seems to work when called here.
  [NSApp activateIgnoringOtherApps:YES];
}

@end
