/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file gtkStatsStripChart.h
 * @author drose
 * @date 2006-01-16
 */

#ifndef GTKSTATSSTRIPCHART_H
#define GTKSTATSSTRIPCHART_H

#include "pandatoolbase.h"

#include "gtkStatsGraph.h"
#include "pStatStripChart.h"
#include "pointerTo.h"

#include <gtk/gtk.h>

class GtkStatsMonitor;

/**
 * A window that draws a strip chart, given a view.
 */
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
  virtual void set_scroll_speed(double scroll_speed);
  virtual void clicked_label(int collector_index);
  void set_vertical_scale(double value_height);

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

  virtual gboolean handle_button_press(GtkWidget *widget, int graph_x, int graph_y,
               bool double_click);
  virtual gboolean handle_button_release(GtkWidget *widget, int graph_x, int graph_y);
  virtual gboolean handle_motion(GtkWidget *widget, int graph_x, int graph_y);

private:
  void draw_guide_bar(GdkDrawable *surface, int from_x, int to_x,
          const PStatGraph::GuideBar &bar);
  void draw_guide_labels();
  int draw_guide_label(const PStatGraph::GuideBar &bar, int last_y);

  static void toggled_callback(GtkToggleButton *button, gpointer data);
  static gboolean expose_event_callback(GtkWidget *widget,
          GdkEventExpose *event, gpointer data);

private:
  int _brush_origin;
  std::string _net_value_text;

  GtkWidget *_top_hbox;
  GtkWidget *_smooth_check_box;
  GtkWidget *_total_label;
};

#endif
