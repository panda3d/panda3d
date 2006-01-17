// Filename: gtkStatsLabel.cxx
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

#include "gtkStatsLabel.h"
#include "gtkStatsMonitor.h"
#include "gtkStatsGraph.h"

int GtkStatsLabel::_left_margin = 2;
int GtkStatsLabel::_right_margin = 2;
int GtkStatsLabel::_top_margin = 2;
int GtkStatsLabel::_bottom_margin = 2;

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsLabel::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
GtkStatsLabel::
GtkStatsLabel(GtkStatsMonitor *monitor, GtkStatsGraph *graph,
              int thread_index, int collector_index, bool use_fullname) :
  _monitor(monitor),
  _graph(graph),
  _thread_index(thread_index),
  _collector_index(collector_index)
{
  _widget = NULL;
  if (use_fullname) {
    _text = _monitor->get_client_data()->get_collector_fullname(_collector_index);
  } else {
    _text = _monitor->get_client_data()->get_collector_name(_collector_index);
  }

  /*
  RGBColorf rgb = _monitor->get_collector_color(_collector_index);
  int r = (int)(rgb[0] * 255.0f);
  int g = (int)(rgb[1] * 255.0f);
  int b = (int)(rgb[2] * 255.0f);
  _bg_color = RGB(r, g, b);
  _bg_brush = CreateSolidBrush(RGB(r, g, b));

  // Should our foreground be black or white?
  float bright =
    rgb[0] * 0.299 +
    rgb[1] * 0.587 +
    rgb[2] * 0.114;

  if (bright >= 0.5) {
    _fg_color = RGB(0, 0, 0);
    _highlight_brush = (HBRUSH)GetStockObject(BLACK_BRUSH);
  } else {
    _fg_color = RGB(255, 255, 255);
    _highlight_brush = (HBRUSH)GetStockObject(WHITE_BRUSH);
  }
  */

  _highlight = false;
  _mouse_within = false;
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsLabel::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
GtkStatsLabel::
~GtkStatsLabel() {
  //  DeleteObject(_bg_brush);
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsLabel::setup
//       Access: Public
//  Description: Creates the actual widget.
////////////////////////////////////////////////////////////////////
GtkWidget *GtkStatsLabel::
setup() {
  _widget = gtk_event_box_new();
  GtkWidget *label = gtk_label_new(_text.c_str());
  gtk_container_add(GTK_CONTAINER(_widget), label);
  gtk_widget_show(label);
  gtk_widget_show(_widget);

  return _widget;
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsLabel::get_widget
//       Access: Public
//  Description: Returns the widget for this label.
////////////////////////////////////////////////////////////////////
GtkWidget *GtkStatsLabel::
get_widget() const {
  return _widget;
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsLabel::get_collector_index
//       Access: Public
//  Description: Returns the collector this label represents.
////////////////////////////////////////////////////////////////////
int GtkStatsLabel::
get_collector_index() const {
  return _collector_index;
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsLabel::set_highlight
//       Access: Public
//  Description: Enables or disables the visual highlight for this
//               label.
////////////////////////////////////////////////////////////////////
void GtkStatsLabel::
set_highlight(bool highlight) {
  if (_highlight != highlight) {
    _highlight = highlight;
    /*
    InvalidateRect(_widget, NULL, TRUE);
    */
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsLabel::get_highlight
//       Access: Public
//  Description: Returns true if the visual highlight for this
//               label is enabled.
////////////////////////////////////////////////////////////////////
bool GtkStatsLabel::
get_highlight() const {
  return _highlight;
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsLabel::set_mouse_within
//       Access: Private
//  Description: Used internally to indicate whether the mouse is
//               within the label's widget.
////////////////////////////////////////////////////////////////////
void GtkStatsLabel::
set_mouse_within(bool mouse_within) {
  if (_mouse_within != mouse_within) {
    _mouse_within = mouse_within;
    /*
    InvalidateRect(_widget, NULL, TRUE);
    */
  }
}
