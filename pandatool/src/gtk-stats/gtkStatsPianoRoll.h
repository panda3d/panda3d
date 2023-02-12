/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file gtkStatsPianoRoll.h
 * @author drose
 * @date 2006-01-16
 */

#ifndef GTKSTATSPIANOROLL_H
#define GTKSTATSPIANOROLL_H

#include "pandatoolbase.h"

#include "gtkStatsGraph.h"
#include "pStatPianoRoll.h"
#include "pointerTo.h"

#include <gtk/gtk.h>

class GtkStatsMonitor;

/**
 * A window that draws a piano-roll style chart, which shows the collectors
 * explicitly stopping and starting, one frame at a time.
 */
class GtkStatsPianoRoll final : public PStatPianoRoll, public GtkStatsGraph {
public:
  GtkStatsPianoRoll(GtkStatsMonitor *monitor, int thread_index);
  virtual ~GtkStatsPianoRoll();

  virtual void new_data(int thread_index, int frame_number);
  virtual void force_redraw();
  virtual void changed_graph_size(int graph_xsize, int graph_ysize);

  virtual void set_time_units(int unit_mask);
  virtual void on_click_label(int collector_index);
  virtual void on_popup_label(int collector_index);
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

  virtual void additional_graph_window_paint(cairo_t *cr);
  virtual std::string get_graph_tooltip(int mouse_x, int mouse_y) const;
  virtual DragMode consider_drag_start(int graph_x, int graph_y);

  virtual gboolean handle_button_press(int graph_x, int graph_y,
                                       bool double_click, int button);
  virtual gboolean handle_button_release(int graph_x, int graph_y);
  virtual gboolean handle_motion(int graph_x, int graph_y);
  virtual gboolean handle_leave();

private:
  int get_collector_under_pixel(int xpoint, int ypoint) const;
  void update_labels();
  void draw_guide_bar(cairo_t *cr, const PStatGraph::GuideBar &bar);
  void draw_guide_labels(cairo_t *cr);
  void draw_guide_label(cairo_t *cr, const PStatGraph::GuideBar &bar);

  static gboolean draw_callback(GtkWidget *widget, cairo_t *cr, gpointer data);
};

#endif
