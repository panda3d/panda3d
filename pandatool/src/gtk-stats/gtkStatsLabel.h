/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file gtkStatsLabel.h
 * @author drose
 * @date 2006-01-16
 */

#ifndef GTKSTATSLABEL_H
#define GTKSTATSLABEL_H

#include "pandatoolbase.h"

#include <gtk/gtk.h>

class GtkStatsMonitor;
class GtkStatsGraph;

/**
 * A text label that will draw in color appropriate for a particular
 * collector.  It also responds when the user double-clicks on it.  This is
 * handy for putting colored labels on strip charts.
 */
class GtkStatsLabel {
public:
  GtkStatsLabel(GtkStatsMonitor *monitor, GtkStatsGraph *graph,
                int thread_index, int collector_index, bool use_fullname);
  ~GtkStatsLabel();

  GtkWidget *get_widget() const;
  int get_height() const;

  int get_collector_index() const;
  int get_thread_index() const;

  void set_highlight(bool highlight);
  bool get_highlight() const;

private:
  void set_mouse_within(bool mouse_within);
  static gboolean expose_event_callback(GtkWidget *widget,
          GdkEventExpose *event, gpointer data);
  static gboolean enter_notify_event_callback(GtkWidget *widget,
                GdkEventCrossing *event,
                gpointer data);
  static gboolean leave_notify_event_callback(GtkWidget *widget,
                GdkEventCrossing *event,
                gpointer data);
  static gboolean button_press_event_callback(GtkWidget *widget,
                GdkEventButton *event,
                gpointer data);

  GtkStatsMonitor *_monitor;
  GtkStatsGraph *_graph;
  int _thread_index;
  int _collector_index;
  std::string _text;
  GtkWidget *_widget;
  GdkColor _fg_color;
  GdkColor _bg_color;
  PangoLayout *_layout;

  /*
  COLORREF _bg_color;
  COLORREF _fg_color;
  HBRUSH _bg_brush;
  HBRUSH _highlight_brush;
  */

  bool _highlight;
  bool _mouse_within;
  int _height;

  static int _left_margin, _right_margin;
  static int _top_margin, _bottom_margin;
};

#endif
