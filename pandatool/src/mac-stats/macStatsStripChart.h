/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file macStatsStripChart.h
 * @author rdb
 * @date 2023-08-18
 */

#ifndef MACSTATSSTRIPCHART_H
#define MACSTATSSTRIPCHART_H

#include "macStatsGraph.h"
#include "pStatStripChart.h"
#include "macStatsChartMenuDelegate.h"

class MacStatsMonitor;

/**
 * A window that draws a strip chart, given a view.
 */
class MacStatsStripChart final : public PStatStripChart, public MacStatsGraph {
public:
  MacStatsStripChart(MacStatsMonitor *monitor,
                     int thread_index, int collector_index, bool show_level);
  virtual ~MacStatsStripChart();

  virtual void new_collector(int collector_index);
  virtual void new_data(int thread_index, int frame_number);
  virtual void force_redraw();
  virtual void changed_graph_size(int graph_xsize, int graph_ysize);

  virtual void set_time_units(int unit_mask);
  virtual void set_scroll_speed(double scroll_speed);
  virtual void on_click_label(int collector_index);
  virtual NSMenu *get_label_menu(int collector_index) const;
  virtual std::string get_label_tooltip(int collector_index) const;
  void set_vertical_scale(double value_height);
  void set_auto_vertical_scale();

protected:
  virtual void update_labels();

  virtual void clear_region();
  virtual void draw_slice(int x, int w,
                          const PStatStripChart::FrameData &fdata);
  virtual void draw_empty(int x, int w);
  virtual void draw_cursor(int x);
  virtual void end_draw(int from_x, int to_x);

  virtual bool get_window_state(int &x, int &y, int &width, int &height,
                                bool &maximized, bool &minimized) const;
  virtual void set_window_state(int x, int y, int width, int height,
                                bool maximized, bool minimized);

  virtual NSMenu *get_graph_menu(int mouse_x, int mouse_y) const;
  virtual std::string get_graph_tooltip(int mouse_x, int mouse_y) const;
  virtual DragMode consider_drag_start(int graph_x, int graph_y);
  virtual void set_drag_mode(DragMode drag_mode);

  virtual void handle_button_press(int graph_x, int graph_y,
                                   bool double_click, int button);
  virtual void handle_button_release(int graph_x, int graph_y);
  virtual void handle_motion(int graph_x, int graph_y);
  virtual void handle_leave();
  virtual void handle_magnify(int graph_x, int graph_y, double scale);
  virtual void handle_draw_graph(CGContextRef ctx, NSRect rect);
  virtual void handle_draw_scale_area(CGContextRef ctx, NSRect rect);
  virtual void handle_back();

private:
  void draw_guide_bar(CGContextRef ctx, int from_x, int to_x,
                      const PStatGraph::GuideBar &bar);
  void draw_guide_labels(CGContextRef ctx);
  int draw_guide_label(CGContextRef ctx, const PStatGraph::GuideBar &bar, int last_y);

private:
  MacStatsChartMenuDelegate *_menu_delegate;
  //NSButton *_smooth_checkbox;
  //NSTextField *_total_label;
  NSToolbarItem *_total_item;

  std::vector<int> _back_stack;
};

#endif
