/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file gtkStatsGraph.h
 * @author drose
 * @date 2006-01-16
 */

#ifndef GTKSTATSGRAPH_H
#define GTKSTATSGRAPH_H

#include "pandatoolbase.h"
#include "gtkStatsLabelStack.h"
#include "pmap.h"
#include "luse.h"

#include <gtk/gtk.h>
#include <cairo.h>

class GtkStatsMonitor;

/**
 * This is just an abstract base class to provide a common pointer type for
 * the various kinds of graphs that may be created for a GtkStatsMonitor.
 */
class GtkStatsGraph {
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
  GtkStatsGraph(GtkStatsMonitor *monitor, bool has_label_stack);
  virtual ~GtkStatsGraph();

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

  void reset_collector_color(int collector_index);

protected:
  void close();

  void start_animation();
  virtual bool animate(double time, double dt);

  void get_window_state(int &x, int &y, int &width, int &height,
                        bool &maximized, bool &minimized) const;
  void set_window_state(int x, int y, int width, int height,
                        bool maximized, bool minimized);

  cairo_pattern_t *get_collector_pattern(int collector_index, bool highlight = false);
  LRGBColor get_collector_text_color(int collector_index, bool highlight = false);

  virtual void additional_graph_window_paint(cairo_t *cr);
  virtual std::string get_graph_tooltip(int mouse_x, int mouse_y) const;
  virtual DragMode consider_drag_start(int graph_x, int graph_y);
  virtual void set_drag_mode(DragMode drag_mode);

  virtual gboolean handle_button_press(int graph_x, int graph_y,
                                       bool double_click, int button);
  virtual gboolean handle_button_release(int graph_x, int graph_y);
  virtual gboolean handle_motion(int graph_x, int graph_y);
  virtual gboolean handle_leave();

protected:
  // Table of patterns for our various collectors.
  typedef pmap<int, std::pair<cairo_pattern_t *, cairo_pattern_t *> > Brushes;
  Brushes _brushes;

  typedef pmap<int, std::pair<LRGBColor, LRGBColor> > TextColors;
  TextColors _text_colors;

  GtkStatsMonitor *_monitor;
  GtkWidget *_parent_window = nullptr;
  GtkWidget *_window = nullptr;
  GtkWidget *_graph_frame;
  GtkWidget *_graph_window = nullptr;
  GtkWidget *_graph_hbox;
  GtkWidget *_graph_vbox;
  GtkWidget *_hpaned;
  GtkWidget *_scale_area = nullptr;
  GtkStatsLabelStack _label_stack;

  GdkCursor *_hand_cursor;

  cairo_surface_t *_cr_surface;
  cairo_t *_cr;
  int _surface_xsize, _surface_ysize;
  PangoAttrList *_pango_attrs;
  int _cr_scale;
  int _pixel_scale;

  DragMode _drag_mode;
  DragMode _potential_drag_mode;
  int _drag_start_x, _drag_start_y;
  double _drag_scale_start;
  int _drag_guide_bar;

  int _highlighted_index = -1;

  bool _pause;

  guint _timer_id = 0;
  gint64 _time = 0;

  static const double rgb_white[3];
  static const double rgb_light_gray[3];
  static const double rgb_dark_gray[3];
  static const double rgb_black[3];
  static const double rgb_user_guide_bar[3];

private:
  void setup_surface(int xsize, int ysize, int scale);
  void release_surface();

  static gboolean window_delete_event(GtkWidget *widget, GdkEvent *event,
                                      gpointer data);
  static void window_destroy(GtkWidget *widget, gpointer data);
  static gboolean graph_draw_callback(GtkWidget *widget,
                                      cairo_t *cr, gpointer data);
  static gboolean configure_graph_callback(GtkWidget *widget,
                                           GdkEventConfigure *event,
                                           gpointer data);

protected:
  static gboolean button_press_event_callback(GtkWidget *widget,
                                              GdkEventButton *event,
                                              gpointer data);
  static gboolean button_release_event_callback(GtkWidget *widget,
                                                GdkEventButton *event,
                                                gpointer data);
  static gboolean motion_notify_event_callback(GtkWidget *widget,
                                               GdkEventMotion *event,
                                               gpointer data);
  static gboolean leave_notify_event_callback(GtkWidget *widget,
                                              GdkEventCrossing *event,
                                              gpointer data);
  static gboolean query_tooltip_callback(GtkWidget *widget, gint x, gint y,
                                         gboolean keyboard_tip,
                                         GtkTooltip *tooltip, gpointer data);
  static gboolean tick_callback(GtkWidget *widget, GdkFrameClock *clock,
                                gpointer data);
};

#endif
