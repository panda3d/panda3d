/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file gtkStatsGraph.h
 * @author drose
 * @date 2006-01-16
 */

#ifndef GTKSTATSGRAPH_H
#define GTKSTATSGRAPH_H

#include "pandatoolbase.h"
#include "gtkStatsLabelStack.h"
#include "pmap.h"

#include <gtk/gtk.h>
#include <cairo.h>

class GtkStatsMonitor;

/**
 * This is just an abstract base class to provide a common pointer type for
 * the various kinds of graphs that may be created for a GtkStatsMonitor.
 */
class GtkStatsGraph {
public:
  // What is the user adjusting by dragging the mouse in a window?
  enum DragMode {
    DM_none,
    DM_scale,
    DM_guide_bar,
    DM_new_guide_bar,
    DM_sizing,
  };

public:
  GtkStatsGraph(GtkStatsMonitor *monitor);
  virtual ~GtkStatsGraph();

  virtual void new_collector(int collector_index);
  virtual void new_data(int thread_index, int frame_number);
  virtual void force_redraw()=0;
  virtual void changed_graph_size(int graph_xsize, int graph_ysize);

  virtual void set_time_units(int unit_mask);
  virtual void set_scroll_speed(double scroll_speed);
  void set_pause(bool pause);

  void user_guide_bars_changed();
  virtual void on_click_label(int collector_index);
  virtual void on_enter_label(int collector_index);
  virtual void on_leave_label(int collector_index);
  virtual std::string get_label_tooltip(int collector_index) const;

protected:
  void close();
  cairo_pattern_t *get_collector_pattern(int collector_index, bool highlight = false);

  virtual void additional_graph_window_paint(cairo_t *cr);
  virtual DragMode consider_drag_start(int graph_x, int graph_y);
  virtual void set_drag_mode(DragMode drag_mode);

  virtual gboolean handle_button_press(GtkWidget *widget, int graph_x, int graph_y,
               bool double_click);
  virtual gboolean handle_button_release(GtkWidget *widget, int graph_x, int graph_y);
  virtual gboolean handle_motion(GtkWidget *widget, int graph_x, int graph_y);

protected:
  // Table of patterns for our various collectors.
  typedef pmap<int, std::pair<cairo_pattern_t *, cairo_pattern_t *> > Brushes;
  Brushes _brushes;

  GtkStatsMonitor *_monitor;
  GtkWidget *_parent_window;
  GtkWidget *_window;
  GtkWidget *_graph_frame;
  GtkWidget *_graph_overlay;
  GtkWidget *_graph_window;
  GtkWidget *_graph_hbox;
  GtkWidget *_graph_vbox;
  GtkWidget *_hpaned;
  GtkWidget *_scale_area;
  GtkStatsLabelStack _label_stack;

  GdkCursor *_hand_cursor;

  cairo_surface_t *_cr_surface;
  cairo_t *_cr;
  int _surface_xsize, _surface_ysize;

  DragMode _drag_mode;
  DragMode _potential_drag_mode;
  int _drag_start_x, _drag_start_y;
  double _drag_scale_start;
  int _drag_guide_bar;

  int _highlighted_index = -1;

  bool _pause;

  static const double rgb_white[3];
  static const double rgb_light_gray[3];
  static const double rgb_dark_gray[3];
  static const double rgb_black[3];
  static const double rgb_user_guide_bar[3];

private:
  void setup_surface(int xsize, int ysize);
  void release_surface();

  static gboolean window_delete_event(GtkWidget *widget, GdkEvent *event,
              gpointer data);
  static void window_destroy(GtkWidget *widget, gpointer data);
  static gboolean graph_draw_callback(GtkWidget *widget,
              cairo_t *cr, gpointer data);
  static gboolean configure_graph_callback(GtkWidget *widget,
              GdkEventConfigure *event, gpointer data);

protected:
  static gboolean button_press_event_callback(GtkWidget *widget,
                GdkEventButton *event,
                gpointer data);
  static gboolean button_release_event_callback(GtkWidget *widget,
            GdkEventButton *event,
            gpointer data);
  static gboolean motion_notify_event_callback(GtkWidget *widget,
                 GdkEventMotion *event,
                 gpointer data);
};

#endif
