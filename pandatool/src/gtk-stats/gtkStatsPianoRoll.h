// Filename: gtkStatsPianoRoll.h
// Created by:  drose (18Jul00)
// 
////////////////////////////////////////////////////////////////////

#ifndef GTKSTATSPIANOROLL_H
#define GTKSTATSPIANOROLL_H

#include <pandatoolbase.h>

#include "gtkStatsMonitor.h"

#include <pStatPianoRoll.h>
#include <pointerTo.h>

#include <gtk--.h>
#include <map>

class PStatView;
class GtkStatsGuide;

////////////////////////////////////////////////////////////////////
//       Class : GtkStatsPianoRoll
// Description : A special widget that draws a piano-roll style chart,
//               which shows the collectors explicitly stopping and
//               starting, one frame at a time.
////////////////////////////////////////////////////////////////////
class GtkStatsPianoRoll : public Gtk::DrawingArea, public PStatPianoRoll {
public:
  GtkStatsPianoRoll(GtkStatsMonitor *monitor, int thread_index,
                    int xsize, int ysize);

  void mark_dead();

  Gtk::Alignment *get_labels();

  Gdk_GC get_collector_gc(int collector_index);

private:
  virtual void begin_draw();
  virtual void draw_bar(int row, int from_x, int to_x);
  virtual void end_draw();
  virtual void idle();

  virtual gint configure_event_impl(GdkEventConfigure *event);
  virtual gint expose_event_impl(GdkEventExpose *event);

  void pack_labels();
  void setup_white_gc();

private:
  // Backing pixmap for drawing area.
  Gdk_Pixmap _pixmap;

  // Graphics contexts for fg/bg.  We don't use the contexts defined
  // in the style, because that would probably interfere with the
  // visibility of the chart.
  Gdk_GC _white_gc;
  Gdk_GC _black_gc;
  Gdk_GC _dark_gc;
  Gdk_GC _light_gc;

  // Table of graphics contexts for our various collectors.
  typedef map<int, Gdk_GC> GCs;
  GCs _gcs;

  // Table of Y-positions for each of our rows, measured from the
  // bottom.
  vector_int _y_positions;

  Gtk::Alignment *_label_align;
  Gtk::VBox *_label_box;
  bool _is_dead;
};

#include "gtkStatsPianoRoll.I"

#endif

