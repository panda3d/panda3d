/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file winStatsFlameGraph.h
 * @author rdb
 * @date 2022-01-28
 */

#ifndef WINSTATSFLAMEGRAPH_H
#define WINSTATSFLAMEGRAPH_H

#include "pandatoolbase.h"

#include "winStatsGraph.h"
#include "pStatFlameGraph.h"

class WinStatsLabel;

/**
 * A window that draws a flame chart, which shows the collectors explicitly
 * stopping and starting, one frame at a time.
 */
class WinStatsFlameGraph : public PStatFlameGraph, public WinStatsGraph {
public:
  WinStatsFlameGraph(WinStatsMonitor *monitor, int thread_index,
                     int collector_index=-1, int frame_number=-1);
  virtual ~WinStatsFlameGraph();

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

  LONG window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
  virtual LONG graph_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
  virtual void additional_window_paint(HDC hdc);
  virtual void additional_graph_window_paint(HDC hdc);
  virtual std::string get_graph_tooltip(int mouse_x, int mouse_y) const;
  virtual DragMode consider_drag_start(int mouse_x, int mouse_y,
                                       int width, int height);
  virtual void move_graph_window(int graph_left, int graph_top,
                                 int graph_xsize, int graph_ysize);

private:
  int pixel_to_depth(int y) const;
  void draw_guide_bar(HDC hdc, const GuideBar &bar);
  void draw_guide_label(HDC hdc, int y, const PStatGraph::GuideBar &bar);
  void create_window();
  static void register_window_class(HINSTANCE application);

  static LONG WINAPI static_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

  std::string _net_value_text;
  HWND _average_check_box;
  int _popup_index = -1;

  static bool _window_class_registered;
  static const char * const _window_class_name;
};

#endif
