// Filename: gtkStatsPianoRoll.cxx
// Created by:  drose (18Jul00)
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

#include "gtkStatsPianoRoll.h"
#include "gtkStatsLabel.h"
#include "gtkStatsGuide.h"

#include <request_initial_size.h>
#include <pStatThreadData.h>
#include <pStatFrameData.h>
#include <pStatView.h>

#include <algorithm>


////////////////////////////////////////////////////////////////////
//     Function: GtkStatsPianoRoll::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
GtkStatsPianoRoll::
GtkStatsPianoRoll(GtkStatsMonitor *monitor, int thread_index,
                  int xsize, int ysize) :
  PStatPianoRoll(monitor, thread_index, xsize, ysize)
{
  _is_dead = false;
  set_events(GDK_EXPOSURE_MASK);

  _label_align = manage(new Gtk::Alignment(1.0, 1.0));
  _label_align->show();

  _label_box = NULL;
  pack_labels();

  request_initial_size(*this, get_xsize(), get_ysize());
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsPianoRoll::mark_dead
//       Access: Public
//  Description: Called when the client's connection has been lost,
//               this should update the window in some obvious way to
//               indicate that the window is no longer live.
////////////////////////////////////////////////////////////////////
void GtkStatsPianoRoll::
mark_dead() {
  _is_dead = true;

  setup_white_gc();
  force_redraw();
}


////////////////////////////////////////////////////////////////////
//     Function: GtkStatsPianoRoll::get_labels
//       Access: Public
//  Description: Returns an alignment widget that contains all of the
//               labels appropriate to this chart, already formatted
//               and stacked up bottom-to-top.  The window should pack
//               this widget suitably near the strip chart.
////////////////////////////////////////////////////////////////////
Gtk::Alignment *GtkStatsPianoRoll::
get_labels() {
  return _label_align;
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsPianoRoll::get_collector_gc
//       Access: Public
//  Description: Returns a graphics context suitable for drawing in
//               the indicated collector's color.
////////////////////////////////////////////////////////////////////
Gdk_GC GtkStatsPianoRoll::
get_collector_gc(int collector_index) {
  GCs::iterator gi;
  gi = _gcs.find(collector_index);
  if (gi != _gcs.end()) {
    return (*gi).second;
  }

  // Ask the monitor what color this guy should be.
  RGBColorf rgb = get_monitor()->get_collector_color(collector_index);
  Gdk_Color color;
  color.set_rgb_p(rgb[0], rgb[1], rgb[2]);

  // Now allocate the color from the system colormap.
  Gdk_Colormap::get_system().alloc(color);

  // Allocate a new graphics context.
  Gdk_GC gc(_pixmap);
  gc.set_foreground(color);

  _gcs[collector_index] = gc;
  return gc;
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsPianoRoll::begin_draw
//       Access: Protected, Virtual
//  Description: Erases the chart area in preparation for drawing it
//               full of bars.
////////////////////////////////////////////////////////////////////
void GtkStatsPianoRoll::
begin_draw() {
  _pixmap.draw_rectangle(_white_gc, true, 0, 0, get_xsize(), get_ysize());

  Gdk_Font font = get_style()->gtkobj()->font;
  int text_height = font.height();

  // Draw in the guide bars.
  int num_guide_bars = get_num_guide_bars();
  for (int i = 0; i < num_guide_bars; i++) {
    const GuideBar &bar = get_guide_bar(i);
    int x = (int)((float)get_xsize() * bar._height / get_horizontal_scale());

    if (x >= 5 && x <= get_xsize() - 5) {
      // Only draw it if it's not too close to either edge.
      if (bar._is_target) {
        _pixmap.draw_line(_light_gc, x, text_height + 4, x, get_ysize());
      } else {
        _pixmap.draw_line(_dark_gc, x, text_height + 4, x, get_ysize());
      }
    }
  }

}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsPianoRoll::draw_bar
//       Access: Protected, Virtual
//  Description: Draws a single bar on the chart.
////////////////////////////////////////////////////////////////////
void GtkStatsPianoRoll::
draw_bar(int row, int from_x, int to_x) {
  if (row >= 0 && row < (int)_y_positions.size()) {
    int y = height() - _y_positions[row];
    _pixmap.draw_rectangle(get_collector_gc(get_label_collector(row)),
                           true, from_x, y - 6, to_x - from_x, 12);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsPianoRoll::end_draw
//       Access: Protected, Virtual
//  Description: Called after all the bars have been drawn, this
//               triggers a refresh event to draw it to the window.
////////////////////////////////////////////////////////////////////
void GtkStatsPianoRoll::
end_draw() {
  // Draw in the labels for the guide bars.  We do this in end_draw()
  // instead of in begin_draw() so the labels will appear on top of
  // any of the color bars.
  Gdk_Font font = get_style()->gtkobj()->font;
  int text_ascent = font.ascent();

  int num_guide_bars = get_num_guide_bars();
  for (int i = 0; i < num_guide_bars; i++) {
    const GuideBar &bar = get_guide_bar(i);
    int x = (int)((float)get_xsize() * bar._height / get_horizontal_scale());

    if (x >= 5 && x <= get_xsize() - 5) {
      // Only draw it if it's not too close to either edge.
      int width = font.string_measure(bar._label);
      _pixmap.draw_string(font, _black_gc, x - width / 2, text_ascent + 2,
                          bar._label);
    }
  }

  GdkRectangle update_rect;
  update_rect.x = 0;
  update_rect.y = 0;
  update_rect.width = get_xsize();
  update_rect.height = get_ysize();
  draw(&update_rect);
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsPianoRoll::idle
//       Access: Protected, Virtual
//  Description: Called at the end of the draw cycle.
////////////////////////////////////////////////////////////////////
void GtkStatsPianoRoll::
idle() {
  if (_labels_changed) {
    pack_labels();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsPianoRoll::configure_event_impl
//       Access: Private, Virtual
//  Description: Creates a new backing pixmap of the appropriate size.
////////////////////////////////////////////////////////////////////
gint GtkStatsPianoRoll::
configure_event_impl(GdkEventConfigure *) {
  if (width() != get_xsize() || height() != get_ysize() ||
      _pixmap.gdkobj() == (GdkDrawable *)NULL) {
    if (_pixmap) {
      _pixmap.release();
    }

    _pixmap.create(get_window(), width(), height());

    Gdk_Colormap system_colormap = Gdk_Colormap::get_system();

    _white_gc = Gdk_GC(_pixmap);
    setup_white_gc();

    _black_gc = Gdk_GC(_pixmap);
    _black_gc.set_foreground(system_colormap.black());

    _dark_gc = Gdk_GC(_pixmap);
    Gdk_Color dark;
    dark.set_grey_p(0.2);
    system_colormap.alloc(dark);
    _dark_gc.set_foreground(dark);

    _light_gc = Gdk_GC(_pixmap);
    Gdk_Color light;
    light.set_grey_p(0.6);
    system_colormap.alloc(light);
    _light_gc.set_foreground(light);

    _pixmap.draw_rectangle(_white_gc, true, 0, 0, width(), height());

    changed_size(width(), height());
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsPianoRoll::expose_event_impl
//       Access: Private, Virtual
//  Description: Redraw the screen from the backing pixmap.
////////////////////////////////////////////////////////////////////
gint GtkStatsPianoRoll::
expose_event_impl(GdkEventExpose *event) {
  get_window().draw_pixmap(_white_gc, _pixmap,
                           event->area.x, event->area.y,
                           event->area.x, event->area.y,
                           event->area.width, event->area.height);

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsPianoRoll::pack_labels
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void GtkStatsPianoRoll::
pack_labels() {
  // First, remove the old labels.
  _label_align->remove();

  // Now add the new labels back in.
  _label_box = manage(new Gtk::VBox);
  _label_box->show();
  _label_align->add(*_label_box);

  Gdk_Font font = get_style()->gtkobj()->font;
  int num_labels = get_num_labels();

  while ((int)_y_positions.size() < num_labels) {
    _y_positions.push_back(0);
  }

  int y = 0;
  for (int i = 0; i < num_labels; i++) {
    int collector_index = get_label_collector(i);
    GtkStatsLabel *label =
      new GtkStatsLabel(get_monitor(), collector_index, font);
    label->show();

    _label_box->pack_end(*manage(label), false, false);
    _y_positions[i] = y + label->get_height() / 2;

    y += label->get_height();
  }

  _labels_changed = false;
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsPianoRoll::setup_white_gc
//       Access: Private
//  Description: Sets the color on _white_gc to be either actually
//               white (if the chart is still alive) or a light gray
//               (if the chart is dead).
////////////////////////////////////////////////////////////////////
void GtkStatsPianoRoll::
setup_white_gc() {
  Gdk_Colormap system_colormap = Gdk_Colormap::get_system();

  if (_is_dead) {
    Gdk_Color death;
    death.set_grey_p(0.8);
    system_colormap.alloc(death);

    _white_gc.set_foreground(death);

  } else {
    _white_gc.set_foreground(system_colormap.white());
  }

}

