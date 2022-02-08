/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file gtkStatsFlameGraph.cxx
 * @author rdb
 * @date 2022-02-02
 */

#include "gtkStatsFlameGraph.h"
#include "gtkStatsLabel.h"
#include "gtkStatsMonitor.h"
#include "pStatCollectorDef.h"

static const int default_flame_graph_width = 800;
static const int default_flame_graph_height = 150;

/**
 *
 */
GtkStatsFlameGraph::
GtkStatsFlameGraph(GtkStatsMonitor *monitor, int thread_index,
                   int collector_index) :
  PStatFlameGraph(monitor, monitor->get_view(thread_index),
                  thread_index, collector_index,
                  default_flame_graph_width,
                  default_flame_graph_height),
  GtkStatsGraph(monitor)
{
  // Let's show the units on the guide bar labels.  There's room.
  set_guide_bar_units(get_guide_bar_units() | GBU_show_units);

  // Put some stuff on top of the graph.
  _top_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_box_pack_start(GTK_BOX(_graph_vbox), _top_hbox,
         FALSE, FALSE, 0);

  _average_check_box = gtk_check_button_new_with_label("Average");
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(_average_check_box), TRUE);
  g_signal_connect(G_OBJECT(_average_check_box), "toggled",
       G_CALLBACK(toggled_callback), this);

  // Add a DrawingArea widget on top of the graph, to display all of the scale
  // units.
  _scale_area = gtk_drawing_area_new();
  g_signal_connect(G_OBJECT(_scale_area), "draw", G_CALLBACK(draw_callback), this);

  _total_label = gtk_label_new("");
  gtk_box_pack_start(GTK_BOX(_top_hbox), _average_check_box, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(_top_hbox), _scale_area, TRUE, TRUE, 0);
  gtk_box_pack_end(GTK_BOX(_top_hbox), _total_label, FALSE, FALSE, 0);

  gtk_widget_set_size_request(_graph_window, default_flame_graph_width,
                              default_flame_graph_height);

  // Add a fixed container to the overlay to allow arbitrary positioning
  // of labels therein.
  _fixed = gtk_fixed_new();
  gtk_overlay_add_overlay(GTK_OVERLAY(_graph_overlay), _fixed);

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
GtkStatsFlameGraph::
~GtkStatsFlameGraph() {
}

/**
 * Called whenever a new Collector definition is received from the client.
 */
void GtkStatsFlameGraph::
new_collector(int collector_index) {
  GtkStatsGraph::new_collector(collector_index);
}

/**
 * Called as each frame's data is made available.  There is no guarantee the
 * frames will arrive in order, or that all of them will arrive at all.  The
 * monitor should be prepared to accept frames received out-of-order or
 * missing.
 */
void GtkStatsFlameGraph::
new_data(int thread_index, int frame_number) {
  if (is_title_unknown()) {
    std::string window_title = get_title_text();
    if (!is_title_unknown()) {
      gtk_window_set_title(GTK_WINDOW(_window), window_title.c_str());
    }
  }

  if (!_pause) {
    update();

    std::string text = format_number(get_horizontal_scale(), get_guide_bar_units(), get_guide_bar_unit_name());
    if (_net_value_text != text) {
      _net_value_text = text;
      gtk_label_set_text(GTK_LABEL(_total_label), _net_value_text.c_str());
    }
  }
}

/**
 * Called when it is necessary to redraw the entire graph.
 */
void GtkStatsFlameGraph::
force_redraw() {
  PStatFlameGraph::force_redraw();
}

/**
 * Called when the user has resized the window, forcing a resize of the graph.
 */
void GtkStatsFlameGraph::
changed_graph_size(int graph_xsize, int graph_ysize) {
  PStatFlameGraph::changed_size(graph_xsize, graph_ysize);
}

/**
 * Called when the user selects a new time units from the monitor pulldown
 * menu, this should adjust the units for the graph to the indicated mask if
 * it is a time-based graph.
 */
void GtkStatsFlameGraph::
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
void GtkStatsFlameGraph::
on_click_label(int collector_index) {
  int prev_collector_index = get_collector_index();
  if (collector_index == prev_collector_index && collector_index != 0) {
    // Clicking on the top label means to go up to the parent level.
    const PStatClientData *client_data =
      GtkStatsGraph::_monitor->get_client_data();
    if (client_data->has_collector(collector_index)) {
      const PStatCollectorDef &def =
        client_data->get_collector_def(collector_index);
      collector_index = def._parent_index;
      set_collector_index(collector_index);
    }
  }
  else {
    // Clicking on any other label means to focus on that.
    set_collector_index(collector_index);
  }

  // Change the root collector to show the full name.
  if (prev_collector_index != collector_index) {
    auto it = _labels.find(prev_collector_index);
    if (it != _labels.end()) {
      it->second->update_text(false);
    }
    it = _labels.find(collector_index);
    if (it != _labels.end()) {
      it->second->update_text(true);
    }
  }
}

/**
 * Called when the user hovers the mouse over a label.
 */
void GtkStatsFlameGraph::
on_enter_label(int collector_index) {
  if (collector_index != _highlighted_index) {
    _highlighted_index = collector_index;
  }
}

/**
 * Called when the user's mouse cursor leaves a label.
 */
void GtkStatsFlameGraph::
on_leave_label(int collector_index) {
  if (collector_index == _highlighted_index && collector_index != -1) {
    _highlighted_index = -1;
  }
}

/**
 * Called when the mouse hovers over a label, and should return the text that
 * should appear on the tooltip.
 */
std::string GtkStatsFlameGraph::
get_label_tooltip(int collector_index) const {
  return PStatFlameGraph::get_label_tooltip(collector_index);
}

/**
 * Repositions the labels.
 */
void GtkStatsFlameGraph::
update_labels() {
  PStatFlameGraph::update_labels();
}

/**
 * Repositions a label.  If width is 0, the label should be deleted.
 */
void GtkStatsFlameGraph::
update_label(int collector_index, int row, int x, int width) {
  GtkStatsLabel *label;

  auto it = _labels.find(collector_index);
  if (it != _labels.end()) {
    label = it->second;
    if (width == 0) {
      gtk_container_remove(GTK_CONTAINER(_fixed), label->get_widget());
      delete label;
      _labels.erase(it);
      return;
    }
    gtk_fixed_move(GTK_FIXED(_fixed), label->get_widget(), x, _ysize - (row + 1) * label->get_height());
  }
  else {
    if (width == 0) {
      return;
    }
    label = new GtkStatsLabel(GtkStatsGraph::_monitor, this, _thread_index, collector_index, false, false);
    _labels[collector_index] = label;
    gtk_fixed_put(GTK_FIXED(_fixed), label->get_widget(), x, _ysize - (row + 1) * label->get_height());
  }

  gtk_widget_set_size_request(label->get_widget(), std::min(width, _xsize), label->get_height());
}

/**
 * Calls update_guide_bars with parameters suitable to this kind of graph.
 */
void GtkStatsFlameGraph::
normal_guide_bars() {
  // We want vaguely 100 pixels between guide bars.
  double res = gdk_screen_get_resolution(gdk_screen_get_default());
  int num_bars = (int)(get_xsize() / (100.0 * (res > 0 ? res / 96.0 : 1.0)));

  _guide_bars.clear();

  double dist = get_horizontal_scale() / num_bars;

  for (int i = 1; i < num_bars; ++i) {
    _guide_bars.push_back(make_guide_bar(i * dist));
  }

  _guide_bars_changed = true;
}

/**
 * Erases the chart area.
 */
void GtkStatsFlameGraph::
clear_region() {
  cairo_set_source_rgb(_cr, 1.0, 1.0, 1.0);
  cairo_paint(_cr);
}

/**
 * Erases the chart area in preparation for drawing a bunch of bars.
 */
void GtkStatsFlameGraph::
begin_draw() {
  clear_region();

  // Draw in the guide bars.
  int num_guide_bars = get_num_guide_bars();
  for (int i = 0; i < num_guide_bars; i++) {
    draw_guide_bar(_cr, get_guide_bar(i));
  }
}

/**
 * Called after all the bars have been drawn, this triggers a refresh event to
 * draw it to the window.
 */
void GtkStatsFlameGraph::
end_draw() {
  gtk_widget_queue_draw(_graph_window);
}

/**
 * Called at the end of the draw cycle.
 */
void GtkStatsFlameGraph::
idle() {
}

/**
 * This is called during the servicing of the draw event; it gives a derived
 * class opportunity to do some further painting into the graph window.
 */
void GtkStatsFlameGraph::
additional_graph_window_paint(cairo_t *cr) {
  int num_user_guide_bars = get_num_user_guide_bars();
  for (int i = 0; i < num_user_guide_bars; i++) {
    draw_guide_bar(cr, get_user_guide_bar(i));
  }
}

/**
 * Based on the mouse position within the window's client area, look for
 * draggable things the mouse might be hovering over and return the
 * apprioprate DragMode enum or DM_none if nothing is indicated.
 */
GtkStatsGraph::DragMode GtkStatsFlameGraph::
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

  return DM_none;
}

/**
 * Called when the mouse button is depressed within the graph window.
 */
gboolean GtkStatsFlameGraph::
handle_button_press(GtkWidget *widget, int graph_x, int graph_y,
        bool double_click) {
  if (graph_x >= 0 && graph_y >= 0 && graph_x < get_xsize() && graph_y < get_ysize()) {
    if (double_click) {
      // Clicking on whitespace in the graph goes to the parent.
      on_click_label(get_collector_index());
      return TRUE;
    }
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
gboolean GtkStatsFlameGraph::
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
gboolean GtkStatsFlameGraph::
handle_motion(GtkWidget *widget, int graph_x, int graph_y) {
  if (_drag_mode == DM_new_guide_bar) {
    // We haven't created the new guide bar yet; we won't until the mouse
    // comes within the graph's region.
    if (graph_x >= 0 && graph_x < get_xsize()) {
      set_drag_mode(DM_guide_bar);
      _drag_guide_bar = add_user_guide_bar(pixel_to_height(graph_x));
      return TRUE;
    }
  }
  else if (_drag_mode == DM_guide_bar) {
    move_user_guide_bar(_drag_guide_bar, pixel_to_height(graph_x));
    return TRUE;
  }

  return GtkStatsGraph::handle_motion(widget, graph_x, graph_y);
}

/**
 * Draws the line for the indicated guide bar on the graph.
 */
void GtkStatsFlameGraph::
draw_guide_bar(cairo_t *cr, const PStatGraph::GuideBar &bar) {
  int x = height_to_pixel(bar._height);

  if (x > 0 && x < get_xsize() - 1) {
    // Only draw it if it's not too close to the top.
    switch (bar._style) {
    case GBS_target:
      cairo_set_source_rgb(cr, rgb_light_gray[0], rgb_light_gray[1], rgb_light_gray[2]);
      break;

    case GBS_user:
      cairo_set_source_rgb(cr, rgb_user_guide_bar[0], rgb_user_guide_bar[1], rgb_user_guide_bar[2]);
      break;

    case GBS_normal:
      cairo_set_source_rgb(cr, rgb_dark_gray[0], rgb_dark_gray[1], rgb_dark_gray[2]);
      break;
    }
    cairo_move_to(cr, x, 0);
    cairo_line_to(cr, x, get_ysize());
    cairo_stroke(cr);
  }
}

/**
 * This is called during the servicing of the draw event.
 */
void GtkStatsFlameGraph::
draw_guide_labels(cairo_t *cr) {
  int i;
  int num_guide_bars = get_num_guide_bars();
  for (i = 0; i < num_guide_bars; i++) {
    draw_guide_label(cr, get_guide_bar(i));
  }

  int num_user_guide_bars = get_num_user_guide_bars();
  for (i = 0; i < num_user_guide_bars; i++) {
    draw_guide_label(cr, get_user_guide_bar(i));
  }
}

/**
 * Draws the text for the indicated guide bar label at the top of the graph.
 */
void GtkStatsFlameGraph::
draw_guide_label(cairo_t *cr, const PStatGraph::GuideBar &bar) {
  switch (bar._style) {
  case GBS_target:
    cairo_set_source_rgb(cr, rgb_light_gray[0], rgb_light_gray[1], rgb_light_gray[2]);
    break;

  case GBS_user:
    cairo_set_source_rgb(cr, rgb_user_guide_bar[0], rgb_user_guide_bar[1], rgb_user_guide_bar[2]);
    break;

  case GBS_normal:
    cairo_set_source_rgb(cr, rgb_dark_gray[0], rgb_dark_gray[1], rgb_dark_gray[2]);
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

    GtkAllocation allocation;
    gtk_widget_get_allocation(_scale_area, &allocation);

    int this_x = x - width / 2;
    if (this_x >= 0 && this_x + width < allocation.width) {
      cairo_move_to(cr, this_x, allocation.height - height);
      pango_cairo_show_layout(cr, layout);
    }
  }

  g_object_unref(layout);
}

/**
 * Called when the average check box is toggled.
 */
void GtkStatsFlameGraph::
toggled_callback(GtkToggleButton *button, gpointer data) {
  GtkStatsFlameGraph *self = (GtkStatsFlameGraph *)data;

  bool active = gtk_toggle_button_get_active(button);
  self->set_average_mode(active);
}

/**
 * Draws in the scale labels.
 */
gboolean GtkStatsFlameGraph::
draw_callback(GtkWidget *widget, cairo_t *cr, gpointer data) {
  GtkStatsFlameGraph *self = (GtkStatsFlameGraph *)data;
  self->draw_guide_labels(cr);

  return TRUE;
}
