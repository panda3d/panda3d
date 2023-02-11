/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file winStatsPianoRoll.h
 * @author drose
 * @date 2004-01-12
 */

#ifndef WINSTATSPIANOROLL_H
#define WINSTATSPIANOROLL_H

#include "pandatoolbase.h"

#include "winStatsGraph.h"
#include "pStatPianoRoll.h"
#include "pointerTo.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>

class WinStatsMonitor;

/**
 * A window that draws a piano-roll style chart, which shows the collectors
 * explicitly stopping and starting, one frame at a time.
 */
class WinStatsPianoRoll : public PStatPianoRoll, public WinStatsGraph {
public:
  WinStatsPianoRoll(WinStatsMonitor *monitor, int thread_index);
  virtual ~WinStatsPianoRoll();

  virtual void new_data(int thread_index, int frame_number);
  virtual void force_redraw();
  virtual void changed_graph_size(int graph_xsize, int graph_ysize);

  virtual void set_time_units(int unit_mask);
  virtual void on_click_label(int collector_index);
  virtual void on_popup_label(int collector_index);
  virtual std::string get_label_tooltip(int collector_index) const;
  void set_horizontal_scale(double time_width);

protected:
  virtual void normal_guide_bars();
  void clear_region();
  virtual void begin_draw();
  virtual void begin_row(int row);
  virtual void draw_bar(int row, int from_x, int to_x);
  virtual void end_draw();
  virtual void idle();

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
  int get_collector_under_pixel(int xpoint, int ypoint) const;
  void update_labels();
  void draw_guide_bar(HDC hdc, const GuideBar &bar);
  void draw_guide_label(HDC hdc, int y, const PStatGraph::GuideBar &bar);

  void create_window();
  static void register_window_class(HINSTANCE application);

  static LONG WINAPI static_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

  int _popup_index = -1;

  static bool _window_class_registered;
  static const char * const _window_class_name;
};

#endif
