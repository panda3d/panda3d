/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file macStatsFlameGraph.h
 * @author rdb
 * @date 2023-08-18
 */

#ifndef MACSTATSFLAMEGRAPH_H
#define MACSTATSFLAMEGRAPH_H

#include "macStatsGraph.h"
#include "pStatFlameGraph.h"
#include "macStatsChartMenuDelegate.h"

/**
 * A window that draws a flame chart, which shows the collectors explicitly
 * stopping and starting, one frame at a time.
 */
class MacStatsFlameGraph final : public PStatFlameGraph, public MacStatsGraph {
public:
  MacStatsFlameGraph(MacStatsMonitor *monitor, int thread_index,
                     int collector_index=-1, int frame_number=-1);
  virtual ~MacStatsFlameGraph();

  virtual void new_collector(int collector_index);
  virtual void new_data(int thread_index, int frame_number);
  virtual void force_redraw();
  virtual void changed_graph_size(int graph_xsize, int graph_ysize);

  virtual void set_time_units(int unit_mask);
  virtual void on_click_label(int collector_index);
  virtual void on_enter_label(int collector_index);
  virtual void on_leave_label(int collector_index);
  virtual NSMenu *get_label_menu(int collector_index) const;

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

  virtual NSMenu *get_graph_menu(int mouse_x, int mouse_y) const;
  virtual std::string get_graph_tooltip(int mouse_x, int mouse_y) const;
  virtual DragMode consider_drag_start(int graph_x, int graph_y);

  virtual bool handle_key(int graph_x, int graph_y, bool pressed,
                          UniChar c, unsigned short key_code);
  virtual void handle_button_press(int graph_x, int graph_y,
                                   bool double_click, int button);
  virtual void handle_button_release(int graph_x, int graph_y);
  virtual void handle_motion(int graph_x, int graph_y);
  virtual void handle_leave();
  virtual void handle_wheel(int graph_x, int graph_y, double dx, double dy);
  virtual void handle_draw_graph(CGContextRef ctx, NSRect rect);
  virtual void handle_back();

public:
  void handle_toggle_average(bool state);

private:
  int pixel_to_depth(int y) const;
  void draw_guide_bar(CGContextRef ctx, const PStatGraph::GuideBar &bar);
  void draw_guide_labels(CGContextRef ctx);
  void draw_guide_label(CGContextRef ctx, const PStatGraph::GuideBar &bar);

private:
  NSToolbarItem *_total_item;

  MacStatsChartMenuDelegate *_menu_delegate;
};

#endif
