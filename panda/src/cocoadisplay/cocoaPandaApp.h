// Filename: cocoaPandaApp.h
// Created by:  rdb (08Mar14)
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

#import <AppKit/NSApplication.h>
#import <AppKit/NSEvent.h>

// This class solely exists so that we can override sendEvent in order
// to prevent NSApplication from eating certain keyboard events.
@interface CocoaPandaApp : NSApplication
- (void) sendEvent: (NSEvent *) event;
@end
