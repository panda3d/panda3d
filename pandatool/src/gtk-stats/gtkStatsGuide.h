// Filename: gtkStatsGuide.h
// Created by:  drose (16Jul00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef GTKSTATSGUIDE_H
#define GTKSTATSGUIDE_H

#include "pandatoolbase.h"

#include <gtk--.h>

class PStatStripChart;

////////////////////////////////////////////////////////////////////
//       Class : GtkStatsGuide
// Description : A widget designed to be drawn next to a
//               GtkStatsStripChart that shows the labels associated
//               with the strip chart's guide bars.
////////////////////////////////////////////////////////////////////
class GtkStatsGuide : public Gtk::DrawingArea {
public:
  GtkStatsGuide(PStatStripChart *chart);

private:
  virtual gint configure_event_impl(GdkEventConfigure *event);
  virtual gint expose_event_impl(GdkEventExpose *event);

private:
  PStatStripChart *_chart;
};

#endif

