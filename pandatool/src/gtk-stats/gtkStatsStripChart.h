// Filename: gtkStatsStripChart.h
// Created by:  drose (14Jul00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef GTKSTATSSTRIPCHART_H
#define GTKSTATSSTRIPCHART_H

#include "pandatoolbase.h"

#include "gtkStatsMonitor.h"

#include "pStatStripChart.h"
#include "pointerTo.h"

#include <gtk--.h>
#include "pmap.h"

class PStatView;
class GtkStatsGuide;

////////////////////////////////////////////////////////////////////
//       Class : GtkStatsStripChart
// Description : A special widget that draws a strip chart, given a
//               view.
////////////////////////////////////////////////////////////////////
class GtkStatsStripChart : public Gtk::DrawingArea, public PStatStripChart {
public:
  GtkStatsStripChart(GtkStatsMonitor *monitor,
                     PStatView &view, int collector_index,
                     int xsize, int ysize);

  void mark_dead();

  Gtk::Alignment *get_labels();
  GtkStatsGuide *get_guide();

  Gdk_GC get_collector_gc(int collector_index);

  // This signal is thrown when the user float-clicks on a label or
  // on a band of color.
  SigC::Signal1<void, int> collector_picked;

private:
  virtual void clear_region();
  virtual void copy_region(int start_x, int end_x, int dest_x);
  virtual void draw_slice(int x, int w, int frame_number);
  virtual void draw_empty(int x, int w);
  virtual void draw_cursor(int x);
  virtual void end_draw(int from_x, int to_x);
  virtual void idle();

  virtual gint configure_event_impl(GdkEventConfigure *event);
  virtual gint expose_event_impl(GdkEventExpose *event);
  virtual gint button_press_event_impl(GdkEventButton *button);

  void pack_labels();
  void setup_white_gc();

private:
  // Backing pixmap for drawing area.
  Gdk_Pixmap _pixmap;

  // Graphics contexts for fg/bg.  We don't use the contexts defined
  // in the style, because that would probably interfere with the
  // visibility of the strip chart.
  Gdk_GC _white_gc;
  Gdk_GC _black_gc;
  Gdk_GC _dark_gc;
  Gdk_GC _light_gc;

  // Table of graphics contexts for our various collectors.
  typedef pmap<int, Gdk_GC> GCs;
  GCs _gcs;

  Gtk::Alignment *_label_align;
  Gtk::VBox *_label_box;
  GtkStatsGuide *_guide;
  bool _is_dead;
};

#include "gtkStatsStripChart.I"

#endif

