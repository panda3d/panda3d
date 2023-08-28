/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file macStatsGraph.h
 * @author rdb
 * @date 2023-08-17
 */

#ifndef MACSTATSGRAPH_H
#define MACSTATSGRAPH_H

#include "pandatoolbase.h"
#include "macStatsGraphViewController.h"
#include "macStatsLabelStack.h"
#include "pmap.h"
#include "luse.h"

class MacStatsMonitor;

/**
 * This is just an abstract base class to provide a common pointer type for
 * the various kinds of graphs that may be created for a MacStatsMonitor.
 */
class MacStatsGraph {
public:
  // What is the user adjusting by dragging the mouse in a window?
  enum DragMode {
    DM_none,
    DM_scale,
    DM_guide_bar,
    DM_new_guide_bar,
    DM_sizing,
    DM_pan,
  };

public:
  MacStatsGraph(MacStatsMonitor *monitor, MacStatsGraphViewController *controller);
  virtual ~MacStatsGraph();

  void close();

  MacStatsMonitor *get_monitor() { return _monitor; }
  NSSplitView *get_split_view() { return _split_view; }

  virtual void new_collector(int collector_index);
  virtual void new_data(int thread_index, int frame_number);
  virtual void force_redraw()=0;
  virtual void changed_graph_size(int graph_xsize, int graph_ysize);

  virtual void set_time_units(int unit_mask);
  virtual void set_scroll_speed(double scroll_speed);
  void set_pause(bool pause);

  void user_guide_bars_changed();
  virtual void on_click_label(int collector_index);
  virtual void on_enter_label(int collector_index);
  virtual void on_leave_label(int collector_index);
  virtual NSMenu *get_label_menu(int collector_index) const;
  virtual std::string get_label_tooltip(int collector_index) const;

  void reset_collector_color(int collector_index);

protected:
  void start_animation();
  virtual bool animate(double time, double dt);

  void get_window_state(int &x, int &y, int &width, int &height,
                        bool &maximized, bool &minimized) const;
  void set_window_state(int x, int y, int width, int height,
                        bool maximized, bool minimized);

public:
  // These must be public, because we can't declare Objective-C friends here.
  virtual NSMenu *get_graph_menu(int mouse_x, int mouse_y) const;
  virtual std::string get_graph_tooltip(int mouse_x, int mouse_y) const;
  virtual DragMode consider_drag_start(int graph_x, int graph_y);
  virtual void set_drag_mode(DragMode drag_mode);

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
  virtual void handle_back();
  virtual void handle_timer();

protected:
  CGColorRef _background_color;

  MacStatsMonitor *_monitor;
  NSWindow *_window;
  NSView *_graph_view;
  MacStatsGraphViewController *_graph_view_controller;
  NSView *_scale_area = nullptr;
  NSSplitView *_split_view = nullptr;
  NSScrollView *_sidebar_view = nullptr;
  MacStatsLabelStack _label_stack;

  CGContextRef _ctx = nullptr;
  int _bitmap_xsize, _bitmap_ysize;

  DragMode _drag_mode;
  DragMode _potential_drag_mode;
  int _drag_start_x, _drag_start_y;
  double _drag_scale_start;
  int _drag_guide_bar;

  int _highlighted_index = -1;

  bool _pause;

  NSTimer *_animation_timer = nil;
  double _time = 0.0;

  static const CGFloat rgb_white[4];
  static const CGFloat rgb_light_gray[4];
  static const CGFloat rgb_dark_gray[4];
  static const CGFloat rgb_black[4];
  static const CGFloat rgb_user_guide_bar[4];

private:
  void setup_bitmap(int xsize, int ysize, double scale);
  void release_bitmap();
};

#endif
