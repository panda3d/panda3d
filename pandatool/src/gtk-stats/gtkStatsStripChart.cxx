// Filename: gtkStatsStripChart.cxx
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

#include "gtkStatsStripChart.h"
#include "gtkStatsLabel.h"
#include "gtkStatsGuide.h"

#include "request_initial_size.h"
#include "pStatThreadData.h"
#include "pStatFrameData.h"
#include "pStatView.h"

#include <algorithm>


////////////////////////////////////////////////////////////////////
//     Function: GtkStatsStripChart::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
GtkStatsStripChart::
GtkStatsStripChart(GtkStatsMonitor *monitor, PStatView &view,
                   int collector_index, int xsize, int ysize) :
  PStatStripChart(monitor, view, collector_index, xsize, ysize)
{
  _is_dead = false;
  set_events(GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK);

  _label_align = manage(new Gtk::Alignment(1.0, 1.0));
  _label_align->show();

  _label_box = NULL;
  pack_labels();

  _guide = manage(new GtkStatsGuide(this));
  _guide->show();

  request_initial_size(*this, get_xsize(), get_ysize());
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsStripChart::mark_dead
//       Access: Public
//  Description: Called when the client's connection has been lost,
//               this should update the window in some obvious way to
//               indicate that the window is no longer live.
////////////////////////////////////////////////////////////////////
void GtkStatsStripChart::
mark_dead() {
  _is_dead = true;

  setup_white_gc();

  if (!first_data()) {
    force_redraw();
  } else {
    clear_region();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsStripChart::get_labels
//       Access: Public
//  Description: Returns an alignment widget that contains all of the
//               labels appropriate to this chart, already formatted
//               and stacked up bottom-to-top.  The window should pack
//               this widget suitably near the strip chart.
////////////////////////////////////////////////////////////////////
Gtk::Alignment *GtkStatsStripChart::
get_labels() {
  return _label_align;
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsStripChart::get_guide
//       Access: Public
//  Description: Returns a widget that contains the numeric labels for
//               the guide bars.  The window should pack this widget
//               suitably near the strip chart.
////////////////////////////////////////////////////////////////////
GtkStatsGuide *GtkStatsStripChart::
get_guide() {
  return _guide;
}


////////////////////////////////////////////////////////////////////
//     Function: GtkStatsStripChart::get_collector_gc
//       Access: Public
//  Description: Returns a graphics context suitable for drawing in
//               the indicated collector's color.
////////////////////////////////////////////////////////////////////
Gdk_GC GtkStatsStripChart::
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
//     Function: GtkStatsStripChart::clear_region
//       Access: Protected, Virtual
//  Description: Erases the chart area.
////////////////////////////////////////////////////////////////////
void GtkStatsStripChart::
clear_region() {
  _pixmap.draw_rectangle(_white_gc, true, 0, 0, get_xsize(), get_ysize());
  end_draw(0, get_xsize());
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsStripChart::copy_region
//       Access: Protected, Virtual
//  Description: Should be overridden by the user class to copy a
//               region of the chart from one part of the chart to
//               another.  This is used to implement scrolling.
////////////////////////////////////////////////////////////////////
void GtkStatsStripChart::
copy_region(int start_x, int end_x, int dest_x) {
  _pixmap.copy_area(_white_gc, 0, 0,
                    _pixmap, start_x, 0,
                    end_x - start_x + 1, get_ysize());

  // We could make a window-to-window copy to implement scrolling in
  // the window.  But this leads to trouble if the scrolling window
  // isn't on top.  Instead, we'll just do the scroll in the pixmap,
  // and then blt the pixmap back out--in principle, this ought to be
  // just as fast.
  /*
  Gdk_Window window = get_window();
  window.copy_area(_white_gc, 0, 0,
                   window, start_x, 0,
                   end_x - start_x + 1, get_ysize());
  */

  GdkRectangle update_rect;
  update_rect.x = dest_x;
  update_rect.y = 0;
  update_rect.width = end_x - start_x + 1;
  update_rect.height = get_ysize();
  draw(&update_rect);
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsStripChart::draw_slice
//       Access: Protected, Virtual
//  Description: Draws a single vertical slice of the strip chart, at
//               the given pixel position, and corresponding to the
//               indicated level data.
////////////////////////////////////////////////////////////////////
void GtkStatsStripChart::
draw_slice(int x, int w, const PStatStripChart::FrameData &fdata) {
  while (w > 0) {
    // Start by clearing the band first.
    _pixmap.draw_line(_white_gc, x, 0, x, get_ysize());

    float overall_time = 0.0;
    int y = get_ysize();

    FrameData::const_iterator fi;
    for (fi = fdata.begin(); fi != fdata.end(); ++fi) {
      const ColorData &cd = (*fi);
      overall_time += cd._net_value;

      if (overall_time > get_vertical_scale()) {
        // Off the top.  Go ahead and clamp it by hand, in case it's so
        // far off the top we'd overflow the 16-bit pixel value.
        _pixmap.draw_line(get_collector_gc(cd._collector_index), x, y, x, 0);
        // And we can consider ourselves done now.
        break;
      }

      int top_y = height_to_pixel(overall_time);
      _pixmap.draw_line(get_collector_gc(cd._collector_index), x, y, x, top_y);
      y = top_y;
    }
    x++;
    w--;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsStripChart::draw_empty
//       Access: Protected, Virtual
//  Description: Draws a single vertical slice of background color.
////////////////////////////////////////////////////////////////////
void GtkStatsStripChart::
draw_empty(int x, int w) {
  while (w > 0) {
    _pixmap.draw_line(_white_gc, x, 0, x, get_ysize());
    x++;
    w--;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsStripChart::draw_cursor
//       Access: Protected, Virtual
//  Description: Draws a single vertical slice of foreground color.
////////////////////////////////////////////////////////////////////
void GtkStatsStripChart::
draw_cursor(int x) {
  _pixmap.draw_line(_black_gc, x, 0, x, get_ysize());
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsStripChart::end_draw
//       Access: Protected, Virtual
//  Description: Should be overridden by the user class.  This hook
//               will be called after drawing a series of color bars
//               in the strip chart; it gives the pixel range that
//               was just redrawn.
////////////////////////////////////////////////////////////////////
void GtkStatsStripChart::
end_draw(int from_x, int to_x) {
  // Draw in the guide bars.
  int num_guide_bars = get_num_guide_bars();
  for (int i = 0; i < num_guide_bars; i++) {
    const GuideBar &bar = get_guide_bar(i);
    int y = height_to_pixel(bar._height);

    if (y >= 5) {
      // Only draw it if it's not too close to the top.
      switch (bar._style) {
      case GBS_target:
        _pixmap.draw_line(_light_gc, from_x, y, to_x, y);
        break;

      case GBS_normal:
      default:
        _pixmap.draw_line(_dark_gc, from_x, y, to_x, y);
        break;
      }
    }
  }

  GdkRectangle update_rect;
  update_rect.x = from_x;
  update_rect.y = 0;
  update_rect.width = to_x - from_x + 1;
  update_rect.height = get_ysize();
  draw(&update_rect);
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsStripChart::idle
//       Access: Protected, Virtual
//  Description: Called at the end of the draw cycle.
////////////////////////////////////////////////////////////////////
void GtkStatsStripChart::
idle() {
  if (_labels_changed) {
    pack_labels();
  }
  if (_guide_bars_changed) {
    GdkRectangle update_rect;
    update_rect.x = 0;
    update_rect.y = 0;
    update_rect.width = _guide->width();
    update_rect.height = _guide->height();
    _guide->draw(&update_rect);
    _guide_bars_changed = false;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsStripChart::configure_event_impl
//       Access: Private, Virtual
//  Description: Creates a new backing pixmap of the appropriate size.
////////////////////////////////////////////////////////////////////
gint GtkStatsStripChart::
configure_event_impl(GdkEventConfigure *) {
  if (width() != get_xsize() || height() != get_ysize() ||
      _pixmap.gdkobj() == (GdkDrawable *)NULL) {
    bool is_initial = true;
    if (_pixmap) {
      is_initial = false;
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
//     Function: GtkStatsStripChart::expose_event_impl
//       Access: Private, Virtual
//  Description: Redraw the screen from the backing pixmap.
////////////////////////////////////////////////////////////////////
gint GtkStatsStripChart::
expose_event_impl(GdkEventExpose *event) {
  get_window().draw_pixmap(_white_gc, _pixmap,
                           event->area.x, event->area.y,
                           event->area.x, event->area.y,
                           event->area.width, event->area.height);

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsStripChart::button_press_event_impl
//       Access: Private, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
gint GtkStatsStripChart::
button_press_event_impl(GdkEventButton *button) {
  if (button->type == GDK_2BUTTON_PRESS && button->button == 1) {
    int collector_index = get_collector_under_pixel(button->x, button->y);
    collector_picked(collector_index);
    return true;
  }
  return false;
}


////////////////////////////////////////////////////////////////////
//     Function: GtkStatsStripChart::pack_labels
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void GtkStatsStripChart::
pack_labels() {
  // First, remove the old labels.
  _label_align->remove();

  // Now add the new labels back in.
  _label_box = manage(new Gtk::VBox);
  _label_box->show();
  _label_align->add(*_label_box);

  Gdk_Font font = get_style()->gtkobj()->font;

  int num_labels = get_num_labels();
  for (int i = 0; i < num_labels; i++) {
    int collector_index = get_label_collector(i);
    GtkStatsLabel *label =
      new GtkStatsLabel(get_monitor(), collector_index, font);
    label->show();
    
    label->collector_picked.connect(collector_picked.slot());
    
    _label_box->pack_end(*manage(label), false, false);
  }
  
  _labels_changed = false;
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsStripChart::setup_white_gc
//       Access: Private
//  Description: Sets the color on _white_gc to be either actually
//               white (if the chart is still alive) or a light gray
//               (if the chart is dead).
////////////////////////////////////////////////////////////////////
void GtkStatsStripChart::
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

