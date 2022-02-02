/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file gtkStatsFlameGraph.h
 * @author rdb
 * @date 2022-02-02
 */

#ifndef GTKSTATSFLAMEGRAPH_H
#define GTKSTATSFLAMEGRAPH_H

#include "pandatoolbase.h"

#include "gtkStatsGraph.h"
#include "pStatFlameGraph.h"

class GtkStatsLabel;

/**
 * A window that draws a flame chart, which shows the collectors explicitly
 * stopping and starting, one frame at a time.
 */
class GtkStatsFlameGraph : public PStatFlameGraph, public GtkStatsGraph {
public:
  GtkStatsFlameGraph(GtkStatsMonitor *monitor, int thread_index,
                     int collector_index=0);
  virtual ~GtkStatsFlameGraph();

  virtual void new_collector(int collector_index);
  virtual void new_data(int thread_index, int frame_number);
  virtual void force_redraw();
  virtual void changed_graph_size(int graph_xsize, int graph_ysize);

  virtual void set_time_units(int unit_mask);
  virtual void on_click_label(int collector_index);
  virtual void on_enter_label(int collector_index);
  virtual void on_leave_label(int collector_index);
  virtual std::string get_label_tooltip(int collector_index) const;

protected:
  virtual void update_labels();
  virtual void update_label(int collector_index, int row, int x, int width);
  virtual void normal_guide_bars();

  void clear_region();
  virtual void begin_draw();
  virtual void end_draw();
  virtual void idle();

  virtual void additional_graph_window_paint(cairo_t *cr);
  virtual DragMode consider_drag_start(int graph_x, int graph_y);

  virtual gboolean handle_button_press(GtkWidget *widget, int graph_x, int graph_y,
               bool double_click);
  virtual gboolean handle_button_release(GtkWidget *widget, int graph_x, int graph_y);
  virtual gboolean handle_motion(GtkWidget *widget, int graph_x, int graph_y);

private:
  void draw_guide_bar(cairo_t *cr, const PStatGraph::GuideBar &bar);
  void draw_guide_labels(cairo_t *cr);
  void draw_guide_label(cairo_t *cr, const PStatGraph::GuideBar &bar);

  static void toggled_callback(GtkToggleButton *button, gpointer data);
  static gboolean draw_callback(GtkWidget *widget, cairo_t *cr, gpointer data);

private:
  std::string _net_value_text;
  pmap<int, GtkStatsLabel *> _labels;

  GtkWidget *_top_hbox;
  GtkWidget *_average_check_box;
  GtkWidget *_total_label;
  GtkWidget *_fixed;
};

#endif
