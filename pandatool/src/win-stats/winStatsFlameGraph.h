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
                     int collector_index=0);
  virtual ~WinStatsFlameGraph();

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

  LONG window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
  virtual LONG graph_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
  virtual void additional_window_paint(HDC hdc);
  virtual void additional_graph_window_paint(HDC hdc);
  virtual DragMode consider_drag_start(int mouse_x, int mouse_y,
                                       int width, int height);
  virtual void move_graph_window(int graph_left, int graph_top,
                                 int graph_xsize, int graph_ysize);

private:
  void draw_guide_bar(HDC hdc, const GuideBar &bar);
  void draw_guide_label(HDC hdc, int y, const PStatGraph::GuideBar &bar);
  void create_window();
  static void register_window_class(HINSTANCE application);

  static LONG WINAPI static_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

  std::string _net_value_text;
  pmap<int, WinStatsLabel *> _labels;

  HWND _average_check_box;

  static bool _window_class_registered;
  static const char * const _window_class_name;
};

#endif
