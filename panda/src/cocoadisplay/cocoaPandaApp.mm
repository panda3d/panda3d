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
@end
