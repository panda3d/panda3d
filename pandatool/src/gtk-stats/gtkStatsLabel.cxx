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
#include "convert_srgb.h"

int GtkStatsLabel::_left_margin = 6;
int GtkStatsLabel::_right_margin = 2;
int GtkStatsLabel::_top_margin = 1;
int GtkStatsLabel::_bottom_margin = 1;

/**
 *
 */
GtkStatsLabel::
GtkStatsLabel(GtkStatsMonitor *monitor, GtkStatsGraph *graph,
              int thread_index, int collector_index, bool use_fullname,
              bool align_right) :
  _monitor(monitor),
  _graph(graph),
  _thread_index(thread_index),
  _collector_index(collector_index),
  _align_right(align_right)
{
  _widget = gtk_drawing_area_new();
  gtk_widget_add_events(_widget,
      GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK |
      GDK_BUTTON_PRESS_MASK);
  g_signal_connect(G_OBJECT(_widget), "draw",
       G_CALLBACK(draw_callback), this);
  g_signal_connect(G_OBJECT(_widget), "enter_notify_event",
       G_CALLBACK(enter_notify_event_callback), this);
  g_signal_connect(G_OBJECT(_widget), "leave_notify_event",
       G_CALLBACK(leave_notify_event_callback), this);
  g_signal_connect(G_OBJECT(_widget), "button_press_event",
       G_CALLBACK(button_press_event_callback), this);
  g_signal_connect(G_OBJECT(_widget), "query-tooltip",
       G_CALLBACK(query_tooltip_callback), this);

  gtk_widget_set_has_tooltip(_widget, TRUE);

  _highlight = false;
  _mouse_within = false;

  update_color();
  update_text(use_fullname);
  gtk_widget_show_all(_widget);
}

/**
 *
 */
GtkStatsLabel::
~GtkStatsLabel() {
  if (_layout) {
    g_object_unref(_layout);
    _layout = nullptr;
  }
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
 * Updates the colors.
 */
void GtkStatsLabel::
update_color() {
  // Set the fg and bg colors on the label.
  LRGBColor rgb = _monitor->get_collector_color(_collector_index);
  _bg_color = LRGBColor(
    encode_sRGB_float((float)rgb[0]),
    encode_sRGB_float((float)rgb[1]),
    encode_sRGB_float((float)rgb[2]));

  _highlight_bg_color = LRGBColor(
    encode_sRGB_float((float)rgb[0] * 0.75f),
    encode_sRGB_float((float)rgb[1] * 0.75f),
    encode_sRGB_float((float)rgb[2] * 0.75f));

  // Should our foreground be black or white?
  PN_stdfloat bright = _bg_color.dot(LRGBColor(0.2126, 0.7152, 0.0722));
  if (bright >= 0.5) {
    _fg_color = LRGBColor(0);
  } else {
    _fg_color = LRGBColor(1);
  }
  if (bright * 0.75 >= 0.5) {
    _highlight_fg_color = LRGBColor(0);
  } else {
    _highlight_fg_color = LRGBColor(1);
  }

  gtk_widget_queue_draw(_widget);
}

/**
 * Set to true if the full name of the collector should be shown.
 */
void GtkStatsLabel::
update_text(bool use_fullname) {
  const PStatClientData *client_data = _monitor->get_client_data();
  if (use_fullname) {
    _text = client_data->get_collector_fullname(_collector_index);
  } else {
    _text = client_data->get_collector_name(_collector_index);
  }

  // Make up a PangoLayout to represent the text.
  if (_layout) {
    g_object_unref(_layout);
  }
  _layout = gtk_widget_create_pango_layout(_widget, _text.c_str());

  // What are the extents of the text?  This determines the minimum size of
  // our widget.
  int width, height;
  pango_layout_get_pixel_size(_layout, &width, &height);
  _ideal_width = width + _left_margin + _right_margin;
  _height = height + _top_margin + _bottom_margin;
  gtk_widget_set_size_request(_widget, _ideal_width, _height);
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
draw_callback(GtkWidget *widget, cairo_t *cr, gpointer data) {
  GtkStatsLabel *self = (GtkStatsLabel *)data;

  LRGBColor bg, fg;
  if (self->_highlight || self->_mouse_within) {
    bg = self->_highlight_bg_color;
    fg = self->_highlight_fg_color;
  } else {
    bg = self->_bg_color;
    fg = self->_fg_color;

  }
  cairo_set_source_rgb(cr, bg[0], bg[1], bg[2]);

  GtkAllocation allocation;
  gtk_widget_get_allocation(widget, &allocation);

  cairo_rectangle(cr, 0, 0, allocation.width, allocation.height);
  cairo_fill(cr);

  int width, height;
  pango_layout_get_pixel_size(self->_layout, &width, &height);

  cairo_set_source_rgb(cr, fg[0], fg[1], fg[2]);
  if (self->_align_right) {
    cairo_move_to(cr, allocation.width - width - _right_margin, _top_margin);
  } else {
    cairo_move_to(cr, _left_margin, _top_margin);
  }
  pango_cairo_show_layout(cr, self->_layout);

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
  if (event->type == GDK_2BUTTON_PRESS && event->button == 1) {
    self->_graph->on_click_label(self->_collector_index);
  }
  else if (event->type == GDK_BUTTON_PRESS && event->button == 3) {
    self->_graph->on_popup_label(self->_collector_index);
  }
  return TRUE;
}

/**
 * Called when a tooltip should be displayed.
 */
gboolean GtkStatsLabel::
query_tooltip_callback(GtkWidget *widget, gint x, gint y,
                       gboolean keyboard_tip, GtkTooltip *tooltip,
                       gpointer data) {
  GtkStatsLabel *self = (GtkStatsLabel *)data;

  std::string text = self->_graph->get_label_tooltip(self->_collector_index);
  gtk_tooltip_set_text(tooltip, text.c_str());
  return !text.empty();
}
