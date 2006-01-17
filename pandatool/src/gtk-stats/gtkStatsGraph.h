// Filename: gtkStatsGraph.h
// Created by:  drose (16Jan06)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
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
    DM_left_margin,
    DM_right_margin,
    DM_guide_bar,
    DM_new_guide_bar,
    DM_sizing,
  };

public:
  GtkStatsGraph(GtkStatsMonitor *monitor, int thread_index);
  virtual ~GtkStatsGraph();

  int get_thread_index() const;

  virtual void new_collector(int collector_index);
  virtual void new_data(int thread_index, int frame_number);
  virtual void force_redraw();
  virtual void changed_graph_size(int graph_xsize, int graph_ysize);

  virtual void set_time_units(int unit_mask);
  virtual void set_scroll_speed(float scroll_speed);
  void set_pause(bool pause);

  void user_guide_bars_changed();
  virtual void clicked_label(int collector_index);

protected:
  void close();
  GdkGC *get_collector_gc(int collector_index);

  /*
  virtual void additional_window_paint(HDC hdc);
  virtual void additional_graph_window_paint(HDC hdc);
  */
  virtual DragMode consider_drag_start(int mouse_x, int mouse_y, 
                                       int width, int height);
  virtual void set_drag_mode(DragMode drag_mode);

protected:
  // Table of GC's for our various collectors.
  typedef pmap<int, GdkGC *> Brushes;
  Brushes _brushes;

  GtkStatsMonitor *_monitor;
  int _thread_index;
  GtkWidget *_parent_window;
  GtkWidget *_window;
  GtkWidget *_graph_window;
  GtkStatsLabelStack _label_stack;

  /*
  HCURSOR _sizewe_cursor;
  HCURSOR _hand_cursor;
  */

  GdkPixmap *_pixmap;
  GdkGC *_pixmap_gc;

  int _graph_left, _graph_top;
  int _pixmap_xsize, _pixmap_ysize;
  int _left_margin, _right_margin;
  int _top_margin, _bottom_margin;

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
  float _drag_scale_start;
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
  void create_graph_window();

  static gboolean window_delete_event(GtkWidget *widget, GdkEvent *event, 
				      gpointer data);
  static void window_destroy(GtkWidget *widget, gpointer data);
  static gboolean expose_event_callback(GtkWidget *widget, 
					GdkEventExpose *event, gpointer data);
  static gboolean configure_event_callback(GtkWidget *widget, 
					   GdkEventConfigure *event, gpointer data);
};

#endif

