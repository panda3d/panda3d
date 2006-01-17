// Filename: gtkStatsLabel.h
// Created by:  drose (16Jan06)
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

#include <gtk/gtk.h>

class GtkStatsMonitor;
class GtkStatsGraph;

////////////////////////////////////////////////////////////////////
//       Class : GtkStatsLabel
// Description : A text label that will draw in color appropriate for
//               a particular collector.  It also responds when the
//               user double-clicks on it.  This is handy for putting
//               colored labels on strip charts.
////////////////////////////////////////////////////////////////////
class GtkStatsLabel {
public:
  GtkStatsLabel(GtkStatsMonitor *monitor, GtkStatsGraph *graph,
                int thread_index, int collector_index, bool use_fullname);
  ~GtkStatsLabel();

  GtkWidget *setup();
  GtkWidget *get_widget() const;

  int get_collector_index() const;

  void set_highlight(bool highlight);
  bool get_highlight() const;

private:
  void set_mouse_within(bool mouse_within);

  GtkStatsMonitor *_monitor;
  GtkStatsGraph *_graph;
  int _thread_index;
  int _collector_index;
  string _text;
  GtkWidget *_widget;

  /*
  COLORREF _bg_color;
  COLORREF _fg_color;
  HBRUSH _bg_brush;
  HBRUSH _highlight_brush;
  */

  bool _highlight;
  bool _mouse_within;

  static int _left_margin, _right_margin;
  static int _top_margin, _bottom_margin;
};

#endif

