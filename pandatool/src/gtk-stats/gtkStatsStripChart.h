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
class GtkStatsStripChart final : public PStatStripChart, public GtkStatsGraph {
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
  virtual void on_click_label(int collector_index);
  virtual void on_popup_label(int collector_index);
  virtual std::string get_label_tooltip(int collector_index) const;
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

  virtual bool get_window_state(int &x, int &y, int &width, int &height,
                                bool &maximized, bool &minimized) const;
  virtual void set_window_state(int x, int y, int width, int height,
                                bool maximized, bool minimized);

  virtual void additional_graph_window_paint(cairo_t *cr);
  virtual std::string get_graph_tooltip(int mouse_x, int mouse_y) const;
  virtual DragMode consider_drag_start(int graph_x, int graph_y);
  virtual void set_drag_mode(DragMode drag_mode);

  virtual gboolean handle_button_press(int graph_x, int graph_y,
                                       bool double_click, int button);
  virtual gboolean handle_button_release(int graph_x, int graph_y);
  virtual gboolean handle_motion(int graph_x, int graph_y);
  virtual gboolean handle_leave();

private:
  void draw_guide_bar(cairo_t *cr, int from_x, int to_x,
                      const PStatGraph::GuideBar &bar);
  void draw_guide_labels(cairo_t *cr);
  int draw_guide_label(cairo_t *cr, const PStatGraph::GuideBar &bar, int last_y);

  static void toggled_callback(GtkToggleButton *button, gpointer data);
  static gboolean draw_callback(GtkWidget *widget, cairo_t *cr, gpointer data);

private:
  std::string _net_value_text;

  GtkWidget *_top_hbox;
  GtkWidget *_smooth_check_box;
  GtkWidget *_total_label;

  int _popup_index = -1;
};

#endif
