/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file macStatsMonitor.h
 * @author rdb
 * @date 2023-08-17
 */

#ifndef MACSTATSMONITOR_H
#define MACSTATSMONITOR_H

#include "pandatoolbase.h"

#include "pStatMonitor.h"
#include "pointerTo.h"
#include "pset.h"
#include "pvector.h"
#include "pmap.h"

#import <Cocoa/Cocoa.h>

class MacStatsGraph;
class MacStatsServer;
class MacStatsChartMenu;

/**
 * This class represents a connection to a PStatsClient and manages the data
 * exchange with the client.
 */
class MacStatsMonitor : public PStatMonitor {
public:
  MacStatsMonitor(MacStatsServer *server);
  virtual ~MacStatsMonitor();

  void close();

  virtual std::string get_monitor_name();

  virtual void initialized();
  virtual void got_hello();
  virtual void got_bad_version(int client_major, int client_minor,
                               int server_major, int server_minor);
  virtual void new_collector(int collector_index);
  virtual void new_thread(int thread_index);
  virtual void new_data(int thread_index, int frame_number);
  virtual void remove_thread(int thread_index);
  virtual void lost_connection();
  virtual void idle();
  virtual bool has_idle();

  virtual void user_guide_bars_changed();

  PStatGraph *open_timeline();
  PStatGraph *open_strip_chart(int thread_index, int collector_index, bool show_level);
  PStatGraph *open_flame_graph(int thread_index, int collector_index = -1, int frame_number = -1);
  PStatGraph *open_piano_roll(int thread_index);

  CGColorRef get_collector_color(int collector_index, bool highlight = false);
  NSColor *get_collector_text_color(int collector_index, bool highlight = false);

  void choose_collector_color(int collector_index);
  void handle_choose_collector_color(const LRGBColor &color);
  void reset_collector_color(int collector_index);

  void set_show_status_item(bool show);
  void set_time_units(int unit_mask);
  void set_scroll_speed(double scroll_speed);
  void set_pause(bool pause);

  void add_graph(MacStatsGraph *graph);
  void remove_graph(MacStatsGraph *graph);
  void close_all_graphs();

private:
  void setup_speed_menu();
  void update_status_bar();

private:
  typedef pset<MacStatsGraph *> Graphs;
  Graphs _graphs;

  typedef pvector<MacStatsChartMenu *> ChartMenus;
  ChartMenus _chart_menus;

  NSUserNotification *_notification = nullptr;
  NSMenu *_main_menu;
  NSMenuItem *_speed_menu_item = nullptr;
  NSMenuItem *_speed_menu_item_1;
  NSMenuItem *_speed_menu_item_2;
  NSMenuItem *_speed_menu_item_3;
  NSMenuItem *_speed_menu_item_6;
  NSMenuItem *_speed_menu_item_12;
  NSMenuItem *_speed_menu_item_pause;
  int _next_chart_index;
  NSStatusItem *_frame_rate_status_item = nullptr;
  double _scroll_speed;
  bool _pause;
  bool _have_data = false;
  int _choosing_color_collector_index;

  struct ColorSet {
    CGColorRef _bg[2];
    NSColor *_fg[2];
  };
  typedef pmap<int, ColorSet> Colors;
  Colors _colors;

  friend class MacStatsGraph;
  friend class MacStatsServer;
};

#endif
