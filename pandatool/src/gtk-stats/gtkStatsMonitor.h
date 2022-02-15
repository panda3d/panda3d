/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file gtkStatsMonitor.h
 * @author drose
 * @date 2006-01-16
 */

#ifndef GTKSTATSMONITOR_H
#define GTKSTATSMONITOR_H

#include "pandatoolbase.h"

#include "gtkStatsGraph.h"
#include "pStatMonitor.h"
#include "pointerTo.h"
#include "pset.h"
#include "pvector.h"
#include "pmap.h"

#include <gtk/gtk.h>

class GtkStatsServer;
class GtkStatsChartMenu;

/**
 * This class represents a connection to a PStatsClient and manages the data
 * exchange with the client.
 */
class GtkStatsMonitor : public PStatMonitor {
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
    GtkStatsMonitor *_monitor;
  };

  GtkStatsMonitor(GtkStatsServer *server);
  virtual ~GtkStatsMonitor();

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

  GtkWidget *get_window() const;
  double get_resolution() const;

  void open_strip_chart(int thread_index, int collector_index, bool show_level);
  void open_piano_roll(int thread_index);
  void open_flame_graph(int thread_index, int collector_index = -1);
  void open_timeline();

  const MenuDef *add_menu(const MenuDef &menu_def);

  void set_time_units(int unit_mask);
  void set_scroll_speed(double scroll_speed);
  void set_pause(bool pause);

private:
  void add_graph(GtkStatsGraph *graph);
  void remove_graph(GtkStatsGraph *graph);

  void create_window();
  void shutdown();
  static gboolean window_delete_event(GtkWidget *widget, GdkEvent *event,
              gpointer data);
  static void window_destroy(GtkWidget *widget, gpointer data);
  void setup_options_menu();
  void setup_speed_menu();
  void setup_frame_rate_label();
  void update_status_bar();
  bool show_popup_menu(int collector);

  static gboolean status_bar_button_event(GtkWidget *widget,
                                          GdkEventButton *event,
                                          gpointer data);
public:
  static void menu_activate(GtkWidget *widget, gpointer data);

private:
  typedef pset<GtkStatsGraph *> Graphs;
  Graphs _graphs;

  typedef pvector<GtkStatsChartMenu *> ChartMenus;
  ChartMenus _chart_menus;

  typedef pset<MenuDef> Menus;
  Menus _menus;

  GtkWidget *_window;
  GtkWidget *_menu_bar;
  GtkWidget *_options_menu;
  GtkWidget *_speed_menu;
  int _next_chart_index;
  GtkWidget *_frame_rate_menu_item;
  GtkWidget *_frame_rate_label;
  GtkWidget *_status_bar;
  pvector<int> _status_bar_collectors;
  pvector<GtkWidget *> _status_bar_labels;
  std::string _window_title;
  int _time_units;
  double _scroll_speed;
  bool _pause;
  double _resolution;

  friend class GtkStatsGraph;
};

#include "gtkStatsMonitor.I"

#endif
