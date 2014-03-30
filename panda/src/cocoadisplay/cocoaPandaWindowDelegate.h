// Filename: cocoaPandaWindowDelegate.h
// Created by:  rdb (24May12)
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

#import <AppKit/NSWindow.h>

#import "cocoaGraphicsWindow.h"

class CocoaGraphicsWindow;

#if __MAC_OS_X_VERSION_MAX_ALLOWED >= 1060
@interface CocoaPandaWindowDelegate : NSObject<NSWindowDelegate> {
#else
@interface CocoaPandaWindowDelegate : NSObject {
#endif
  @private
    CocoaGraphicsWindow *_graphicsWindow;
}

- (id) initWithGraphicsWindow:(CocoaGraphicsWindow*)window;
- (void)windowDidMove:(NSNotification *)notification;
- (void)windowDidResize:(NSNotification *)notification;
- (void)windowDidMiniaturize:(NSNotification *)notification;
- (void)windowDidDeminiaturize:(NSNotification *)notification;
- (void)windowDidBecomeKey:(NSNotification *)notification;
- (void)windowDidResignKey:(NSNotification *)notification;
- (BOOL)windowShouldClose:(id)sender;
- (void)windowWillClose:(NSNotification *)notification;

//TODO: handle fullscreen on Lion.

@end
