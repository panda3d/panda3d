/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file winStatsLabel.h
 * @author drose
 * @date 2004-01-07
 */

#ifndef WINSTATSLABEL_H
#define WINSTATSLABEL_H

#include "pandatoolbase.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>

class WinStatsMonitor;
class WinStatsGraph;

/**
 * A text label that will draw in color appropriate for a particular
 * collector.  It also responds when the user double-clicks on it.  This is
 * handy for putting colored labels on strip charts.
 */
class WinStatsLabel {
public:
  WinStatsLabel(WinStatsMonitor *monitor, WinStatsGraph *graph,
                int thread_index, int collector_index, bool use_fullname);
  ~WinStatsLabel();

  void setup(HWND parent_window);
  void set_pos(int x, int y, int width);

  int get_x() const;
  int get_y() const;
  int get_width() const;
  int get_height() const;
  int get_ideal_width() const;

  int get_collector_index() const;

  void set_highlight(bool highlight);
  bool get_highlight() const;

private:
  void set_mouse_within(bool mouse_within);

  void create_window(HWND parent_window);
  static void register_window_class(HINSTANCE application);

  static LONG WINAPI static_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
  LONG window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

  WinStatsMonitor *_monitor;
  WinStatsGraph *_graph;
  int _thread_index;
  int _collector_index;
  std::string _text;
  HWND _window;
  COLORREF _bg_color;
  COLORREF _fg_color;
  HBRUSH _bg_brush;
  HBRUSH _highlight_brush;

  int _x;
  int _y;
  int _width;
  int _height;
  int _ideal_width;
  bool _highlight;
  bool _mouse_within;

  static int _left_margin, _right_margin;
  static int _top_margin, _bottom_margin;

  static bool _window_class_registered;
  static const char * const _window_class_name;
};

#endif
