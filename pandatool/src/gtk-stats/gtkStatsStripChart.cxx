// Filename: gtkStatsStripChart.cxx
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

#include "gtkStatsStripChart.h"
#include "gtkStatsMonitor.h"
#include "pStatCollectorDef.h"
#include "numeric_types.h"

static const int default_strip_chart_width = 400;
static const int default_strip_chart_height = 100;

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsStripChart::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
GtkStatsStripChart::
GtkStatsStripChart(GtkStatsMonitor *monitor, int thread_index,
                   int collector_index, bool show_level) :
  PStatStripChart(monitor, 
                  show_level ? monitor->get_level_view(collector_index, thread_index) : monitor->get_view(thread_index), 
                  collector_index, 
                  default_strip_chart_width,
                  default_strip_chart_height),
  GtkStatsGraph(monitor, thread_index)
{
  _brush_origin = 0;

  _left_margin = 96;
  _right_margin = 32;
  _top_margin = 16;
  _bottom_margin = 8;

  if (show_level) {
    // If it's a level-type graph, show the appropriate units.
    if (_unit_name.empty()) {
      set_guide_bar_units(GBU_named);
    } else {
      set_guide_bar_units(GBU_named | GBU_show_units);
    }

  } else {
    // If it's a time-type graph, show the ms/Hz units.
    set_guide_bar_units(get_guide_bar_units() | GBU_show_units);
  }

  _smooth_check_box = 0;

  GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(hbox), _label_stack.get_widget(),
		     FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(hbox), _graph_window,
		     TRUE, TRUE, 0);

  gtk_container_add(GTK_CONTAINER(_window), hbox);

  gtk_widget_show_all(_window);  
  gtk_widget_show(_window);

  clear_region();
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsStripChart::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
GtkStatsStripChart::
~GtkStatsStripChart() {
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsStripChart::new_collector
//       Access: Public, Virtual
//  Description: Called whenever a new Collector definition is
//               received from the client.
////////////////////////////////////////////////////////////////////
void GtkStatsStripChart::
new_collector(int collector_index) {
  GtkStatsGraph::new_collector(collector_index);
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsStripChart::new_data
//       Access: Public, Virtual
//  Description: Called as each frame's data is made available.  There
//               is no gurantee the frames will arrive in order, or
//               that all of them will arrive at all.  The monitor
//               should be prepared to accept frames received
//               out-of-order or missing.
////////////////////////////////////////////////////////////////////
void GtkStatsStripChart::
new_data(int thread_index, int frame_number) {
  if (is_title_unknown()) {
    string window_title = get_title_text();
    if (!is_title_unknown()) {
      gtk_window_set_title(GTK_WINDOW(_window), window_title.c_str());
    }
  }

  if (!_pause) {
    update();

    /*
    string text = format_number(get_average_net_value(), get_guide_bar_units(), get_guide_bar_unit_name());
    if (_net_value_text != text) {
      _net_value_text = text;
      RECT rect;
      GetClientRect(_window, &rect);
      rect.bottom = _top_margin;
      InvalidateRect(_window, &rect, TRUE);
    }
    */
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsStripChart::force_redraw
//       Access: Public, Virtual
//  Description: Called when it is necessary to redraw the entire graph.
////////////////////////////////////////////////////////////////////
void GtkStatsStripChart::
force_redraw() {
  PStatStripChart::force_redraw();
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsStripChart::changed_graph_size
//       Access: Public, Virtual
//  Description: Called when the user has resized the window, forcing
//               a resize of the graph.
////////////////////////////////////////////////////////////////////
void GtkStatsStripChart::
changed_graph_size(int graph_xsize, int graph_ysize) {
  PStatStripChart::changed_size(graph_xsize, graph_ysize);
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsStripChart::set_time_units
//       Access: Public, Virtual
//  Description: Called when the user selects a new time units from
//               the monitor pulldown menu, this should adjust the
//               units for the graph to the indicated mask if it is a
//               time-based graph.
////////////////////////////////////////////////////////////////////
void GtkStatsStripChart::
set_time_units(int unit_mask) {
  int old_unit_mask = get_guide_bar_units();
  if ((old_unit_mask & (GBU_hz | GBU_ms)) != 0) {
    unit_mask = unit_mask & (GBU_hz | GBU_ms);
    unit_mask |= (old_unit_mask & GBU_show_units);
    set_guide_bar_units(unit_mask);

    /*
    RECT rect;
    GetClientRect(_window, &rect);
    rect.left = _right_margin;
    InvalidateRect(_window, &rect, TRUE);

    GetClientRect(_window, &rect);
    rect.bottom = _top_margin;
    InvalidateRect(_window, &rect, TRUE);
    */
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsStripChart::set_scroll_speed
//       Access: Public
//  Description: Called when the user selects a new scroll speed from
//               the monitor pulldown menu, this should adjust the
//               speed for the graph to the indicated value.
////////////////////////////////////////////////////////////////////
void GtkStatsStripChart::
set_scroll_speed(float scroll_speed) {
  // The speed factor indicates chart widths per minute.
  if (scroll_speed != 0.0f) {
    set_horizontal_scale(60.0f / scroll_speed);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsStripChart::clicked_label
//       Access: Public, Virtual
//  Description: Called when the user single-clicks on a label.
////////////////////////////////////////////////////////////////////
void GtkStatsStripChart::
clicked_label(int collector_index) {
  if (collector_index < 0) {
    // Clicking on whitespace in the graph is the same as clicking on
    // the top label.
    collector_index = get_collector_index();
  }

  if (collector_index == get_collector_index() && collector_index != 0) {
    // Clicking on the top label means to go up to the parent level.
    const PStatClientData *client_data = 
      GtkStatsGraph::_monitor->get_client_data();
    if (client_data->has_collector(collector_index)) {
      const PStatCollectorDef &def =
        client_data->get_collector_def(collector_index);
      if (def._parent_index == 0 && get_view().get_show_level()) {
        // Unless the parent is "Frame", and we're not a time collector.
      } else {
        set_collector_index(def._parent_index);
      }
    }

  } else {
    // Clicking on any other label means to focus on that.
    set_collector_index(collector_index);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsStripChart::set_vertical_scale
//       Access: Public
//  Description: Changes the value the height of the vertical axis
//               represents.  This may force a redraw.
////////////////////////////////////////////////////////////////////
void GtkStatsStripChart::
set_vertical_scale(float value_height) {
  PStatStripChart::set_vertical_scale(value_height);

  gtk_widget_queue_draw(_graph_window);
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsStripChart::update_labels
//       Access: Protected, Virtual
//  Description: Resets the list of labels.
////////////////////////////////////////////////////////////////////
void GtkStatsStripChart::
update_labels() {
  PStatStripChart::update_labels();

  _label_stack.clear_labels();
  for (int i = 0; i < get_num_labels(); i++) {
    _label_stack.add_label(GtkStatsGraph::_monitor, this, _thread_index,
                           get_label_collector(i), false);
  }
  _labels_changed = false;
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsStripChart::clear_region
//       Access: Protected, Virtual
//  Description: Erases the chart area.
////////////////////////////////////////////////////////////////////
void GtkStatsStripChart::
clear_region() {
  gdk_gc_set_rgb_fg_color(_pixmap_gc, &rgb_white);
  gdk_draw_rectangle(_pixmap, _pixmap_gc, TRUE, 0, 0, 
		     get_xsize(), get_ysize());
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
  gdk_draw_drawable(_pixmap, _pixmap_gc, _pixmap,
		    start_x, 0, dest_x, 0,
		    end_x - start_x, get_ysize());
  
  // Also shift the brush origin over, so we still get proper
  // dithering.
  _brush_origin += (dest_x - start_x);
  //  SetBrushOrgEx(_bitmap_dc, _brush_origin, 0, NULL);

  GdkRectangle rect = {
    dest_x, 0, end_x - start_x, get_ysize() 
  };
  gdk_window_invalidate_rect(_graph_window->window, &rect, FALSE);
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
  // Start by clearing the band first.
  gdk_gc_set_rgb_fg_color(_pixmap_gc, &rgb_white);
  gdk_draw_rectangle(_pixmap, _pixmap_gc, TRUE, x, 0, 
		     w + 1, get_ysize());

  float overall_time = 0.0;
  int y = get_ysize();

  FrameData::const_iterator fi;
  for (fi = fdata.begin(); fi != fdata.end(); ++fi) {
    const ColorData &cd = (*fi);
    overall_time += cd._net_value;
    GdkGC *gc = get_collector_gc(cd._collector_index);

    if (overall_time > get_vertical_scale()) {
      // Off the top.  Go ahead and clamp it by hand, in case it's so
      // far off the top we'd overflow the 16-bit pixel value.
      gdk_draw_rectangle(_pixmap, gc, TRUE, x, 0, w + 1, y);

      // And we can consider ourselves done now.
      return;
    }

    int top_y = height_to_pixel(overall_time);
    gdk_draw_rectangle(_pixmap, gc, TRUE, x, top_y, w + 1, y);
    y = top_y;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsStripChart::draw_empty
//       Access: Protected, Virtual
//  Description: Draws a single vertical slice of background color.
////////////////////////////////////////////////////////////////////
void GtkStatsStripChart::
draw_empty(int x, int w) {
  gdk_gc_set_rgb_fg_color(_pixmap_gc, &rgb_white);
  gdk_draw_rectangle(_pixmap, _pixmap_gc, TRUE, x, 0, 
		     w + 1, get_ysize());
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsStripChart::draw_cursor
//       Access: Protected, Virtual
//  Description: Draws a single vertical slice of foreground color.
////////////////////////////////////////////////////////////////////
void GtkStatsStripChart::
draw_cursor(int x) {
  gdk_gc_set_rgb_fg_color(_pixmap_gc, &rgb_black);
  gdk_draw_line(_pixmap, _pixmap_gc, x, 0, x, get_ysize());
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
    draw_guide_bar(from_x, to_x, get_guide_bar(i));
  }

  GdkRectangle rect = {
    from_x, 0, to_x - from_x + 1, get_ysize() 
  };
  gdk_window_invalidate_rect(_graph_window->window, &rect, FALSE);
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsStripChart::consider_drag_start
//       Access: Protected, Virtual
//  Description: Based on the mouse position within the window's
//               client area, look for draggable things the mouse
//               might be hovering over and return the apprioprate
//               DragMode enum or DM_none if nothing is indicated.
////////////////////////////////////////////////////////////////////
GtkStatsGraph::DragMode GtkStatsStripChart::
consider_drag_start(int mouse_x, int mouse_y, int width, int height) {
  if (mouse_x >= _graph_left && mouse_x < _graph_left + get_xsize()) {
    if (mouse_y >= _graph_top && mouse_y < _graph_top + get_ysize()) {
      // See if the mouse is over a user-defined guide bar.
      int y = mouse_y - _graph_top;
      float from_height = pixel_to_height(y + 2);
      float to_height = pixel_to_height(y - 2);
      _drag_guide_bar = find_user_guide_bar(from_height, to_height);
      if (_drag_guide_bar >= 0) {
        return DM_guide_bar;
      }

    } else {
      // The mouse is above or below the graph; maybe create a new
      // guide bar.
      return DM_new_guide_bar;
    }
  }

  return GtkStatsGraph::consider_drag_start(mouse_x, mouse_y, width, height);
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsStripChart::set_drag_mode
//       Access: Protected, Virtual
//  Description: This should be called whenever the drag mode needs to
//               change state.  It provides hooks for a derived class
//               to do something special.
////////////////////////////////////////////////////////////////////
void GtkStatsStripChart::
set_drag_mode(GtkStatsGraph::DragMode drag_mode) {
  GtkStatsGraph::set_drag_mode(drag_mode);

  switch (_drag_mode) {
  case DM_scale:
  case DM_left_margin:
  case DM_right_margin:
  case DM_sizing:
    // Disable smoothing for these expensive operations.
    set_average_mode(false);
    break;

  default:
    // Restore smoothing according to the current setting of the check
    // box.
    /*
    int result = SendMessage(_smooth_check_box, BM_GETCHECK, 0, 0);
    set_average_mode(result == BST_CHECKED);
    */
    break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsStripChart::draw_guide_bar
//       Access: Private
//  Description: Draws the line for the indicated guide bar on the
//               graph.
////////////////////////////////////////////////////////////////////
void GtkStatsStripChart::
draw_guide_bar(int from_x, int to_x, 
               const PStatGraph::GuideBar &bar) {
  int y = height_to_pixel(bar._height);

  if (y > 0) {
    // Only draw it if it's not too close to the top.
    switch (bar._style) {
    case GBS_target:
      gdk_gc_set_rgb_fg_color(_pixmap_gc, &rgb_light_gray);
      break;

    case GBS_user:
      gdk_gc_set_rgb_fg_color(_pixmap_gc, &rgb_user_guide_bar);
      break;
      
    case GBS_normal:
      gdk_gc_set_rgb_fg_color(_pixmap_gc, &rgb_dark_gray);
      break;
    }
    gdk_draw_line(_pixmap, _pixmap_gc, from_x, y, to_x, y);
  }
}
