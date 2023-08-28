/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file macStatsPianoRoll.h
 * @author rdb
 * @date 2023-08-19
 */

#ifndef MACSTATSPIANOROLL_H
#define MACSTATSPIANOROLL_H

#include "macStatsGraph.h"
#include "pStatPianoRoll.h"
#include "macStatsChartMenuDelegate.h"

class MacStatsMonitor;

/**
 * A window that draws a piano-roll style chart, which shows the collectors
 * explicitly stopping and starting, one frame at a time.
 */
class MacStatsPianoRoll final : public PStatPianoRoll, public MacStatsGraph {
public:
  MacStatsPianoRoll(MacStatsMonitor *monitor, int thread_index);
  virtual ~MacStatsPianoRoll();

  virtual void new_data(int thread_index, int frame_number);
  virtual void force_redraw();
  virtual void changed_graph_size(int graph_xsize, int graph_ysize);

  virtual void set_time_units(int unit_mask);
  virtual void on_click_label(int collector_index);
  virtual NSMenu *get_label_menu(int collector_index) const;
  virtual std::string get_label_tooltip(int collector_index) const;
  void set_horizontal_scale(double time_width);

protected:
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

  virtual NSMenu *get_graph_menu(int mouse_x, int mouse_y) const;
  virtual std::string get_graph_tooltip(int mouse_x, int mouse_y) const;
  virtual DragMode consider_drag_start(int graph_x, int graph_y);

  virtual void handle_button_press(int graph_x, int graph_y,
                                   bool double_click, int button);
  virtual void handle_button_release(int graph_x, int graph_y);
  virtual void handle_motion(int graph_x, int graph_y);
  virtual void handle_leave();
  virtual void handle_scroll();
  virtual void handle_magnify(int graph_x, int graph_y, double scale);
  virtual void handle_draw_graph(CGContextRef ctx, NSRect rect);
  virtual void handle_draw_graph_overhang(CGContextRef ctx, NSRect rect);
  virtual void handle_draw_scale_area(CGContextRef ctx, NSRect rect);

private:
  int get_collector_under_pixel(int xpoint, int ypoint) const;
  void update_labels();
  void draw_guide_bars(CGContextRef ctx, int y, int height);
  void draw_guide_bar(CGContextRef ctx, const PStatGraph::GuideBar &bar,
                      int y, int height);
  void draw_guide_labels(CGContextRef ctx);
  void draw_guide_label(CGContextRef ctx, const PStatGraph::GuideBar &bar);

private:
  MacStatsChartMenuDelegate *_menu_delegate;
  NSScrollView *_sidebar_scroll_view;
};

#endif
