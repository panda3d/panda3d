/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file macStats.mm
 * @author rdb
 * @date 2023-08-17
 */

#include "pandatoolbase.h"
#include "macStats.h"
#include "macStatsServer.h"
#include "config_pstatclient.h"

#include <Carbon/Carbon.h>
#include <objc/runtime.h>

extern "C" {
  OSStatus CPSSetProcessName(ProcessSerialNumber *psn, char *name);
};

@implementation NSBundle(swizzle)
- (NSString *)__bundleIdentifier {
  if (self == [NSBundle mainBundle]) {
    return @"org.panda3d.pstats";
  } else {
    return [self __bundleIdentifier];
  }
}
@end

void
keyboard_interrupt(int sig) {
  dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^(void) {
    [NSApp terminate:NSApp];
  });
}

int
main(int argc, char *argv[]) {
  // This hack is necessary to allow showing notifications when run as a console app.
  Class cls = objc_getClass("NSBundle");
  if (cls) {
    method_exchangeImplementations(class_getInstanceMethod(cls, @selector(bundleIdentifier)),
                                   class_getInstanceMethod(cls, @selector(__bundleIdentifier)));
  }

  // Set the bundle name of the application.
  ProcessSerialNumber psn;
  GetCurrentProcess(&psn);
  CPSSetProcessName(&psn, (char *)"PStats");

  @autoreleasepool {
    // Create the server application.
    MacStatsServer *server = new MacStatsServer;

    // Register a SIGINT handler to terminate the application correctly.
    // Otherwise, notifications may linger in the notification center.
    struct sigaction act = {};
    act.sa_handler = &keyboard_interrupt;
    act.sa_flags = SA_RESETHAND;
    sigaction(SIGINT, &act, nullptr);

    // Get lost in the Cocoa main loop.
    server->run(argc, argv);
  }

  return 0;
}
