// Filename: cocoaPandaApp.mm
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

#import "cocoaPandaApp.h"

@implementation CocoaPandaApp
- (void) sendEvent: (NSEvent *) event {
  // This is a hack that allows us to receive cmd-key-up events correctly.
  // Also prevent it from eating the insert/help key.
  if (([event type] == NSKeyUp && ([event modifierFlags] & NSCommandKeyMask))
    ||([event type] == NSKeyDown && [event keyCode] == 0x72)) {

    [[self keyWindow] sendEvent: event];
  } else {
    [super sendEvent: event];
  }
}
@end
