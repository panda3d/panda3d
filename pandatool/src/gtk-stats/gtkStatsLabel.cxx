// Filename: gtkStatsLabel.cxx
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

#include "gtkStatsLabel.h"
#include "gtkStatsMonitor.h"

#include "pStatClientData.h"
#include "pStatMonitor.h"

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsLabel::Constructor
//       Access: Public
//  Description: This constructor automatically figures out the
//               appropriate name and color for the label.
////////////////////////////////////////////////////////////////////
GtkStatsLabel::
GtkStatsLabel(PStatMonitor *monitor, int collector_index,
              Gdk_Font font) :
  _collector_index(collector_index),
  _font(font)
{
  set_events(GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK);

  _text = monitor->get_client_data()->get_collector_name(_collector_index);
  RGBColorf rgb = monitor->get_collector_color(_collector_index);
  _bg_color.set_rgb_p(rgb[0], rgb[1], rgb[2]);

  // Should our foreground be black or white?
  float bright =
    rgb[0] * 0.299 +
    rgb[1] * 0.587 +
    rgb[2] * 0.114;

  if (bright >= 0.5) {
    _fg_color.set_rgb_p(0, 0, 0);
  } else {
    _fg_color.set_rgb_p(1, 1, 1);
  }

  Gdk_Colormap::get_system().alloc(_fg_color);
  Gdk_Colormap::get_system().alloc(_bg_color);

  int text_width = _font.string_width(_text);
  int text_height = _font.height();

  _height = text_height + 4;
  _width = text_width + 4;

  set_usize(_width, _height);
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsLabel::get_width
//       Access: Public
//  Description: Returns the width of the widget as we requested it.
////////////////////////////////////////////////////////////////////
int GtkStatsLabel::
get_width() const {
  return _width;
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsLabel::get_height
//       Access: Public
//  Description: Returns the height of the widget as we requested it.
////////////////////////////////////////////////////////////////////
int GtkStatsLabel::
get_height() const {
  return _height;
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsLabel::configure_event_impl
//       Access: Private, Virtual
//  Description: Creates a new backing pixmap of the appropriate size.
////////////////////////////////////////////////////////////////////
gint GtkStatsLabel::
configure_event_impl(GdkEventConfigure *) {
  Gdk_Window window = get_window();

  _gc = Gdk_GC(window);
  _gc.set_foreground(_fg_color);
  _gc.set_background(_bg_color);
  _gc.set_font(_font);

  _reverse_gc = Gdk_GC(window);
  _reverse_gc.set_foreground(_bg_color);
  _reverse_gc.set_background(_fg_color);
  _reverse_gc.set_font(_font);

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsLabel::expose_event_impl
//       Access: Private, Virtual
//  Description: Redraw the text.  We don't bother with clipping
//               regions here, but just draw the whole text every
//               time.
////////////////////////////////////////////////////////////////////
gint GtkStatsLabel::
expose_event_impl(GdkEventExpose *event) {
  int text_width = _font.string_width(_text);
  int text_height = _font.height();

  Gdk_Window window = get_window();

  window.draw_rectangle(_reverse_gc, true, 0, 0, width(), height());
  window.draw_string(_font, _gc, width() - text_width - 2,
                      height() - (height() - text_height) / 2 - _font.descent(),
                      _text);
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsLabel::button_press_event_impl
//       Access: Private, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
gint GtkStatsLabel::
button_press_event_impl(GdkEventButton *button) {
  if (button->type == GDK_2BUTTON_PRESS && button->button == 1) {
    collector_picked(_collector_index);
    return true;
  }
  return false;
}
