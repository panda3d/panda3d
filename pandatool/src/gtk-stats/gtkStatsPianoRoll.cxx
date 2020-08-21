/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file gtkStatsPianoRoll.cxx
 * @author drose
 * @date 2006-01-16
 */

#include "gtkStatsPianoRoll.h"
#include "gtkStatsMonitor.h"
#include "numeric_types.h"
#include "gtkStatsLabelStack.h"

static const int default_piano_roll_width = 400;
static const int default_piano_roll_height = 200;

/**
 *
 */
GtkStatsPianoRoll::
GtkStatsPianoRoll(GtkStatsMonitor *monitor, int thread_index) :
  PStatPianoRoll(monitor, thread_index,
                 default_piano_roll_width,
                 default_piano_roll_height),
  GtkStatsGraph(monitor)
{
  // Let's show the units on the guide bar labels.  There's room.
  set_guide_bar_units(get_guide_bar_units() | GBU_show_units);

  // Add a DrawingArea widget on top of the graph, to display all of the scale
  // units.
  _scale_area = gtk_drawing_area_new();
  g_signal_connect(G_OBJECT(_scale_area), "expose_event",
       G_CALLBACK(expose_event_callback), this);
  gtk_box_pack_start(GTK_BOX(_graph_vbox), _scale_area,
         FALSE, FALSE, 0);
  gtk_widget_set_size_request(_scale_area, 0, 20);


  gtk_widget_set_size_request(_graph_window, default_piano_roll_width,
            default_piano_roll_height);

  const PStatClientData *client_data =
    GtkStatsGraph::_monitor->get_client_data();
  std::string thread_name = client_data->get_thread_name(_thread_index);
  std::string window_title = thread_name + " thread piano roll";
  gtk_window_set_title(GTK_WINDOW(_window), window_title.c_str());

  gtk_widget_show_all(_window);
  gtk_widget_show(_window);

  // Allow the window to be resized as small as the user likes.  We have to do
  // this after the window has been shown; otherwise, it will affect the
  // window's initial size.
  gtk_widget_set_size_request(_window, 0, 0);

  clear_region();
}

/**
 *
 */
GtkStatsPianoRoll::
~GtkStatsPianoRoll() {
}

/**
 * Called as each frame's data is made available.  There is no gurantee the
 * frames will arrive in order, or that all of them will arrive at all.  The
 * monitor should be prepared to accept frames received out-of-order or
 * missing.
 */
void GtkStatsPianoRoll::
new_data(int thread_index, int frame_number) {
  if (!_pause) {
    update();
  }
}

/**
 * Called when it is necessary to redraw the entire graph.
 */
void GtkStatsPianoRoll::
force_redraw() {
  PStatPianoRoll::force_redraw();
}

/**
 * Called when the user has resized the window, forcing a resize of the graph.
 */
void GtkStatsPianoRoll::
changed_graph_size(int graph_xsize, int graph_ysize) {
  PStatPianoRoll::changed_size(graph_xsize, graph_ysize);
}

/**
 * Called when the user selects a new time units from the monitor pulldown
 * menu, this should adjust the units for the graph to the indicated mask if
 * it is a time-based graph.
 */
void GtkStatsPianoRoll::
set_time_units(int unit_mask) {
  int old_unit_mask = get_guide_bar_units();
  if ((old_unit_mask & (GBU_hz | GBU_ms)) != 0) {
    unit_mask = unit_mask & (GBU_hz | GBU_ms);
    unit_mask |= (old_unit_mask & GBU_show_units);
    set_guide_bar_units(unit_mask);

    gtk_widget_queue_draw(_scale_area);
  }
}

/**
 * Called when the user single-clicks on a label.
 */
void GtkStatsPianoRoll::
clicked_label(int collector_index) {
  if (collector_index >= 0) {
    GtkStatsGraph::_monitor->open_strip_chart(_thread_index, collector_index, false);
  }
}

/**
 * Changes the amount of time the width of the horizontal axis represents.
 * This may force a redraw.
 */
void GtkStatsPianoRoll::
set_horizontal_scale(double time_width) {
  PStatPianoRoll::set_horizontal_scale(time_width);

  gtk_widget_queue_draw(_graph_window);
  gtk_widget_queue_draw(_scale_area);
}

/**
 * Erases the chart area.
 */
void GtkStatsPianoRoll::
clear_region() {
  gdk_gc_set_rgb_fg_color(_pixmap_gc, &rgb_white);
  gdk_draw_rectangle(_pixmap, _pixmap_gc, TRUE, 0, 0,
         get_xsize(), get_ysize());
}

/**
 * Erases the chart area in preparation for drawing a bunch of bars.
 */
void GtkStatsPianoRoll::
begin_draw() {
  clear_region();

  // Draw in the guide bars.
  int num_guide_bars = get_num_guide_bars();
  for (int i = 0; i < num_guide_bars; i++) {
    draw_guide_bar(_pixmap, get_guide_bar(i));
  }
}

/**
 * Draws a single bar on the chart.
 */
void GtkStatsPianoRoll::
draw_bar(int row, int from_x, int to_x) {
  if (row >= 0 && row < _label_stack.get_num_labels()) {
    int y = _label_stack.get_label_y(row, _graph_window);
    int height = _label_stack.get_label_height(row);

    int collector_index = get_label_collector(row);
    GdkGC *gc = get_collector_gc(collector_index);

    gdk_draw_rectangle(_pixmap, gc, TRUE,
           from_x, y - height + 2,
           to_x - from_x, height - 4);
  }
}

/**
 * Called after all the bars have been drawn, this triggers a refresh event to
 * draw it to the window.
 */
void GtkStatsPianoRoll::
end_draw() {
  gtk_widget_queue_draw(_graph_window);
}

/**
 * Called at the end of the draw cycle.
 */
void GtkStatsPianoRoll::
idle() {
  if (_labels_changed) {
    update_labels();
  }
}

/**
 * This is called during the servicing of expose_event; it gives a derived
 * class opportunity to do some further painting into the graph window.
 */
void GtkStatsPianoRoll::
additional_graph_window_paint() {
  int num_user_guide_bars = get_num_user_guide_bars();
  for (int i = 0; i < num_user_guide_bars; i++) {
    draw_guide_bar(_graph_window->window, get_user_guide_bar(i));
  }
}

/**
 * Based on the mouse position within the graph window, look for draggable
 * things the mouse might be hovering over and return the appropriate DragMode
 * enum or DM_none if nothing is indicated.
 */
GtkStatsGraph::DragMode GtkStatsPianoRoll::
consider_drag_start(int graph_x, int graph_y) {
  if (graph_y >= 0 && graph_y < get_ysize()) {
    if (graph_x >= 0 && graph_x < get_xsize()) {
      // See if the mouse is over a user-defined guide bar.
      int x = graph_x;
      double from_height = pixel_to_height(x - 2);
      double to_height = pixel_to_height(x + 2);
      _drag_guide_bar = find_user_guide_bar(from_height, to_height);
      if (_drag_guide_bar >= 0) {
        return DM_guide_bar;
      }

    } else {
      // The mouse is left or right of the graph; maybe create a new guide
      // bar.
      return DM_new_guide_bar;
    }
  }

  return GtkStatsGraph::consider_drag_start(graph_x, graph_y);
}

/**
 * Called when the mouse button is depressed within the graph window.
 */
gboolean GtkStatsPianoRoll::
handle_button_press(GtkWidget *widget, int graph_x, int graph_y,
        bool double_click) {
  if (double_click) {
    // Double-clicking on a color bar in the graph is the same as double-
    // clicking on the corresponding label.
    clicked_label(get_collector_under_pixel(graph_x, graph_y));
    return TRUE;
  }

  if (_potential_drag_mode == DM_none) {
    set_drag_mode(DM_scale);
    _drag_scale_start = pixel_to_height(graph_x);
    // SetCapture(_graph_window);
    return TRUE;

  } else if (_potential_drag_mode == DM_guide_bar && _drag_guide_bar >= 0) {
    set_drag_mode(DM_guide_bar);
    _drag_start_x = graph_x;
    // SetCapture(_graph_window);
    return TRUE;
  }

  return GtkStatsGraph::handle_button_press(widget, graph_x, graph_y,
              double_click);
}

/**
 * Called when the mouse button is released within the graph window.
 */
gboolean GtkStatsPianoRoll::
handle_button_release(GtkWidget *widget, int graph_x, int graph_y) {
  if (_drag_mode == DM_scale) {
    set_drag_mode(DM_none);
    // ReleaseCapture();
    return handle_motion(widget, graph_x, graph_y);

  } else if (_drag_mode == DM_guide_bar) {
    if (graph_x < 0 || graph_x >= get_xsize()) {
      remove_user_guide_bar(_drag_guide_bar);
    } else {
      move_user_guide_bar(_drag_guide_bar, pixel_to_height(graph_x));
    }
    set_drag_mode(DM_none);
    // ReleaseCapture();
    return handle_motion(widget, graph_x, graph_y);
  }

  return GtkStatsGraph::handle_button_release(widget, graph_x, graph_y);
}

/**
 * Called when the mouse is moved within the graph window.
 */
gboolean GtkStatsPianoRoll::
handle_motion(GtkWidget *widget, int graph_x, int graph_y) {
  if (_drag_mode == DM_none && _potential_drag_mode == DM_none) {
    // When the mouse is over a color bar, highlight it.
    _label_stack.highlight_label(get_collector_under_pixel(graph_x, graph_y));

    /*
    // Now we want to get a WM_MOUSELEAVE when the mouse leaves the graph
    // window.
    TRACKMOUSEEVENT tme = {
      sizeof(TRACKMOUSEEVENT),
      TME_LEAVE,
      _graph_window,
      0
    };
    TrackMouseEvent(&tme);
    */

  } else {
    // If the mouse is in some drag mode, stop highlighting.
    _label_stack.highlight_label(-1);
  }

  if (_drag_mode == DM_scale) {
    double ratio = (double)graph_x / (double)get_xsize();
    if (ratio > 0.0f) {
      set_horizontal_scale(_drag_scale_start / ratio);
    }
    return TRUE;

  } else if (_drag_mode == DM_new_guide_bar) {
    // We haven't created the new guide bar yet; we won't until the mouse
    // comes within the graph's region.
    if (graph_x >= 0 && graph_x < get_xsize()) {
      set_drag_mode(DM_guide_bar);
      _drag_guide_bar = add_user_guide_bar(pixel_to_height(graph_x));
      return TRUE;
    }

  } else if (_drag_mode == DM_guide_bar) {
    move_user_guide_bar(_drag_guide_bar, pixel_to_height(graph_x));
    return TRUE;
  }

  return GtkStatsGraph::handle_motion(widget, graph_x, graph_y);
}

/**
 * Returns the collector index associated with the indicated vertical row, or
 * -1.
 */
int GtkStatsPianoRoll::
get_collector_under_pixel(int xpoint, int ypoint) {
  if (_label_stack.get_num_labels() == 0) {
    return -1;
  }

  // Assume all of the labels are the same height.
  int height = _label_stack.get_label_height(0);
  int row = (get_ysize() - ypoint) / height;
  if (row >= 0 && row < _label_stack.get_num_labels()) {
    return _label_stack.get_label_collector_index(row);
  } else  {
    return -1;
  }
}

/**
 * Resets the list of labels.
 */
void GtkStatsPianoRoll::
update_labels() {
  _label_stack.clear_labels();
  for (int i = 0; i < get_num_labels(); i++) {
    _label_stack.add_label(GtkStatsGraph::_monitor, this,
         _thread_index,
         get_label_collector(i), true);
  }
  _labels_changed = false;
}

/**
 * Draws the line for the indicated guide bar on the graph.
 */
void GtkStatsPianoRoll::
draw_guide_bar(GdkDrawable *surface, const PStatGraph::GuideBar &bar) {
  int x = height_to_pixel(bar._height);

  if (x > 0 && x < get_xsize() - 1) {
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
    gdk_draw_line(surface, _pixmap_gc, x, 0, x, get_ysize());
  }
}

/**
 * This is called during the servicing of expose_event.
 */
void GtkStatsPianoRoll::
draw_guide_labels() {
  int i;
  int num_guide_bars = get_num_guide_bars();
  for (i = 0; i < num_guide_bars; i++) {
    draw_guide_label(get_guide_bar(i));
  }

  int num_user_guide_bars = get_num_user_guide_bars();
  for (i = 0; i < num_user_guide_bars; i++) {
    draw_guide_label(get_user_guide_bar(i));
  }
}

/**
 * Draws the text for the indicated guide bar label at the top of the graph.
 */
void GtkStatsPianoRoll::
draw_guide_label(const PStatGraph::GuideBar &bar) {
  GdkGC *gc = gdk_gc_new(_scale_area->window);

  switch (bar._style) {
  case GBS_target:
    gdk_gc_set_rgb_fg_color(gc, &rgb_light_gray);
    break;

  case GBS_user:
    gdk_gc_set_rgb_fg_color(gc, &rgb_user_guide_bar);
    break;

  case GBS_normal:
    gdk_gc_set_rgb_fg_color(gc, &rgb_dark_gray);
    break;
  }

  int x = height_to_pixel(bar._height);
  const std::string &label = bar._label;

  PangoLayout *layout = gtk_widget_create_pango_layout(_window, label.c_str());
  int width, height;
  pango_layout_get_pixel_size(layout, &width, &height);

  if (bar._style != GBS_user) {
    double from_height = pixel_to_height(x - width);
    double to_height = pixel_to_height(x + width);
    if (find_user_guide_bar(from_height, to_height) >= 0) {
      // Omit the label: there's a user-defined guide bar in the same space.
      g_object_unref(layout);
      g_object_unref(gc);
      return;
    }
  }

  if (x >= 0 && x < get_xsize()) {
    // Now convert our x to a coordinate within our drawing area.
    int junk_y;

    // The x coordinate comes from the graph_window.
    gtk_widget_translate_coordinates(_graph_window, _scale_area,
             x, 0,
             &x, &junk_y);

    int this_x = x - width / 2;
    gdk_draw_layout(_scale_area->window, gc, this_x,
        _scale_area->allocation.height - height, layout);
  }

  g_object_unref(layout);
  g_object_unref(gc);
}

/**
 * Draws in the scale labels.
 */
gboolean GtkStatsPianoRoll::
expose_event_callback(GtkWidget *widget, GdkEventExpose *event, gpointer data) {
  GtkStatsPianoRoll *self = (GtkStatsPianoRoll *)data;
  self->draw_guide_labels();

  return TRUE;
}
