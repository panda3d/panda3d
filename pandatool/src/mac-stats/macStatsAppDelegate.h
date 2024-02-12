/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file macStatsAppDelegate.h
 * @author rdb
 * @date 2023-08-17
 */

#ifndef MACSTATSAPPDELEGATE_H
#define MACSTATSAPPDELEGATE_H

#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>

class MacStatsServer;

@interface MacStatsAppDelegate : NSObject<NSApplicationDelegate, NSUserNotificationCenterDelegate> {
  @private
    MacStatsServer *_server;
    NSTimer *_timer;
}

- (id)initWithServer:(MacStatsServer *)server;
- (void)applicationDidFinishLaunching:(NSApplication *)sender;
- (BOOL)applicationShouldTerminate:(NSApplication *)sender;
- (void)applicationWillTerminate:(NSApplication *)sender;
- (BOOL)userNotificationCenter:(NSUserNotificationCenter *)center
                               shouldPresentNotification:(NSUserNotification *)notification;

@end

#endif
