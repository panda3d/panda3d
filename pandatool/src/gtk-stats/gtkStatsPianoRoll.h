// Filename: gtkStatsPianoRoll.h
// Created by:  drose (16Jan06)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef GTKSTATSPIANOROLL_H
#define GTKSTATSPIANOROLL_H

#include "pandatoolbase.h"

#include "gtkStatsGraph.h"
#include "pStatPianoRoll.h"
#include "pointerTo.h"

#include <gtk/gtk.h>

class GtkStatsMonitor;

////////////////////////////////////////////////////////////////////
//       Class : GtkStatsPianoRoll
// Description : A window that draws a piano-roll style chart,
//               which shows the collectors explicitly stopping and
//               starting, one frame at a time.
////////////////////////////////////////////////////////////////////
class GtkStatsPianoRoll : public PStatPianoRoll, public GtkStatsGraph {
public:
  GtkStatsPianoRoll(GtkStatsMonitor *monitor, int thread_index);
  virtual ~GtkStatsPianoRoll();

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

  virtual void additional_graph_window_paint();
  virtual DragMode consider_drag_start(int graph_x, int graph_y);

  virtual gboolean handle_button_press(GtkWidget *widget, int graph_x, int graph_y,
				       bool double_click);
  virtual gboolean handle_button_release(GtkWidget *widget, int graph_x, int graph_y);
  virtual gboolean handle_motion(GtkWidget *widget, int graph_x, int graph_y);

private:
  int get_collector_under_pixel(int xpoint, int ypoint);
  void update_labels();
  void draw_guide_bar(GdkDrawable *surface, const PStatGraph::GuideBar &bar);
  void draw_guide_labels();
  void draw_guide_label(const PStatGraph::GuideBar &bar);

  static gboolean expose_event_callback(GtkWidget *widget, 
					GdkEventExpose *event, gpointer data);
};

#endif

