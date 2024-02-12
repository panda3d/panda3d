/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file macStatsTimeline.h
 * @author rdb
 * @date 2023-08-19
 */

#ifndef MACSTATSTIMELINE_H
#define MACSTATSTIMELINE_H

#include "macStatsGraph.h"
#include "pStatTimeline.h"

class MacStatsMonitor;

/**
 * A window that draws all of the start/stop event pairs on each thread on a
 * horizontal scrolling timeline, with concurrent start/stop pairs stacked
 * underneath each other.
 */
class MacStatsTimeline final : public PStatTimeline, public MacStatsGraph {
public:
  MacStatsTimeline(MacStatsMonitor *monitor);
  virtual ~MacStatsTimeline();

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
  virtual void handle_scroll();
  virtual void handle_wheel(int graph_x, int graph_y, double dx, double dy);
  virtual void handle_magnify(int graph_x, int graph_y, double scale);
  virtual void handle_draw_graph(CGContextRef ctx, NSRect rect);
  virtual void handle_draw_graph_overhang(CGContextRef ctx, NSRect rect);
  virtual void handle_draw_scale_area(CGContextRef ctx, NSRect rect);

public:
  void handle_zoom_to();
  void handle_open_strip_chart();
  void handle_open_flame_graph();
  void handle_open_piano_roll();

private:
  void draw_guide_bar(CGContextRef ctx, GuideBarStyle style, int x, int y, int height);
  void draw_guide_labels(CGContextRef ctx);
  void draw_guide_label(CGContextRef ctx, const GuideBar &bar);

  int row_to_pixel(int row) const {
    return row * 4 * 5 + 4 - _scroll;
  }
  int pixel_to_row(int y) const {
    return (y + _scroll - 4) / (4 * 5);
  }

  NSView *_thread_area;
  pvector<std::pair<NSTextField *, NSLayoutConstraint *> > _thread_labels;
  NSLayoutConstraint *_graph_height_constraint;
  NSScrollView *_sidebar_scroll_view;

  int _highlighted_row = -1;
  int _highlighted_x = 0;
  int _scroll = 0;
  mutable ColorBar _popup_bar;
};

#endif
