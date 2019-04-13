/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file winStatsLabelStack.h
 * @author drose
 * @date 2004-01-07
 */

#ifndef WINSTATSLABELSTACK_H
#define WINSTATSLABELSTACK_H

#include "pandatoolbase.h"
#include "pvector.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>

class WinStatsLabel;
class WinStatsMonitor;
class WinStatsGraph;

/**
 * A window that contains a stack of labels from bottom to top.
 */
class WinStatsLabelStack {
public:
  WinStatsLabelStack();
  ~WinStatsLabelStack();

  void setup(HWND parent_window);
  bool is_setup() const;
  void set_pos(int x, int y, int width, int height);

  int get_x() const;
  int get_y() const;
  int get_width() const;
  int get_height() const;
  int get_ideal_width() const;

  int get_label_y(int label_index) const;
  int get_label_height(int label_index) const;
  int get_label_collector_index(int label_index) const;

  void clear_labels();
  int add_label(WinStatsMonitor *monitor, WinStatsGraph *graph,
                int thread_index, int collector_index, bool use_fullname);
  int get_num_labels() const;

  void highlight_label(int collector_index);

private:
  void create_window(HWND parent_window);
  static void register_window_class(HINSTANCE application);

  static LONG WINAPI static_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
  LONG window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

  HWND _window;
  int _x;
  int _y;
  int _width;
  int _height;
  int _ideal_width;
  int _highlight_label;

  typedef pvector<WinStatsLabel *> Labels;
  Labels _labels;

  static bool _window_class_registered;
  static const char * const _window_class_name;
};

#endif
