/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file winStatsTimeline.h
 * @author rdb
 * @date 2022-02-11
 */

#ifndef WINSTATSTIMELINE_H
#define WINSTATSTIMELINE_H

#include "pandatoolbase.h"

#include "winStatsGraph.h"
#include "pStatTimeline.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>

class WinStatsMonitor;

/**
 * A window that draws all of the start/stop event pairs on each thread on a
 * horizontal scrolling timeline, with concurrent start/stop pairs stacked
 * underneath each other.
 */
class WinStatsTimeline : public PStatTimeline, public WinStatsGraph {
public:
  WinStatsTimeline(WinStatsMonitor *monitor);
  virtual ~WinStatsTimeline();

  virtual void new_data(int thread_index, int frame_number);
  virtual void force_redraw();
  virtual void changed_graph_size(int graph_xsize, int graph_ysize);

protected:
  virtual void clear_region();
  virtual void begin_draw();
  virtual void draw_separator(int row);
  virtual void draw_guide_bar(int x, GuideBarStyle style);
  virtual void draw_bar(int row, int from_x, int to_x, int collector_index,
                        const std::string &collector_name);
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

private:
  void draw_guide_label(HDC hdc, int y, const GuideBar &bar);
  void draw_thread_label(HDC hdc, const ThreadRow &thread_row);

  void create_window();
  static void register_window_class(HINSTANCE application);

  static LONG WINAPI static_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

  int row_to_pixel(int y) const {
    return y * _pixel_scale * 5 + _pixel_scale - _scroll;
  }
  int pixel_to_row(int y) const {
    return (y + _scroll - _pixel_scale) / (_pixel_scale * 5);
  }

  static bool _window_class_registered;
  static const char * const _window_class_name;

  HBRUSH _grid_brush;

  int _highlighted_row = -1;
  int _highlighted_x = 0;
  int _scroll = 0;
  ColorBar _popup_bar;
};

#endif
