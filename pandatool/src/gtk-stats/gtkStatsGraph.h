// Filename: gtkStatsGraph.h
// Created by:  drose (16Jan06)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef GTKSTATSGRAPH_H
#define GTKSTATSGRAPH_H

#include "pandatoolbase.h"
#include "gtkStatsLabelStack.h"
#include "pmap.h"

#include <gtk/gtk.h>

class GtkStatsMonitor;

////////////////////////////////////////////////////////////////////
//       Class : GtkStatsGraph
// Description : This is just an abstract base class to provide a
//               common pointer type for the various kinds of graphs
//               that may be created for a GtkStatsMonitor.
////////////////////////////////////////////////////////////////////
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
  virtual void force_redraw();
  virtual void changed_graph_size(int graph_xsize, int graph_ysize);

  virtual void set_time_units(int unit_mask);
  virtual void set_scroll_speed(double scroll_speed);
  void set_pause(bool pause);

  void user_guide_bars_changed();
  virtual void clicked_label(int collector_index);

protected:
  void close();
  GdkGC *get_collector_gc(int collector_index);

  virtual void additional_graph_window_paint();
  virtual DragMode consider_drag_start(int graph_x, int graph_y);
  virtual void set_drag_mode(DragMode drag_mode);

  virtual gboolean handle_button_press(GtkWidget *widget, int graph_x, int graph_y,
				       bool double_click);
  virtual gboolean handle_button_release(GtkWidget *widget, int graph_x, int graph_y);
  virtual gboolean handle_motion(GtkWidget *widget, int graph_x, int graph_y);

protected:
  // Table of GC's for our various collectors.
  typedef pmap<int, GdkGC *> Brushes;
  Brushes _brushes;

  GtkStatsMonitor *_monitor;
  GtkWidget *_parent_window;
  GtkWidget *_window;
  GtkWidget *_graph_window;
  GtkWidget *_graph_hbox;
  GtkWidget *_graph_vbox;
  GtkWidget *_hpaned;
  GtkWidget *_scale_area;
  GtkStatsLabelStack _label_stack;

  GdkCursor *_hand_cursor;

  GdkPixmap *_pixmap;
  GdkGC *_pixmap_gc;
  int _pixmap_xsize, _pixmap_ysize;

  /*
  COLORREF _dark_color;
  COLORREF _light_color;
  COLORREF _user_guide_bar_color;
  HPEN _dark_pen;
  HPEN _light_pen;
  HPEN _user_guide_bar_pen;
  */

  DragMode _drag_mode;
  DragMode _potential_drag_mode;
  int _drag_start_x, _drag_start_y;
  double _drag_scale_start;
  int _drag_guide_bar;

  bool _pause;

  static const GdkColor rgb_white;
  static const GdkColor rgb_light_gray;
  static const GdkColor rgb_dark_gray;
  static const GdkColor rgb_black;
  static const GdkColor rgb_user_guide_bar;

private:
  void setup_pixmap(int xsize, int ysize);
  void release_pixmap();

  static gboolean window_delete_event(GtkWidget *widget, GdkEvent *event, 
				      gpointer data);
  static void window_destroy(GtkWidget *widget, gpointer data);
  static gboolean graph_expose_callback(GtkWidget *widget, 
					GdkEventExpose *event, gpointer data);
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

