/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file macStatsAppDelegate.mm
 * @author rdb
 * @date 2023-08-17
 */

#include "macStatsAppDelegate.h"
#include "macStatsServer.h"
#include "pStatGraph.h"

@implementation MacStatsAppDelegate

- (id)initWithServer:(MacStatsServer *)server {
  if (self = [super init]) {
    _server = server;
    _timer = nil;
  }

  return self;
}

- (BOOL)applicationSupportsSecureRestorableState:(NSApplication *)app {
  // Squelches an annoying warning.
  return YES;
}

- (BOOL)application:(NSApplication *)sender
           openFile:(NSString *)filename {
  Filename fn([filename UTF8String]);
  fn.set_binary();
  return _server->open_session(fn);
}

- (void)applicationDidFinishLaunching:(NSApplication *)sender {
  // Set this object as delegate for user notifications.
  {
    NSUserNotificationCenter *center = [NSUserNotificationCenter defaultUserNotificationCenter];
    center.delegate = self;
  }

  // Register default preferences.
  NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
  NSDictionary *dict = [NSDictionary
    dictionaryWithObjects:@[@"", [NSNumber numberWithBool:YES], [NSNumber numberWithInt:PStatGraph::GBU_ms]]
                  forKeys:@[@"Appearance", @"ShowStatusItem", @"TimeUnits"]];
  [defaults registerDefaults:dict];

  if (_server != nil) {
    // Apply preferences.
    [self applyDefaults:nil];

    // Create a timer to poll the server.
    _timer = [NSTimer scheduledTimerWithTimeInterval:0.2
      target:self
      selector:@selector(pollServer)
      userInfo:nil
      repeats:YES];

    // Start new session if we don't have one yet.
    if (_server->get_monitor() == nullptr) {
      _server->new_session();
    }
  }

  // Watch for defaults change.
  {
    NSNotificationCenter *center = [NSNotificationCenter defaultCenter];
    [center addObserver:self
               selector:@selector(applyDefaults:)
                   name:NSUserDefaultsDidChangeNotification
                 object:nil];
  }
}

- (void)applyDefaults:(NSNotification *)notification {
  if (_server != nil) {
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    _server->set_show_status_item([defaults boolForKey:@"ShowStatusItem"]);
    _server->set_appearance([defaults stringForKey:@"Appearance"]);
    _server->set_time_units([defaults integerForKey:@"TimeUnits"]);
  }
}

- (void)pollServer {
  _server->poll();
}

- (BOOL)applicationShouldTerminate:(NSApplication *)sender {
  if (_server != nil) {
    return _server->close_session();
  }
  return YES;
}

- (void)applicationWillTerminate:(NSApplication *)sender {
  if (_timer != nil) {
    [_timer invalidate];
    _timer = nil;
  }

  if (_server != nil) {
    delete _server;
    _server = nil;
  }
}

- (BOOL)userNotificationCenter:(NSUserNotificationCenter *)center
                               shouldPresentNotification:(NSUserNotification *)notification {
  return YES;
}

- (void)userNotificationCenter:(NSUserNotificationCenter *)center
       didActivateNotification:(NSUserNotification *)notification {
  NSUserNotificationAction *action = notification.additionalActivationAction;
  if (action != nil) {
    NSString *ident = action.identifier;
    if ([ident isEqual:@"quit"]) {
      [NSApp terminate:self];
    }
    else if ([ident isEqual:@"new"]) {
      _server->new_session();
    }
    else if ([ident isEqual:@"open"]) {
      _server->open_session();
    }
    else if ([ident isEqual:@"openLast"]) {
      _server->open_last_session();
    }
  }
}

- (void)handleNewSession:(NSMenuItem *)item {
  _server->new_session();
}

- (void)handleOpenSession:(NSMenuItem *)item {
  _server->open_session();
}

- (void)handleOpenLastSession:(NSMenuItem *)item {
  _server->open_last_session();
}

- (void)handleSaveSession:(NSMenuItem *)item {
  _server->save_session();
}

- (void)handleCloseSession:(NSMenuItem *)item {
  _server->close_session();
}

- (void)handleExportSession:(NSMenuItem *)item {
  _server->export_session();
}

- (void)handleToggleSettingsBool:(NSMenuItem *)item {
  [[NSUserDefaults standardUserDefaults] setBool:(item.state != NSOnState) forKey:item.representedObject];
}

- (void)handleSettingsInteger:(NSMenuItem *)item {
  [[NSUserDefaults standardUserDefaults] setInteger:item.tag forKey:item.representedObject];
}

- (void)handleSettingsAppearance:(NSMenuItem *)item {
  [[NSUserDefaults standardUserDefaults] setObject:item.representedObject forKey:@"Appearance"];
}

- (void)handleSpeed:(NSMenuItem *)item {
  MacStatsMonitor *monitor = _server->get_monitor();
  if (monitor != nullptr) {
    monitor->set_scroll_speed(item.tag);
  }
}

- (void)handlePause:(NSMenuItem *)item {
  MacStatsMonitor *monitor = _server->get_monitor();
  if (monitor != nullptr) {
    monitor->set_pause(!item.state);
  }
}

- (void)handleClickStatusItem:(id)sender {
  [NSApp activateIgnoringOtherApps:YES];
}

- (void)handleChooseCollectorColor:(NSColorPanel *)panel {
  MacStatsMonitor *monitor = _server->get_monitor();
  if (monitor != nullptr) {
    NSColor *color = panel.color;
    monitor->handle_choose_collector_color(LRGBColor(color.redComponent, color.greenComponent, color.blueComponent));
  }
}

@end
