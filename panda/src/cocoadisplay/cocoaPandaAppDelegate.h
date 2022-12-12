/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cocoaPandaAppDelegate.h
 * @author Donny Lawrence
 * @date 2018-02-25
 */

#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>

class GraphicsEngine;

// Cocoa is picky about where and when certain methods are called in the initialization process.
@interface CocoaPandaAppDelegate : NSObject<NSApplicationDelegate> {
  @private
    GraphicsEngine *_engine;
}

- (id) initWithEngine:(GraphicsEngine *)engine;
- (void)applicationDidFinishLaunching:(NSNotification *)notification;
- (BOOL)applicationShouldTerminate:(NSApplication *)app;
- (void)applicationWillTerminate:(NSNotification *)notification;

@end
