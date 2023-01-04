/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file winStatsServer.h
 * @author drose
 * @date 2003-12-02
 */

#ifndef WINSTATSSERVER_H
#define WINSTATSSERVER_H

#include "pandatoolbase.h"
#include "programBase.h"
#include "pStatServer.h"
#include "winStatsMonitor.h"

/**
 * The class that owns the main loop, waiting for client connections.
 */
class WinStatsServer : public PStatServer, public ProgramBase {
public:
  WinStatsServer();

  virtual bool handle_args(Args &args) override;

  virtual PStatMonitor *make_monitor(const NetAddress &address) override;
  virtual void lost_connection(PStatMonitor *monitor) override;

  bool new_session();
  bool open_session();
  bool open_last_session();
  bool save_session();
  bool export_session();
  bool close_session();

  HWND get_window() const;
  HMENU get_menu_bar() const;
  HWND get_status_bar() const;
  HFONT get_font() const;
  int get_pixel_scale() const;
  POINT get_client_origin() const;

  int get_time_units() const;
  void set_time_units(int unit_mask);

private:
  void create_window();
  void setup_session_menu();
  void setup_options_menu();
  void create_status_bar(HINSTANCE application);
  static void register_window_class(HINSTANCE application);

  static LONG WINAPI static_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
  LONG window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
  void handle_menu_command(int menu_id);

  PT(WinStatsMonitor) _monitor;

  Filename _last_session;

  int _port = -1;
  HWND _window = 0;
  HMENU _menu_bar = 0;
  HMENU _session_menu = 0;
  HMENU _options_menu = 0;
  HWND _status_bar;
  POINT _client_origin;
  int _time_units = 0;
  int _pixel_scale;

  HFONT _font;

  static bool _window_class_registered;
  static const char * const _window_class_name;
};

#endif
