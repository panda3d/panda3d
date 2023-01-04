/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file winStatsGraph.h
 * @author drose
 * @date 2003-12-03
 */

#ifndef WINSTATSGRAPH_H
#define WINSTATSGRAPH_H

#include "pandatoolbase.h"
#include "winStatsLabelStack.h"
#include "pmap.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>

class PStatGraph;
class WinStatsMonitor;

/**
 * This is just an abstract base class to provide a common pointer type for
 * the various kinds of graphs that may be created for a WinStatsMonitor.
 */
class WinStatsGraph {
public:
  // What is the user adjusting by dragging the mouse in a window?
  enum DragMode {
    DM_none,
    DM_scale,
    DM_left_margin,
    DM_right_margin,
    DM_guide_bar,
    DM_new_guide_bar,
    DM_sizing,
    DM_pan,
  };

public:
  WinStatsGraph(WinStatsMonitor *monitor);
  virtual ~WinStatsGraph();

  virtual void new_collector(int collector_index);
  virtual void new_data(int thread_index, int frame_number);
  virtual void force_redraw()=0;
  virtual void changed_graph_size(int graph_xsize, int graph_ysize);

  virtual void set_time_units(int unit_mask);
  virtual void set_scroll_speed(double scroll_speed);
  void set_pause(bool pause);

  void user_guide_bars_changed();
  virtual void on_click_label(int collector_index);
  virtual void on_popup_label(int collector_index);
  virtual void on_enter_label(int collector_index);
  virtual void on_leave_label(int collector_index);
  virtual std::string get_label_tooltip(int collector_index) const;

  void clear_graph_tooltip();

  HWND get_window();

  void reset_collector_color(int collector_index);

protected:
  void close();

  void setup_label_stack();
  void move_label_stack();

  void start_animation();
  virtual bool animate(double time, double dt);

  void get_window_state(int &x, int &y, int &width, int &height,
                        bool &maximized, bool &minimized) const;
  void set_window_state(int x, int y, int width, int height,
                        bool maximized, bool minimized);

  HBRUSH get_collector_brush(int collector_index, bool highlight = false);
  COLORREF get_collector_text_color(int collector_index, bool highlight = false);

  LONG window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
  virtual LONG graph_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

  virtual void additional_window_paint(HDC hdc);
  virtual void additional_graph_window_paint(HDC hdc);
  virtual std::string get_graph_tooltip(int mouse_x, int mouse_y) const;
  virtual DragMode consider_drag_start(int mouse_x, int mouse_y,
                                       int width, int height);
  virtual void set_drag_mode(DragMode drag_mode);

  virtual void move_graph_window(int graph_left, int graph_top,
                                 int graph_xsize, int graph_ysize);

protected:
  // Table of brushes for our various collectors.
  typedef pmap<int, std::pair<HBRUSH, HBRUSH> > Brushes;
  Brushes _brushes;

  typedef pmap<int, std::pair<COLORREF, COLORREF> > TextColors;
  TextColors _text_colors;

  WinStatsMonitor *_monitor;
  HWND _window;
  HWND _graph_window;
  HWND _tooltip_window;
  WinStatsLabelStack _label_stack;
  std::string _tooltip_text;

  HCURSOR _sizewe_cursor;
  HCURSOR _hand_cursor;

  HBITMAP _bitmap;
  HDC _bitmap_dc;

  int _graph_left, _graph_top;
  int _bitmap_xsize, _bitmap_ysize;
  int _left_margin, _right_margin;
  int _top_margin, _bottom_margin;
  int _top_label_stack_margin;
  int _pixel_scale;

  COLORREF _dark_color;
  COLORREF _light_color;
  COLORREF _user_guide_bar_color;
  COLORREF _frame_guide_bar_color;
  HPEN _dark_pen;
  HPEN _light_pen;
  HPEN _user_guide_bar_pen;
  HPEN _frame_guide_bar_pen;

  DragMode _drag_mode;
  DragMode _potential_drag_mode;
  int _drag_start_x, _drag_start_y;
  double _drag_scale_start;
  int _drag_guide_bar;

  int _highlighted_index = -1;

  bool _pause;

  bool _timer_running = false;
  double _time;

private:
  void setup_bitmap(int xsize, int ysize);
  void release_bitmap();
  void create_graph_window();

  static LRESULT WINAPI static_graph_subclass_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam, UINT_PTR subclass, DWORD_PTR ref_data);

protected:
  static DWORD graph_window_style;
};

#endif
