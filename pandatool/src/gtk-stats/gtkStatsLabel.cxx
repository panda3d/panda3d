/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file gtkStatsLabel.cxx
 * @author drose
 * @date 2006-01-16
 */

#include "gtkStatsLabel.h"
#include "gtkStatsMonitor.h"
#include "gtkStatsGraph.h"

int GtkStatsLabel::_left_margin = 2;
int GtkStatsLabel::_right_margin = 2;
int GtkStatsLabel::_top_margin = 2;
int GtkStatsLabel::_bottom_margin = 2;

/**
 *
 */
GtkStatsLabel::
GtkStatsLabel(GtkStatsMonitor *monitor, GtkStatsGraph *graph,
              int thread_index, int collector_index, bool use_fullname) :
  _monitor(monitor),
  _graph(graph),
  _thread_index(thread_index),
  _collector_index(collector_index)
{
  _widget = nullptr;
  if (use_fullname) {
    _text = _monitor->get_client_data()->get_collector_fullname(_collector_index);
  } else {
    _text = _monitor->get_client_data()->get_collector_name(_collector_index);
  }

  _widget = gtk_drawing_area_new();
  gtk_widget_add_events(_widget,
      GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK |
      GDK_BUTTON_PRESS_MASK);
  g_signal_connect(G_OBJECT(_widget), "expose_event",
       G_CALLBACK(expose_event_callback), this);
  g_signal_connect(G_OBJECT(_widget), "enter_notify_event",
       G_CALLBACK(enter_notify_event_callback), this);
  g_signal_connect(G_OBJECT(_widget), "leave_notify_event",
       G_CALLBACK(leave_notify_event_callback), this);
  g_signal_connect(G_OBJECT(_widget), "button_press_event",
       G_CALLBACK(button_press_event_callback), this);

  gtk_widget_show(_widget);

  // Make up a PangoLayout to represent the text.
  _layout = gtk_widget_create_pango_layout(_widget, _text.c_str());

  // Set the fg and bg colors on the label.
  LRGBColor rgb = _monitor->get_collector_color(_collector_index);
  _bg_color.red = (int)(rgb[0] * 65535.0f);
  _bg_color.green = (int)(rgb[1] * 65535.0f);
  _bg_color.blue = (int)(rgb[2] * 65535.0f);

  // Should our foreground be black or white?
  double bright =
    rgb[0] * 0.299 +
    rgb[1] * 0.587 +
    rgb[2] * 0.114;

  if (bright >= 0.5) {
    _fg_color.red = _fg_color.green = _fg_color.blue = 0;
  } else {
    _fg_color.red = _fg_color.green = _fg_color.blue = 0xffff;
  }

  // What are the extents of the text?  This determines the minimum size of
  // our widget.
  int width, height;
  pango_layout_get_pixel_size(_layout, &width, &height);
  gtk_widget_set_size_request(_widget, width + 8, height);

  _highlight = false;
  _mouse_within = false;
  _height = height;
}

/**
 *
 */
GtkStatsLabel::
~GtkStatsLabel() {
  // DeleteObject(_bg_brush);
}

/**
 * Returns the widget for this label.
 */
GtkWidget *GtkStatsLabel::
get_widget() const {
  return _widget;
}

/**
 * Returns the height of the label as we requested it.
 */
int GtkStatsLabel::
get_height() const {
  return _height;
}

/**
 * Returns the collector this label represents.
 */
int GtkStatsLabel::
get_collector_index() const {
  return _collector_index;
}

/**
 * Returns the thread index.
 */
int GtkStatsLabel::
get_thread_index() const {
  return _thread_index;
}

/**
 * Enables or disables the visual highlight for this label.
 */
void GtkStatsLabel::
set_highlight(bool highlight) {
  if (_highlight != highlight) {
    _highlight = highlight;
    gtk_widget_queue_draw(_widget);
  }
}

/**
 * Returns true if the visual highlight for this label is enabled.
 */
bool GtkStatsLabel::
get_highlight() const {
  return _highlight;
}

/**
 * Used internally to indicate whether the mouse is within the label's widget.
 */
void GtkStatsLabel::
set_mouse_within(bool mouse_within) {
  if (_mouse_within != mouse_within) {
    _mouse_within = mouse_within;
    gtk_widget_queue_draw(_widget);
  }
}

/**
 * Draws the background color of the label.
 */
gboolean GtkStatsLabel::
expose_event_callback(GtkWidget *widget, GdkEventExpose *event, gpointer data) {
  GtkStatsLabel *self = (GtkStatsLabel *)data;

  GdkGC *gc = gdk_gc_new(widget->window);
  gdk_gc_set_rgb_fg_color(gc, &self->_bg_color);

  gdk_draw_rectangle(widget->window, gc, TRUE, 0, 0,
         widget->allocation.width, widget->allocation.height);

  // Center the text within the rectangle.
  int width, height;
  pango_layout_get_pixel_size(self->_layout, &width, &height);

  gdk_gc_set_rgb_fg_color(gc, &self->_fg_color);
  gdk_draw_layout(widget->window, gc,
      (widget->allocation.width - width) / 2, 0,
      self->_layout);

  // Now draw the highlight rectangle, if any.
  if (self->_highlight || self->_mouse_within) {
    gdk_draw_rectangle(widget->window, gc, FALSE, 0, 0,
           widget->allocation.width - 1, widget->allocation.height - 1);
  }

  g_object_unref(gc);
  return TRUE;
}

/**
 * Called when the mouse enters the label region
 */
gboolean GtkStatsLabel::
enter_notify_event_callback(GtkWidget *widget, GdkEventCrossing *event,
          gpointer data) {
  GtkStatsLabel *self = (GtkStatsLabel *)data;
  self->set_mouse_within(true);
  return TRUE;
}

/**
 * Called when the mouse leaves the label region
 */
gboolean GtkStatsLabel::
leave_notify_event_callback(GtkWidget *widget, GdkEventCrossing *event,
          gpointer data) {
  GtkStatsLabel *self = (GtkStatsLabel *)data;
  self->set_mouse_within(false);
  return TRUE;
}

/**
 * Called when the mouse button is depressed within the label.
 */
gboolean GtkStatsLabel::
button_press_event_callback(GtkWidget *widget, GdkEventButton *event,
          gpointer data) {
  GtkStatsLabel *self = (GtkStatsLabel *)data;
  bool double_click = (event->type == GDK_2BUTTON_PRESS);
  if (double_click) {
    self->_graph->clicked_label(self->_collector_index);
  }
  return TRUE;
}
