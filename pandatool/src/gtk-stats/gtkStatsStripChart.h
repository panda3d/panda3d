// Filename: gtkStatsStripChart.h
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

#ifndef GTKSTATSSTRIPCHART_H
#define GTKSTATSSTRIPCHART_H

#include "pandatoolbase.h"

#include "gtkStatsGraph.h"
#include "pStatStripChart.h"
#include "pointerTo.h"

#include <gtk/gtk.h>

class GtkStatsMonitor;

////////////////////////////////////////////////////////////////////
//       Class : GtkStatsStripChart
// Description : A window that draws a strip chart, given a view.
////////////////////////////////////////////////////////////////////
class GtkStatsStripChart : public PStatStripChart, public GtkStatsGraph {
public:
  GtkStatsStripChart(GtkStatsMonitor *monitor,
                     int thread_index, int collector_index, bool show_level);
  virtual ~GtkStatsStripChart();

  virtual void new_collector(int collector_index);
  virtual void new_data(int thread_index, int frame_number);
  virtual void force_redraw();
  virtual void changed_graph_size(int graph_xsize, int graph_ysize);

  virtual void set_time_units(int unit_mask);
  virtual void set_scroll_speed(float scroll_speed);
  virtual void clicked_label(int collector_index);
  void set_vertical_scale(float value_height);

protected:
  virtual void update_labels();

  virtual void clear_region();
  virtual void copy_region(int start_x, int end_x, int dest_x);
  virtual void draw_slice(int x, int w, 
                          const PStatStripChart::FrameData &fdata);
  virtual void draw_empty(int x, int w);
  virtual void draw_cursor(int x);
  virtual void end_draw(int from_x, int to_x);

  virtual void additional_graph_window_paint();
  virtual DragMode consider_drag_start(int graph_x, int graph_y);
  virtual void set_drag_mode(DragMode drag_mode);

  virtual gboolean handle_button_press(GtkWidget *widget, int graph_x, int graph_y);
  virtual gboolean handle_button_release(GtkWidget *widget, int graph_x, int graph_y);
  virtual gboolean handle_motion(GtkWidget *widget, int graph_x, int graph_y);

private:
  void draw_guide_bar(GdkDrawable *surface, int from_x, int to_x, 
		      const PStatGraph::GuideBar &bar);
  void draw_guide_labels();
  int draw_guide_label(const PStatGraph::GuideBar &bar, int last_y);
  
  static gboolean expose_event_callback(GtkWidget *widget, 
					GdkEventExpose *event, gpointer data);

private:

  int _brush_origin;
  string _net_value_text;

  GtkWidget *_smooth_check_box;

  static bool _window_class_registered;
  static const char * const _window_class_name;
};

#endif

