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
  virtual void clicked_label(int collector_index);
  void set_horizontal_scale(double time_width);

protected:
  void clear_region();
  virtual void begin_draw();
  virtual void draw_bar(int row, int from_x, int to_x);
  virtual void end_draw();
  virtual void idle();

  LONG window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
  virtual LONG graph_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
  virtual void additional_window_paint(HDC hdc);
  virtual void additional_graph_window_paint(HDC hdc);
  virtual DragMode consider_drag_start(int mouse_x, int mouse_y,
                                       int width, int height);

private:
  int get_collector_under_pixel(int xpoint, int ypoint);
  void update_labels();
  void draw_guide_bar(HDC hdc, const GuideBar &bar);
  void draw_guide_label(HDC hdc, int y, const PStatGraph::GuideBar &bar);

  void create_window();
  static void register_window_class(HINSTANCE application);

  static LONG WINAPI static_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

  static bool _window_class_registered;
  static const char * const _window_class_name;
};

#endif
