// Filename: gtkStatsLabel.h
// Created by:  drose (15Jul00)
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

#ifndef GTKSTATSLABEL_H
#define GTKSTATSLABEL_H

#include "pandatoolbase.h"

#include <gtk--.h>

class PStatMonitor;

////////////////////////////////////////////////////////////////////
//       Class : GtkStatsLabel
// Description : A text label that will draw in color appropriate for
//               a particular collector, instead of referring to some
//               dumb Gtk::Style.  It also throws a signal when the
//               user double-clicks on it, passing in the collector
//               index.  This is handy for putting colored labels on
//               strip charts.
////////////////////////////////////////////////////////////////////
class GtkStatsLabel : public Gtk::DrawingArea {
public:
  GtkStatsLabel(PStatMonitor *monitor, int collector_index,
                Gdk_Font font);

  int get_width() const;
  int get_height() const;

  SigC::Signal1<void, int> collector_picked;

private:
  virtual gint configure_event_impl (GdkEventConfigure *event);
  virtual gint expose_event_impl (GdkEventExpose *event);
  virtual gint button_press_event_impl(GdkEventButton *button);

private:
  int _collector_index;

  string _text;
  Gdk_Font _font;
  Gdk_Color _fg_color;
  Gdk_Color _bg_color;

  int _width;
  int _height;

  Gdk_GC _gc;
  Gdk_GC _reverse_gc;
};

#endif

