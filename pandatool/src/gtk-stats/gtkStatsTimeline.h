/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file gtkStatsTimeline.h
 * @author rdb
 * @date 2022-02-17
 */

#ifndef GTKSTATSTIMELINE_H
#define GTKSTATSTIMELINE_H

#include "pandatoolbase.h"

#include "gtkStatsGraph.h"
#include "pStatTimeline.h"

class GtkStatsMonitor;

/**
 * A window that draws all of the start/stop event pairs on each thread on a
 * horizontal scrolling timeline, with concurrent start/stop pairs stacked
 * underneath each other.
 */
class GtkStatsTimeline final : public PStatTimeline, public GtkStatsGraph {
public:
  GtkStatsTimeline(GtkStatsMonitor *monitor);
  virtual ~GtkStatsTimeline();

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

  virtual void additional_graph_window_paint(cairo_t *cr);
  virtual std::string get_graph_tooltip(int mouse_x, int mouse_y) const;
  virtual DragMode consider_drag_start(int graph_x, int graph_y);

  virtual gboolean handle_button_press(int graph_x, int graph_y,
                                       bool double_click, int button);
  virtual gboolean handle_button_release(int graph_x, int graph_y);
  virtual gboolean handle_motion(int graph_x, int graph_y);
  virtual gboolean handle_leave();
  gboolean handle_scroll(int graph_x, int graph_y,
                         double dx, double dy, bool ctrl_held);
  gboolean handle_zoom(int graph_x, int graph_y, double scale);
  gboolean handle_key(bool pressed, guint val, guint16 hw_code);

private:
  void draw_guide_labels(cairo_t *cr);
  void draw_guide_label(cairo_t *cr, const GuideBar &bar);
  void draw_thread_labels(cairo_t *cr);
  void draw_thread_label(cairo_t *cr, const ThreadRow &thread_row);

  static gboolean scale_area_draw_callback(GtkWidget *widget, cairo_t *cr, gpointer data);
  static gboolean thread_area_draw_callback(GtkWidget *widget, cairo_t *cr, gpointer data);
  static gboolean scroll_callback(GtkWidget *widget, GdkEventScroll *event, gpointer data);
  static gboolean key_press_callback(GtkWidget *widget, GdkEventKey *event, gpointer data);
  static gboolean key_release_callback(GtkWidget *widget, GdkEventKey *event, gpointer data);

  int row_to_pixel(int y) const {
    return y * _pixel_scale * 5 + _pixel_scale - _scroll;
  }
  int pixel_to_row(int y) const {
    return (y + _scroll - _pixel_scale) / (_pixel_scale * 5);
  }

  GtkWidget *_thread_area;

  cairo_pattern_t *_grid_pattern;

  int _highlighted_row = -1;
  int _highlighted_x = 0;
  int _scroll = 0;
  ColorBar _popup_bar;

  double _zoom_scale = 1.0;
  GtkGesture *_zoom_gesture = nullptr;
};

#endif
