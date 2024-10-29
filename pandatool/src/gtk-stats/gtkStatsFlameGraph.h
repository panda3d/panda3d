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
class GtkStatsFlameGraph final : public PStatFlameGraph, public GtkStatsGraph {
public:
  GtkStatsFlameGraph(GtkStatsMonitor *monitor, int thread_index,
                     int collector_index=-1, int frame_number=-1);
  virtual ~GtkStatsFlameGraph();

  virtual void new_collector(int collector_index);
  virtual void new_data(int thread_index, int frame_number);
  virtual void force_redraw();
  virtual void changed_graph_size(int graph_xsize, int graph_ysize);

  virtual void set_time_units(int unit_mask);
  virtual void on_click_label(int collector_index);
  virtual void on_enter_label(int collector_index);
  virtual void on_leave_label(int collector_index);

protected:
  virtual void normal_guide_bars();

  void clear_region();
  virtual void begin_draw();
  virtual void draw_bar(int depth, int from_x, int to_x,
                        int collector_index, int parent_index);
  virtual void end_draw();
  virtual void idle();

  virtual bool animate(double time, double dt);

  virtual bool get_window_state(int &x, int &y, int &width, int &height,
                                bool &maximized, bool &minimized) const;
  virtual void set_window_state(int x, int y, int width, int height,
                                bool maximized, bool minimized);

  virtual void additional_graph_window_paint(cairo_t *cr);
  virtual std::string get_graph_tooltip(int mouse_x, int mouse_y) const;
  virtual DragMode consider_drag_start(int graph_x, int graph_y);

  virtual gboolean handle_button_press(int graph_x, int graph_y,
                                       bool double_click, int button);
  virtual gboolean handle_button_release(int graph_x, int graph_y);
  virtual gboolean handle_motion(int graph_x, int graph_y);
  virtual gboolean handle_leave();

private:
  int pixel_to_depth(int y) const;
  void draw_guide_bar(cairo_t *cr, const PStatGraph::GuideBar &bar);
  void draw_guide_labels(cairo_t *cr);
  void draw_guide_label(cairo_t *cr, const PStatGraph::GuideBar &bar);

  static void toggled_callback(GtkToggleButton *button, gpointer data);
  static gboolean draw_callback(GtkWidget *widget, cairo_t *cr, gpointer data);
  static gboolean scroll_callback(GtkWidget *widget, GdkEventScroll *event, gpointer data);
  static gboolean key_press_callback(GtkWidget *widget, GdkEventKey *event, gpointer data);

private:
  std::string _net_value_text;

  GtkWidget *_top_hbox;
  GtkWidget *_average_check_box;
  GtkWidget *_total_label;

  int _popup_index = -1;
};

#endif
