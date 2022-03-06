/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file winStatsMonitor.h
 * @author drose
 * @date 2003-12-02
 */

#ifndef WINSTATSMONITOR_H
#define WINSTATSMONITOR_H

#include "pandatoolbase.h"

#include "winStatsGraph.h"
#include "pStatMonitor.h"
#include "pointerTo.h"
#include "pset.h"
#include "pvector.h"
#include "pmap.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>

class WinStatsServer;
class WinStatsChartMenu;

/**
 * This class represents a connection to a PStatsClient and manages the data
 * exchange with the client.
 */
class WinStatsMonitor : public PStatMonitor {
public:
  enum ChartType {
    CT_timeline,
    CT_strip_chart,
    CT_flame_graph,
    CT_piano_roll,
  };

  class MenuDef {
  public:
    INLINE MenuDef(int thread_index, int collector_index,
                   ChartType chart_type, bool show_level = false);
    INLINE bool operator < (const MenuDef &other) const;

    int _thread_index;
    int _collector_index;
    ChartType _chart_type;
    bool _show_level;
  };

  WinStatsMonitor(WinStatsServer *server);
  virtual ~WinStatsMonitor();

  virtual std::string get_monitor_name();

  virtual void initialized();
  virtual void got_hello();
  virtual void got_bad_version(int client_major, int client_minor,
                               int server_major, int server_minor);
  virtual void new_collector(int collector_index);
  virtual void new_thread(int thread_index);
  virtual void new_data(int thread_index, int frame_number);
  virtual void lost_connection();
  virtual void idle();
  virtual bool has_idle();

  virtual void user_guide_bars_changed();

  HWND get_window() const;
  HFONT get_font() const;
  int get_pixel_scale() const;
  POINT get_new_window_pos();

  void open_strip_chart(int thread_index, int collector_index, bool show_level);
  void open_piano_roll(int thread_index);
  void open_flame_graph(int thread_index, int collector_index = -1);
  void open_timeline();

  const MenuDef &lookup_menu(int menu_id) const;
  int get_menu_id(const MenuDef &menu_def);

  void set_time_units(int unit_mask);
  void set_scroll_speed(double scroll_speed);
  void set_pause(bool pause);

private:
  void add_graph(WinStatsGraph *graph);
  void remove_graph(WinStatsGraph *graph);

  void create_window();
  void setup_options_menu();
  void setup_speed_menu();
  void setup_frame_rate_label();
  void create_status_bar(HINSTANCE application);
  void update_status_bar();
  void show_popup_menu(int collector);
  static void register_window_class(HINSTANCE application);

  static LONG WINAPI static_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
  LONG window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
  void handle_menu_command(int menu_id);

  typedef pset<WinStatsGraph *> Graphs;
  Graphs _graphs;

  typedef pvector<WinStatsChartMenu *> ChartMenus;
  ChartMenus _chart_menus;

  typedef pvector<MenuDef> MenuById;
  typedef pmap<MenuDef, int> MenuByDef;
  MenuById _menu_by_id;
  MenuByDef _menu_by_def;

  HWND _window;
  HMENU _menu_bar;
  HMENU _options_menu;
  HMENU _speed_menu;
  HWND _status_bar;
  POINT _client_origin;
  pvector<int> _status_bar_collectors;
  std::string _window_title;
  int _time_units;
  double _scroll_speed;
  bool _pause;
  int _pixel_scale;

  HFONT _font;

  static bool _window_class_registered;
  static const char * const _window_class_name;

  friend class WinStatsGraph;
};

#include "winStatsMonitor.I"

#endif
