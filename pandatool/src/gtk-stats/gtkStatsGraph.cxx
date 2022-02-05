/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file gtkStatsGraph.cxx
 * @author drose
 * @date 2006-01-16
 */

#include "gtkStatsGraph.h"
#include "gtkStatsMonitor.h"
#include "gtkStatsLabelStack.h"
#include "convert_srgb.h"

const double GtkStatsGraph::rgb_light_gray[3] = {
  0x9a / (double)0xff, 0x9a / (double)0xff, 0x9a / (double)0xff,
};
const double GtkStatsGraph::rgb_dark_gray[3] = {
  0x33 / (double)0xff, 0x33 / (double)0xff, 0x33 / (double)0xff,
};
const double GtkStatsGraph::rgb_user_guide_bar[3] = {
  0x82 / (double)0xff, 0x96 / (double)0xff, 0xff / (double)0xff,
};

/**
 *
 */
GtkStatsGraph::
GtkStatsGraph(GtkStatsMonitor *monitor) :
  _monitor(monitor)
{
  _parent_window = nullptr;
  _window = nullptr;
  _graph_window = nullptr;
  _scale_area = nullptr;

  GtkWidget *parent_window = monitor->get_window();

  GdkDisplay *display = gdk_window_get_display(gtk_widget_get_window(parent_window));
  _hand_cursor = gdk_cursor_new_for_display(display, GDK_HAND2);

  _cr_surface = nullptr;
  _cr = nullptr;

  _surface_xsize = 0;
  _surface_ysize = 0;

  _window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

  // These calls were intended to kind of emulate the Windows MDI behavior,
  // but it's just weird.  gtk_window_set_transient_for(GTK_WINDOW(_window),
  // GTK_WINDOW(parent_window));
  // gtk_window_set_destroy_with_parent(GTK_WINDOW(_window), TRUE);

  gtk_widget_add_events(_window,
      GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
      GDK_POINTER_MOTION_MASK);
  g_signal_connect(G_OBJECT(_window), "delete_event",
       G_CALLBACK(window_delete_event), this);
  g_signal_connect(G_OBJECT(_window), "destroy",
       G_CALLBACK(window_destroy), this);
  g_signal_connect(G_OBJECT(_window), "button_press_event",
       G_CALLBACK(button_press_event_callback), this);
  g_signal_connect(G_OBJECT(_window), "button_release_event",
       G_CALLBACK(button_release_event_callback), this);
  g_signal_connect(G_OBJECT(_window), "motion_notify_event",
       G_CALLBACK(motion_notify_event_callback), this);

  _graph_window = gtk_drawing_area_new();
  gtk_widget_add_events(_graph_window,
      GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
      GDK_POINTER_MOTION_MASK);
  g_signal_connect(G_OBJECT(_graph_window), "draw",
       G_CALLBACK(graph_draw_callback), this);
  g_signal_connect(G_OBJECT(_graph_window), "configure_event",
       G_CALLBACK(configure_graph_callback), this);
  g_signal_connect(G_OBJECT(_graph_window), "button_press_event",
       G_CALLBACK(button_press_event_callback), this);
  g_signal_connect(G_OBJECT(_graph_window), "button_release_event",
       G_CALLBACK(button_release_event_callback), this);
  g_signal_connect(G_OBJECT(_graph_window), "motion_notify_event",
       G_CALLBACK(motion_notify_event_callback), this);

  // An overlay inside the frame, for charts that want to display widgets on
  // top of the graph.
  _graph_overlay = gtk_overlay_new();
  gtk_container_add(GTK_CONTAINER(_graph_overlay), _graph_window);

  // A Frame to hold the graph.
  _graph_frame = gtk_frame_new(nullptr);
  gtk_frame_set_shadow_type(GTK_FRAME(_graph_frame), GTK_SHADOW_IN);
  gtk_container_add(GTK_CONTAINER(_graph_frame), _graph_overlay);

  // A VBox to hold the graph's frame, and any numbers (scale legend?  total?)
  // above it.
  _graph_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_pack_end(GTK_BOX(_graph_vbox), _graph_frame,
       TRUE, TRUE, 0);

  // An HBox to hold the graph's frame, and the scale legend to the right of
  // it.
  _graph_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_box_pack_start(GTK_BOX(_graph_hbox), _graph_vbox,
         TRUE, TRUE, 0);

  // An HPaned to hold the label stack and the graph hbox.
  _hpaned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
  gtk_paned_set_wide_handle(GTK_PANED(_hpaned), TRUE);
  gtk_container_add(GTK_CONTAINER(_window), _hpaned);
  gtk_container_set_border_width(GTK_CONTAINER(_window), 8);

  gtk_paned_pack1(GTK_PANED(_hpaned), _label_stack.get_widget(), FALSE, FALSE);
  gtk_paned_pack2(GTK_PANED(_hpaned), _graph_hbox, TRUE, TRUE);

  _drag_mode = DM_none;
  _potential_drag_mode = DM_none;
  _drag_scale_start = 0.0f;

  _pause = false;
}

/**
 *
 */
GtkStatsGraph::
~GtkStatsGraph() {
  _monitor = nullptr;
  release_surface();

  for (auto &item : _brushes) {
    cairo_pattern_destroy(item.second.first);
    cairo_pattern_destroy(item.second.second);
  }
  _brushes.clear();

  _label_stack.clear_labels();

  if (_window != nullptr) {
    GtkWidget *window = _window;
    _window = nullptr;
    gtk_widget_destroy(window);
  }
}

/**
 * Called whenever a new Collector definition is received from the client.
 */
void GtkStatsGraph::
new_collector(int new_collector) {
}

/**
 * Called whenever new data arrives.
 */
void GtkStatsGraph::
new_data(int thread_index, int frame_number) {
}

/**
 * Called when the user has resized the window, forcing a resize of the graph.
 */
void GtkStatsGraph::
changed_graph_size(int graph_xsize, int graph_ysize) {
}

/**
 * Called when the user selects a new time units from the monitor pulldown
 * menu, this should adjust the units for the graph to the indicated mask if
 * it is a time-based graph.
 */
void GtkStatsGraph::
set_time_units(int unit_mask) {
}

/**
 * Called when the user selects a new scroll speed from the monitor pulldown
 * menu, this should adjust the speed for the graph to the indicated value.
 */
void GtkStatsGraph::
set_scroll_speed(double scroll_speed) {
}

/**
 * Changes the pause flag for the graph.  When this flag is true, the graph
 * does not update in response to new data.
 */
void GtkStatsGraph::
set_pause(bool pause) {
  _pause = pause;
}

/**
 * Called when the user guide bars have been changed.
 */
void GtkStatsGraph::
user_guide_bars_changed() {
  if (_scale_area != nullptr) {
    gtk_widget_queue_draw(_scale_area);
  }
  gtk_widget_queue_draw(_graph_window);
}

/**
 * Called when the user single-clicks on a label.
 */
void GtkStatsGraph::
on_click_label(int collector_index) {
}

/**
 * Called when the user hovers the mouse over a label.
 */
void GtkStatsGraph::
on_enter_label(int collector_index) {
  if (collector_index != _highlighted_index) {
    _highlighted_index = collector_index;
    force_redraw();
  }
}

/**
 * Called when the user's mouse cursor leaves a label.
 */
void GtkStatsGraph::
on_leave_label(int collector_index) {
  if (collector_index == _highlighted_index && collector_index != -1) {
    _highlighted_index = -1;
    force_redraw();
  }
}

/**
 * Called when the mouse hovers over a label, and should return the text that
 * should appear on the tooltip.
 */
std::string GtkStatsGraph::
get_label_tooltip(int collector_index) const {
  return std::string();
}

/**
 * Should be called when the user closes the associated window.  This tells
 * the monitor to remove the graph.
 */
void GtkStatsGraph::
close() {
  _label_stack.clear_labels(false);
  if (_window != nullptr) {
    _window = nullptr;

    GtkStatsMonitor *monitor = _monitor;
    _monitor = nullptr;
    if (monitor != nullptr) {
      monitor->remove_graph(this);
    }
  }
}

/**
 * Returns a pattern suitable for drawing in the indicated collector's color.
 */
cairo_pattern_t *GtkStatsGraph::
get_collector_pattern(int collector_index, bool highlight) {
  Brushes::iterator bi;
  bi = _brushes.find(collector_index);
  if (bi != _brushes.end()) {
    return highlight ? (*bi).second.second : (*bi).second.first;
  }

  // Ask the monitor what color this guy should be.
  LRGBColor rgb = _monitor->get_collector_color(collector_index);
  cairo_pattern_t *pattern = cairo_pattern_create_rgb(
    encode_sRGB_float((float)rgb[0]),
    encode_sRGB_float((float)rgb[1]),
    encode_sRGB_float((float)rgb[2]));
  cairo_pattern_t *hpattern = cairo_pattern_create_rgb(
    encode_sRGB_float((float)rgb[0] * 0.75f),
    encode_sRGB_float((float)rgb[1] * 0.75f),
    encode_sRGB_float((float)rgb[2] * 0.75f));

  _brushes[collector_index] = std::make_pair(pattern, hpattern);
  return highlight ? hpattern : pattern;
}

/**
 * This is called during the servicing of the draw event; it gives a derived
 * class opportunity to do some further painting into the graph window.
 */
void GtkStatsGraph::
additional_graph_window_paint(cairo_t *cr) {
}

/**
 * Based on the mouse position within the graph window, look for draggable
 * things the mouse might be hovering over and return the appropriate DragMode
 * enum or DM_none if nothing is indicated.
 */
GtkStatsGraph::DragMode GtkStatsGraph::
consider_drag_start(int graph_x, int graph_y) {
  return DM_none;
}

/**
 * This should be called whenever the drag mode needs to change state.  It
 * provides hooks for a derived class to do something special.
 */
void GtkStatsGraph::
set_drag_mode(GtkStatsGraph::DragMode drag_mode) {
  _drag_mode = drag_mode;
}

/**
 * Called when the mouse button is depressed within the window, or any nested
 * window.
 */
gboolean GtkStatsGraph::
handle_button_press(GtkWidget *widget, int graph_x, int graph_y,
        bool double_click) {
  if (_potential_drag_mode != DM_none) {
    set_drag_mode(_potential_drag_mode);
    _drag_start_x = graph_x;
    _drag_start_y = graph_y;
    // SetCapture(_window);
  }
  return TRUE;
}

/**
 * Called when the mouse button is released within the window, or any nested
 * window.
 */
gboolean GtkStatsGraph::
handle_button_release(GtkWidget *widget, int graph_x, int graph_y) {
  set_drag_mode(DM_none);
  // ReleaseCapture();

  return handle_motion(widget, graph_x, graph_y);
}

/**
 * Called when the mouse is moved within the window, or any nested window.
 */
gboolean GtkStatsGraph::
handle_motion(GtkWidget *widget, int graph_x, int graph_y) {
  _potential_drag_mode = consider_drag_start(graph_x, graph_y);

  GdkWindow *window = gtk_widget_get_window(_window);

  if (_potential_drag_mode == DM_guide_bar ||
      _drag_mode == DM_guide_bar) {
    gdk_window_set_cursor(window, _hand_cursor);

  } else {
    gdk_window_set_cursor(window, nullptr);
  }

  return TRUE;
}

/**
 * Sets up a backing-store bitmap of the indicated size.
 */
void GtkStatsGraph::
setup_surface(int xsize, int ysize) {
  release_surface();

  _surface_xsize = std::max(xsize, 0);
  _surface_ysize = std::max(ysize, 0);

  _cr_surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, _surface_xsize, _surface_ysize);
  _cr = cairo_create(_cr_surface);

  cairo_set_source_rgb(_cr, 1.0, 1.0, 1.0);
  cairo_paint(_cr);
}

/**
 * Frees the backing-store bitmap created by setup_surface().
 */
void GtkStatsGraph::
release_surface() {
  if (_cr_surface != nullptr) {
    cairo_surface_destroy(_cr_surface);
    cairo_destroy(_cr);
  }
}

/**
 * Callback when the window is closed by the user.
 */
gboolean GtkStatsGraph::
window_delete_event(GtkWidget *widget, GdkEvent *event, gpointer data) {
  // Returning FALSE to indicate we should destroy the window when the user
  // selects "close".
  return FALSE;
}

/**
 * Callback when the window is destroyed by the system (or by delete_event).
 */
void GtkStatsGraph::
window_destroy(GtkWidget *widget, gpointer data) {
  GtkStatsGraph *self = (GtkStatsGraph *)data;
  self->close();
}

/**
 * Fills in the graph window.
 */
gboolean GtkStatsGraph::
graph_draw_callback(GtkWidget *widget, cairo_t *cr, gpointer data) {
  GtkStatsGraph *self = (GtkStatsGraph *)data;

  if (self->_cr_surface != nullptr) {
    cairo_set_source_surface(cr, self->_cr_surface, 0, 0);
    cairo_paint(cr);
  }

  self->additional_graph_window_paint(cr);

  return TRUE;
}

/**
 * Changes the size of the graph window
 */
gboolean GtkStatsGraph::
configure_graph_callback(GtkWidget *widget, GdkEventConfigure *event,
       gpointer data) {
  GtkStatsGraph *self = (GtkStatsGraph *)data;

  self->changed_graph_size(event->width, event->height);
  self->setup_surface(event->width, event->height);
  self->force_redraw();

  return TRUE;
}

/**
 * Called when the mouse button is depressed within the graph window or main
 * window.
 */
gboolean GtkStatsGraph::
button_press_event_callback(GtkWidget *widget, GdkEventButton *event,
          gpointer data) {
  GtkStatsGraph *self = (GtkStatsGraph *)data;
  int graph_x, graph_y;
  gtk_widget_translate_coordinates(widget, self->_graph_window,
           (int)event->x, (int)event->y,
           &graph_x, &graph_y);

  bool double_click = (event->type == GDK_2BUTTON_PRESS);

  return self->handle_button_press(widget, graph_x, graph_y, double_click);
}

/**
 * Called when the mouse button is released within the graph window or main
 * window.
 */
gboolean GtkStatsGraph::
button_release_event_callback(GtkWidget *widget, GdkEventButton *event,
            gpointer data) {
  GtkStatsGraph *self = (GtkStatsGraph *)data;
  int graph_x, graph_y;
  gtk_widget_translate_coordinates(widget, self->_graph_window,
           (int)event->x, (int)event->y,
           &graph_x, &graph_y);

  return self->handle_button_release(widget, graph_x, graph_y);
}

/**
 * Called when the mouse is moved within the graph window or main window.
 */
gboolean GtkStatsGraph::
motion_notify_event_callback(GtkWidget *widget, GdkEventMotion *event,
           gpointer data) {
  GtkStatsGraph *self = (GtkStatsGraph *)data;
  int graph_x, graph_y;
  gtk_widget_translate_coordinates(widget, self->_graph_window,
           (int)event->x, (int)event->y,
           &graph_x, &graph_y);

  return self->handle_motion(widget, graph_x, graph_y);
}
