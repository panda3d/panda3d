/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file macStatsServer.h
 * @author rdb
 * @date 2023-08-17
 */

#ifndef MACSTATSSERVER_H
#define MACSTATSSERVER_H

#include "pandatoolbase.h"
#include "programBase.h"
#include "pStatServer.h"
#include "macStatsMonitor.h"
#include "macStatsAppDelegate.h"

#include <Cocoa/Cocoa.h>

/**
 * The class that owns the main loop, waiting for client connections.
 */
class MacStatsServer : public PStatServer, public ProgramBase {
public:
  MacStatsServer();

  MacStatsMonitor *get_monitor() { return _monitor; }

  void run(int argc, char *argv[]);

protected:
  virtual bool handle_args(Args &args) override;

  virtual PStatMonitor *make_monitor(const NetAddress &address) override;
  virtual void lost_connection(PStatMonitor *monitor) override;

public:
  bool new_session();
  bool open_session(const Filename &fn);
  bool open_session();
  bool open_last_session();
  bool save_session();
  bool export_session();
  bool close_session();

  void set_show_status_item(bool show);
  void set_appearance(NSString *name);
  void set_time_units(int unit_mask);

private:
  void create_app();
  void setup_session_menu();

private:
  PT(MacStatsMonitor) _monitor;

  Filename _last_session;
  Filename _save_filename;

  int _port = -1;
  NSApplication *_app = nil;
  NSUserNotification *_listening_notification = nil;
  NSMenu *_main_menu = nil;
  NSMenuItem *_show_status_item_menu_item = nil;
  NSMenuItem *_appearance_system_menu_item = nil;
  NSMenuItem *_appearance_aqua_menu_item = nil;
  NSMenuItem *_appearance_dark_aqua_menu_item = nil;
  NSMenuItem *_units_ms_menu_item = nil;
  NSMenuItem *_units_hz_menu_item = nil;
  NSMenuItem *_new_session_menu_item = nil;
  NSMenuItem *_open_last_session_menu_item = nil;
  NSMenuItem *_save_session_menu_item = nil;
  NSMenuItem *_close_session_menu_item = nil;
  NSMenuItem *_export_session_menu_item = nil;
};

#endif
