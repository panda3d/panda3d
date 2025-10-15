/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file gtkStatsTimeline.cxx
 * @author rdb
 * @date 2022-02-17
 */

#include "gtkStatsTimeline.h"
#include "gtkStatsMonitor.h"
#include "numeric_types.h"
#include "gtkStatsLabelStack.h"

static const int default_timeline_width = 1000;
static const int default_timeline_height = 300;

/**
 *
 */
GtkStatsTimeline::
GtkStatsTimeline(GtkStatsMonitor *monitor) :
  PStatTimeline(monitor, 0, 0),
  GtkStatsGraph(monitor, false)
{
  // Let's show the units on the guide bar labels.  There's room.
  set_guide_bar_units(get_guide_bar_units() | GBU_show_units);

  // Add a DrawingArea widget on top of the graph, to display all of the scale
  // units.
  _scale_area = gtk_drawing_area_new();
  g_signal_connect(G_OBJECT(_scale_area), "draw",
                   G_CALLBACK(scale_area_draw_callback), this);
  gtk_box_pack_start(GTK_BOX(_graph_vbox), _scale_area, FALSE, FALSE, 0);

  // It should be large enough to display the labels.
  {
    PangoLayout *layout = gtk_widget_create_pango_layout(_scale_area, "0123456789 ms");
    int width, height;
    pango_layout_get_pixel_size(layout, &width, &height);
    gtk_widget_set_size_request(_scale_area, 0, height + _pixel_scale / 2);
    g_object_unref(layout);
  }

  // Add a drawing area to the left of the graph to show the thread labels.
  _thread_area = gtk_drawing_area_new();
  gtk_box_pack_start(GTK_BOX(_graph_hbox), _thread_area, FALSE, FALSE, 0);
  gtk_box_reorder_child(GTK_BOX(_graph_hbox), _thread_area, 0);
  g_signal_connect(G_OBJECT(_thread_area), "draw",
                   G_CALLBACK(thread_area_draw_callback), this);

  // Listen for mouse wheel and keyboard events.
  gtk_widget_add_events(_graph_window, GDK_SMOOTH_SCROLL_MASK |
                                       GDK_SCROLL_MASK |
                                       GDK_KEY_PRESS_MASK |
                                       GDK_KEY_RELEASE_MASK);
  gtk_widget_set_can_focus(_graph_window, TRUE);
  g_signal_connect(G_OBJECT(_graph_window), "scroll_event",
                   G_CALLBACK(scroll_callback), this);
  g_signal_connect(G_OBJECT(_graph_window), "key_press_event",
                   G_CALLBACK(key_press_callback), this);
  g_signal_connect(G_OBJECT(_graph_window), "key_release_event",
                   G_CALLBACK(key_release_callback), this);

  // Set up trackpad pinch and swipe gestures.
  _zoom_gesture = gtk_gesture_zoom_new(_graph_window);
  g_signal_connect(_zoom_gesture, "begin",
    G_CALLBACK(+[](GtkGestureZoom *gesture, GdkEventSequence *sequence, gpointer data) {
      GtkStatsTimeline *self = (GtkStatsTimeline *)data;
      self->_zoom_scale = self->get_horizontal_scale();
    }), this);

  g_signal_connect(_zoom_gesture, "scale-changed",
    G_CALLBACK((+[](GtkGestureZoom *gesture, gdouble scale, gpointer data) {
      GtkStatsTimeline *self = (GtkStatsTimeline *)data;
      gdouble x, y;
      if (gtk_gesture_get_point(GTK_GESTURE(gesture), NULL, &x, &y)) {
        int graph_x = (int)(x * self->_cr_scale);
        self->zoom_by(log(scale) * 0.8, self->pixel_to_timestamp(graph_x));
        self->start_animation();
      }
    })), this);

  int min_height = 0;
  if (!_threads.empty()) {
    double height = row_to_pixel(get_num_rows()) + _pixel_scale * 2.5;
    min_height = height / _cr_scale;

    // Never make the window taller than the screen.
    GdkScreen *screen = gtk_window_get_screen(GTK_WINDOW(_window));
    min_height = std::min(min_height, gdk_screen_get_height(screen));
  }

  gtk_widget_set_size_request(_graph_window,
    default_timeline_width * monitor->get_resolution() / 96,
    std::max(min_height, (int)(default_timeline_height * monitor->get_resolution() / 96)));

  gtk_window_set_title(GTK_WINDOW(_window), "Timeline");

  _grid_pattern = cairo_pattern_create_rgb(0xdd / 255.0, 0xdd / 255.0, 0xdd / 255.0);

  gtk_widget_show_all(_window);
  gtk_widget_show(_window);

  // Allow the window to be resized as small as the user likes.  We have to do
  // this after the window has been shown; otherwise, it will affect the
  // window's initial size.
  gtk_widget_set_size_request(_graph_window, 0, 0);

  clear_region();
}

/**
 *
 */
GtkStatsTimeline::
~GtkStatsTimeline() {
  cairo_pattern_destroy(_grid_pattern);
  g_object_unref(_zoom_gesture);
}

/**
 * Called as each frame's data is made available.  There is no guarantee the
 * frames will arrive in order, or that all of them will arrive at all.  The
 * monitor should be prepared to accept frames received out-of-order or
 * missing.
 */
void GtkStatsTimeline::
new_data(int thread_index, int frame_number) {
  PStatTimeline::new_data(thread_index, frame_number);
}

/**
 * Called when it is necessary to redraw the entire graph.
 */
void GtkStatsTimeline::
force_redraw() {
  assert(_cr);
  if (_cr) {
    PStatTimeline::force_redraw();
  }
}

/**
 * Called when the user has resized the window, forcing a resize of the graph.
 */
void GtkStatsTimeline::
changed_graph_size(int graph_xsize, int graph_ysize) {
  PStatTimeline::changed_size(graph_xsize, graph_ysize);
}

/**
 * Erases the chart area.
 */
void GtkStatsTimeline::
clear_region() {
  cairo_set_source_rgb(_cr, 1.0, 1.0, 1.0);
  cairo_paint(_cr);
}

/**
 * Erases the chart area in preparation for drawing a bunch of bars.
 */
void GtkStatsTimeline::
begin_draw() {
}

/**
 * Draws a horizontal separator.
 */
void GtkStatsTimeline::
draw_separator(int row) {
  cairo_set_source(_cr, _grid_pattern);
  cairo_rectangle(_cr, 0, (row_to_pixel(row) + row_to_pixel(row + 1)) / 2.0,
                  get_xsize(), _pixel_scale * 1 / 3);
  cairo_fill(_cr);
}

/**
 * Draws a vertical guide bar.  If the row is -1, draws it in all rows.
 */
void GtkStatsTimeline::
draw_guide_bar(int x, GuideBarStyle style) {
  double width = _pixel_scale / 3.0;
  if (style == GBS_frame) {
    width *= 2;
  }

  cairo_set_source(_cr, _grid_pattern);
  cairo_rectangle(_cr, x - width / 2.0, 0, width, get_ysize());
  cairo_fill(_cr);
}

/**
 * Draws a single bar in the chart for the indicated row, in the color for the
 * given collector, for the indicated horizontal pixel range.
 */
void GtkStatsTimeline::
draw_bar(int row, int from_x, int to_x, int collector_index,
         const std::string &collector_name) {
  int top = row_to_pixel(row);
  int bottom = row_to_pixel(row + 1);
  int scale = _pixel_scale;

  bool is_highlighted = row == _highlighted_row && _highlighted_x >= from_x && _highlighted_x < to_x;
  cairo_set_source(_cr, get_collector_pattern(collector_index, is_highlighted));

  if (to_x < from_x + 1) {
    // Too tiny to draw.
  }
  else if (to_x < from_x + scale) {
    // It's just a tiny sliver.  This is a more reliable way to draw it.
    cairo_rectangle(_cr, from_x, top, to_x - from_x, bottom - top);
    cairo_fill(_cr);
  }
  else {
    int left = std::max(from_x, -scale - 1);
    int right = std::min(std::max(to_x, from_x + 1), get_xsize() + scale);

    double radius = std::min((double)scale, (right - left) / 2.0);
    cairo_new_sub_path(_cr);
    cairo_arc(_cr, right - radius, top + radius, radius, -0.5 * M_PI, 0.0);
    cairo_arc(_cr, right - radius, bottom - radius, radius, 0.0, 0.5 * M_PI);
    cairo_arc(_cr, left + radius, bottom - radius, radius, 0.5 * M_PI, M_PI);
    cairo_arc(_cr, left + radius, top + radius, radius, M_PI, 1.5 * M_PI);
    cairo_close_path(_cr);
    cairo_fill(_cr);

    if ((to_x - from_x) >= scale * 4) {
      // Only bother drawing the text if we've got some space to draw on.
      // Choose a suitable foreground color.
      LRGBColor fg = get_collector_text_color(collector_index, is_highlighted);
      cairo_set_source_rgb(_cr, fg[0], fg[1], fg[2]);

      // Make sure that the text doesn't run off the chart.
      int text_width, text_height;
      PangoLayout *layout = gtk_widget_create_pango_layout(_graph_window, collector_name.c_str());
      pango_layout_set_attributes(layout, _pango_attrs);
      pango_layout_set_height(layout, -1);
      pango_layout_get_pixel_size(layout, &text_width, &text_height);

      double center = (from_x + to_x) / 2.0;
      double text_left = std::max(from_x, 0) + scale / 2.0;
      double text_right = std::min(to_x, get_xsize()) - scale / 2.0;
      double text_top = top + (bottom - top - text_height) / 2.0;

      if (text_width >= text_right - text_left) {
        size_t c = collector_name.rfind(':');
        if (text_right - text_left < scale * 6) {
          // It's a really tiny space.  Draw a single letter.
          const char *ch = collector_name.data() + (c != std::string::npos ? c + 1 : 0);
          pango_layout_set_alignment(layout, PANGO_ALIGN_CENTER);
          pango_layout_set_text(layout, ch, 1);
        } else {
          // Maybe just use everything after the last colon.
          if (c != std::string::npos) {
            pango_layout_set_text(layout, collector_name.data() + c + 1,
                                          collector_name.size() - c - 1);
            pango_layout_get_pixel_size(layout, &text_width, &text_height);
          }
        }
      }

      if (text_width >= text_right - text_left) {
        // It's going to be tricky to fit it, let pango figure it out.
        pango_layout_set_ellipsize(layout, PANGO_ELLIPSIZE_END);
        pango_layout_set_width(layout, (text_right - text_left) * PANGO_SCALE);
        cairo_move_to(_cr, text_left, text_top);
      }
      else if (center - text_width / 2.0 < 0.0) {
        // Put it against the left-most edge.
        cairo_move_to(_cr, scale, text_top);
      }
      else if (center + text_width / 2.0 >= get_xsize()) {
        // Put it against the right-most edge.
        cairo_move_to(_cr, get_xsize() - scale - text_width, text_top);
      }
      else {
        // It fits just fine, center it.
        cairo_move_to(_cr, center - text_width / 2.0, text_top);
      }

      pango_cairo_show_layout(_cr, layout);
      g_object_unref(layout);
    }
  }
}

/**
 * Called after all the bars have been drawn, this triggers a refresh event to
 * draw it to the window.
 */
void GtkStatsTimeline::
end_draw() {
  gtk_widget_queue_draw(_graph_window);

  if (_threads_changed) {
    // Calculate the size of the thread area.
    PangoLayout *layout = gtk_widget_create_pango_layout(_thread_area, "");

    int max_width = 0;
    for (const ThreadRow &thread_row : _threads) {
      if (!thread_row._visible) {
        continue;
      }
      pango_layout_set_text(layout, thread_row._label.c_str(), thread_row._label.size());

      int width, height;
      pango_layout_get_pixel_size(layout, &width, &height);

      if (width > max_width) {
        max_width = width;
      }
    }

    gtk_widget_set_size_request(_thread_area, max_width + _pixel_scale * 2, 0);
    g_object_unref(layout);
    gtk_widget_queue_draw(_thread_area);
    _threads_changed = false;
  }

  if (_guide_bars_changed) {
    gtk_widget_queue_draw(_scale_area);
    _guide_bars_changed = false;
  }
}

/**
 * Called at the end of the draw cycle.
 */
void GtkStatsTimeline::
idle() {
}

/**
 * Overridden by a derived class to implement an animation.  If it returns
 * false, the animation timer is stopped.
 */
bool GtkStatsTimeline::
animate(double time, double dt) {
  return PStatTimeline::animate(time, dt);
}

/**
 * Returns the current window dimensions.
 */
bool GtkStatsTimeline::
get_window_state(int &x, int &y, int &width, int &height,
                 bool &maximized, bool &minimized) const {
  GtkStatsGraph::get_window_state(x, y, width, height, maximized, minimized);
  return true;
}

/**
 * Called to restore the graph window to its previous dimensions.
 */
void GtkStatsTimeline::
set_window_state(int x, int y, int width, int height,
                 bool maximized, bool minimized) {
  GtkStatsGraph::set_window_state(x, y, width, height, maximized, minimized);
}

/**
 * This is called during the servicing of the draw event; it gives a derived
 * class opportunity to do some further painting into the graph window.
 */
void GtkStatsTimeline::
additional_graph_window_paint(cairo_t *cr) {
}

/**
 * Called when the mouse hovers over the graph, and should return the text that
 * should appear on the tooltip.
 */
std::string GtkStatsTimeline::
get_graph_tooltip(int mouse_x, int mouse_y) const {
  return PStatTimeline::get_bar_tooltip(pixel_to_row(mouse_y), mouse_x);
}

/**
 * Based on the mouse position within the graph window, look for draggable
 * things the mouse might be hovering over and return the appropriate DragMode
 * enum or DM_none if nothing is indicated.
 */
GtkStatsGraph::DragMode GtkStatsTimeline::
consider_drag_start(int graph_x, int graph_y) {
  return GtkStatsGraph::consider_drag_start(graph_x, graph_y);
}

/**
 * Called when the mouse button is depressed within the graph window.
 */
gboolean GtkStatsTimeline::
handle_button_press(int graph_x, int graph_y, bool double_click, int button) {
  if (graph_x >= 0 && graph_y >= 0 && graph_x < get_xsize() && graph_y < get_ysize()) {
    if (button == 3) {
      // Right-clicking a color bar brings up a context menu.
      int row = pixel_to_row(graph_y);

      ColorBar bar;
      if (find_bar(row, graph_x, bar)) {
        GtkWidget *menu = gtk_menu_new();
        _popup_bar = bar;

        std::string label = get_bar_tooltip(row, graph_x);
        if (!label.empty()) {
          GtkWidget *menu_item = gtk_menu_item_new_with_label(label.c_str());
          gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
          gtk_widget_set_sensitive(menu_item, FALSE);
        }

        {
          GtkWidget *menu_item = gtk_menu_item_new_with_label("Zoom To");
          gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
          g_signal_connect(G_OBJECT(menu_item), "activate",
            G_CALLBACK(+[] (GtkWidget *widget, gpointer data) {
              GtkStatsTimeline *self = (GtkStatsTimeline *)data;
              const ColorBar &bar = self->_popup_bar;
              double width = bar._end - bar._start;
              self->zoom_to(width * 1.5, (bar._end + bar._start) / 2.0);
              self->scroll_to(bar._start - width / 4.0);
              self->start_animation();
            }),
            this);
        }

        {
          const GtkStatsMonitor::MenuDef *menu_def = GtkStatsGraph::_monitor->add_menu({
            GtkStatsMonitor::CT_strip_chart,
            bar._thread_index, bar._collector_index,
          });

          GtkWidget *menu_item = gtk_menu_item_new_with_label("Open Strip Chart");
          gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
          g_signal_connect(G_OBJECT(menu_item), "activate",
                           G_CALLBACK(GtkStatsMonitor::menu_activate),
                           (void *)menu_def);
        }

        {
          const GtkStatsMonitor::MenuDef *menu_def = GtkStatsGraph::_monitor->add_menu({
            GtkStatsMonitor::CT_flame_graph,
            bar._thread_index, bar._collector_index, bar._frame_number,
          });

          GtkWidget *menu_item = gtk_menu_item_new_with_label("Open Flame Graph");
          gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
          g_signal_connect(G_OBJECT(menu_item), "activate",
                           G_CALLBACK(GtkStatsMonitor::menu_activate),
                           (void *)menu_def);
        }

        {
          const GtkStatsMonitor::MenuDef *menu_def = GtkStatsGraph::_monitor->add_menu({
            GtkStatsMonitor::CT_piano_roll, bar._thread_index, -1,
          });

          GtkWidget *menu_item = gtk_menu_item_new_with_label("Open Piano Roll");
          gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
          g_signal_connect(G_OBJECT(menu_item), "activate",
                           G_CALLBACK(GtkStatsMonitor::menu_activate),
                           (void *)menu_def);
        }

        {
          GtkWidget *menu_item = gtk_separator_menu_item_new();
          gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
        }

        {
          const GtkStatsMonitor::MenuDef *menu_def = GtkStatsGraph::_monitor->add_menu({
            GtkStatsMonitor::CT_choose_color, -1, bar._collector_index,
          });

          GtkWidget *menu_item = gtk_menu_item_new_with_label("Change Color...");
          gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
          g_signal_connect(G_OBJECT(menu_item), "activate",
                           G_CALLBACK(GtkStatsMonitor::menu_activate),
                           (void *)menu_def);
        }

        {
          const GtkStatsMonitor::MenuDef *menu_def = GtkStatsGraph::_monitor->add_menu({
            GtkStatsMonitor::CT_reset_color, -1, bar._collector_index,
          });

          GtkWidget *menu_item = gtk_menu_item_new_with_label("Reset Color");
          gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
          g_signal_connect(G_OBJECT(menu_item), "activate",
                           G_CALLBACK(GtkStatsMonitor::menu_activate),
                           (void *)menu_def);
        }

        gtk_widget_show_all(menu);
        gtk_menu_popup_at_pointer(GTK_MENU(menu), nullptr);
        return TRUE;
      }
      return FALSE;
    }
    else if (double_click && button == 1) {
      // Double-clicking on a color bar in the graph will zoom the graph into
      // that collector.
      int row = pixel_to_row(graph_y);
      ColorBar bar;
      if (find_bar(row, graph_x, bar)) {
        double width = bar._end - bar._start;
        zoom_to(width * 1.5, pixel_to_timestamp(graph_x));
        scroll_to(bar._start - width / 4.0);
      } else {
        // Double-clicking the white area zooms out.
        _zoom_speed -= 100.0;
      }
      start_animation();
    }

    if (_potential_drag_mode == DM_none) {
      set_drag_mode(DM_pan);
      _drag_start_x = graph_x;
      _scroll_speed = 0.0;
      _zoom_center = pixel_to_timestamp(graph_x);
      return TRUE;
    }
  }

  return GtkStatsGraph::handle_button_press(graph_x, graph_y,
                                            double_click, button);
}

/**
 * Called when the mouse button is released within the graph window.
 */
gboolean GtkStatsTimeline::
handle_button_release(int graph_x, int graph_y) {
  if (_drag_mode == DM_scale) {
    set_drag_mode(DM_none);
    // ReleaseCapture();
    return handle_motion(graph_x, graph_y);
  }
  else if (_drag_mode == DM_guide_bar) {
    if (graph_x < 0 || graph_x >= get_xsize()) {
      remove_user_guide_bar(_drag_guide_bar);
    } else {
      move_user_guide_bar(_drag_guide_bar, pixel_to_height(graph_x));
    }
    set_drag_mode(DM_none);
    // ReleaseCapture();
    return handle_motion(graph_x, graph_y);
  }

  return GtkStatsGraph::handle_button_release(graph_x, graph_y);
}

/**
 * Called when the mouse is moved within the graph window.
 */
gboolean GtkStatsTimeline::
handle_motion(int graph_x, int graph_y) {
  if (_drag_mode == DM_none && _potential_drag_mode == DM_none &&
      graph_x >= 0 && graph_y >= 0 && graph_x < get_xsize() && graph_y < get_ysize()) {
    // When the mouse is over a color bar, highlight it.
    int row = pixel_to_row(graph_y);
    std::swap(_highlighted_x, graph_x);
    std::swap(_highlighted_row, row);

    if (row >= 0) {
      PStatTimeline::force_redraw(row, graph_x, graph_x);
    }
    PStatTimeline::force_redraw(_highlighted_row, _highlighted_x, _highlighted_x);

    if ((_keys_held & (F_w | F_s)) != 0) {
      // Update the zoom center if we move the mouse while zooming with the
      // keyboard.
      _zoom_center = pixel_to_timestamp(graph_x);
    }
  }
  else {
    // If the mouse is in some drag mode, stop highlighting.
    if (_highlighted_row != -1) {
      int row = _highlighted_row;
      _highlighted_row = -1;
      PStatTimeline::force_redraw(row, _highlighted_x, _highlighted_x);
    }
  }

  if (_drag_mode == DM_pan) {
    int delta = _drag_start_x - graph_x;
    _drag_start_x = graph_x;
    set_horizontal_scroll(get_horizontal_scroll() + pixel_to_height(delta));
    return 0;
  }

  return GtkStatsGraph::handle_motion(graph_x, graph_y);
}

/**
 * Called when the mouse has left the graph window.
 */
gboolean GtkStatsTimeline::
handle_leave() {
  if (_highlighted_row != -1) {
    int row = _highlighted_row;
    _highlighted_row = -1;
    PStatTimeline::force_redraw(row, _highlighted_x, _highlighted_x);
  }
  return TRUE;
}

/**
 *
 */
gboolean GtkStatsTimeline::
handle_scroll(int graph_x, int graph_y, double dx, double dy, bool ctrl_held) {
  gboolean handled = FALSE;

  if (dy != 0.0) {
    handled = TRUE;
    if (ctrl_held) {
      zoom_by(dy, pixel_to_timestamp(graph_x));
      start_animation();
    } else {
      double delta = (int)(dy * _pixel_scale * 5 + 0.5);
      int new_scroll = _scroll - delta;
      if (_threads.empty()) {
        new_scroll = 0;
      } else {
        new_scroll = (std::min)(new_scroll, get_num_rows() * _pixel_scale * 5 + _pixel_scale * 2 - get_ysize());
        new_scroll = (std::max)(new_scroll, 0);
      }
      delta = new_scroll - _scroll;
      if (delta != 0) {
        _scroll = new_scroll;
        gtk_widget_queue_draw(_thread_area);
        force_redraw();
      }
    }
  }

  if (dx != 0.0) {
    _scroll_speed += dx * 10.0;
    handled = TRUE;
    start_animation();
  }

  return handled;
}

/**
 *
 */
gboolean GtkStatsTimeline::
handle_zoom(int graph_x, int graph_y, double scale) {
  zoom_to(get_horizontal_scale() / scale, pixel_to_timestamp(graph_x));
  start_animation();
  return TRUE;
}

/**
 *
 */
gboolean GtkStatsTimeline::
handle_key(bool pressed, guint val, guint16 hw_code) {
  // Accept WASD based on their position rather than their mapping
  int flag = 0;
  switch (hw_code) {
  case 25:
    flag = F_w;
    break;
  case 38:
    flag = F_a;
    break;
  case 39:
    flag = F_s;
    break;
  case 40:
    flag = F_d;
    break;
  }
  if (flag == 0) {
    switch (val) {
    case GDK_KEY_Left:
      flag = F_left;
      break;
    case GDK_KEY_Right:
      flag = F_right;
      break;
    case GDK_KEY_w:
      flag = F_w;
      break;
    case GDK_KEY_a:
      flag = F_a;
      break;
    case GDK_KEY_s:
      flag = F_s;
      break;
    case GDK_KEY_d:
      flag = F_d;
      break;
    }
  }
  if (flag != 0) {
    if (pressed) {
      if (flag & (F_w | F_s)) {
        // Pfoo, GTK sure does make it hard to just get the cursor position.
        GdkWindow *window = gtk_widget_get_window(_graph_window);
        GdkDisplay *display = gdk_window_get_display(window);
        GdkDeviceManager *device_manager = gdk_display_get_device_manager(display);
        GdkDevice *device = gdk_device_manager_get_client_pointer(device_manager);
        gint x, y;
        gdk_window_get_device_position(window, device, &x, &y, nullptr);
        _zoom_center = pixel_to_timestamp(x * _cr_scale);
      }
      if (_keys_held == 0) {
        start_animation();
      }
      _keys_held |= flag;
    }
    else if (_keys_held != 0) {
      _keys_held &= ~flag;
    }
    return TRUE;
  }
  return FALSE;
}

/**
 * This is called during the servicing of the draw event.
 */
void GtkStatsTimeline::
draw_guide_labels(cairo_t *cr) {
  int num_guide_bars = get_num_guide_bars();
  for (int i = 0; i < num_guide_bars; i++) {
    draw_guide_label(cr, get_guide_bar(i));
  }
}

/**
 * Draws the text for the indicated guide bar label at the top of the graph.
 */
void GtkStatsTimeline::
draw_guide_label(cairo_t *cr, const PStatGraph::GuideBar &bar) {
  const std::string &label = bar._label;
  if (label.empty()) {
    return;
  }

  bool center = true;
  switch (bar._style) {
  case GBS_target:
    cairo_set_source_rgb(cr, rgb_light_gray[0], rgb_light_gray[1], rgb_light_gray[2]);
    break;

  case GBS_user:
    cairo_set_source_rgb(cr, rgb_user_guide_bar[0], rgb_user_guide_bar[1], rgb_user_guide_bar[2]);
    break;

  case GBS_normal:
    cairo_set_source_rgb(cr, rgb_light_gray[0], rgb_light_gray[1], rgb_light_gray[2]);
    break;

  case GBS_frame:
    cairo_set_source_rgb(cr, rgb_dark_gray[0], rgb_dark_gray[1], rgb_dark_gray[2]);
    center = false;
    break;
  }

  PangoLayout *layout = gtk_widget_create_pango_layout(_scale_area, label.c_str());

  if (bar._style != GBS_frame) {
    // Make the offsets slightly smaller.
    PangoAttrList *attrs = pango_attr_list_new();
    PangoAttribute *attr = pango_attr_scale_new(0.9);
    attr->start_index = 0;
    attr->end_index = -1;
    pango_attr_list_insert(attrs, attr);
    pango_layout_set_attributes(layout, attrs);
    pango_attr_list_unref(attrs);
  }

  int width, height;
  pango_layout_get_pixel_size(layout, &width, &height);

  int x = timestamp_to_pixel(bar._height);
  if (x >= 0 && x + width * _cr_scale < get_xsize()) {
    // Now convert our x to a coordinate within our drawing area.
    int junk_y;

    x /= _cr_scale;

    // The x coordinate comes from the graph_window.
    gtk_widget_translate_coordinates(_graph_window, _scale_area,
                                     x, 0, &x, &junk_y);

    GtkAllocation allocation;
    gtk_widget_get_allocation(_scale_area, &allocation);

    if (x >= 0) {
      int this_x = x;
      if (center) {
        this_x -= width / 2;
      }
      if (this_x + width < allocation.width) {
        cairo_move_to(cr, this_x, allocation.height - height);
        pango_cairo_show_layout(cr, layout);
      }
    }
  }

  g_object_unref(layout);
}

/**
 * This is called during the servicing of the draw event.
 */
void GtkStatsTimeline::
draw_thread_labels(cairo_t *cr) {
  for (const ThreadRow &thread_row : _threads) {
    if (thread_row._visible) {
      draw_thread_label(cr, thread_row);
    }
  }
}

/**
 * Draws the text for the indicated thread on the side of the graph.
 */
void GtkStatsTimeline::
draw_thread_label(cairo_t *cr, const ThreadRow &thread_row) {
  int top = row_to_pixel(thread_row._row_offset);
  if (top <= get_ysize()) {
    // Now convert our y to a coordinate within our drawing area.
    top /= _cr_scale;

    // The y coordinate comes from the graph_window.
    int junk_x;
    gtk_widget_translate_coordinates(_graph_window, _thread_area,
                                     0, top, &junk_x, &top);

    GtkAllocation allocation;
    gtk_widget_get_allocation(_thread_area, &allocation);

    PangoLayout *layout = gtk_widget_create_pango_layout(_thread_area, thread_row._label.c_str());
    pango_layout_set_alignment(layout, PANGO_ALIGN_RIGHT);
    pango_layout_set_ellipsize(layout, PANGO_ELLIPSIZE_END);
    pango_layout_set_width(layout, (allocation.width - _pixel_scale * 2) * PANGO_SCALE);

    cairo_move_to(cr, _pixel_scale, top);
    pango_cairo_show_layout(cr, layout);
    g_object_unref(layout);
  }
}

/**
 * Draws in the scale labels.
 */
gboolean GtkStatsTimeline::
scale_area_draw_callback(GtkWidget *widget, cairo_t *cr, gpointer data) {
  GtkStatsTimeline *self = (GtkStatsTimeline *)data;
  self->draw_guide_labels(cr);

  return TRUE;
}

/**
 * Draws in the thread labels.
 */
gboolean GtkStatsTimeline::
thread_area_draw_callback(GtkWidget *widget, cairo_t *cr, gpointer data) {
  GtkStatsTimeline *self = (GtkStatsTimeline *)data;
  self->draw_thread_labels(cr);

  return TRUE;
}

/**
 *
 */
gboolean GtkStatsTimeline::
scroll_callback(GtkWidget *widget, GdkEventScroll *event, gpointer data) {
  GtkStatsTimeline *self = (GtkStatsTimeline *)data;

  bool ctrl_held = (event->state & GDK_CONTROL_MASK) != 0;

  double dx, dy;
  if (event->direction == GDK_SCROLL_UP) {
    dx = 0;
    dy = 1;
  }
  else if (event->direction == GDK_SCROLL_DOWN) {
    dx = 0;
    dy = -1;
  }
  else if (event->direction == GDK_SCROLL_LEFT) {
    dx = 1;
    dy = 0;
  }
  else if (event->direction == GDK_SCROLL_RIGHT) {
    dx = -1;
    dy = 0;
  }
  else if (!gdk_event_get_scroll_deltas((GdkEvent *)event, &dx, &dy)) {
    return FALSE;
  }

  int graph_x = (int)(event->x * self->_cr_scale);
  int graph_y = (int)(event->y * self->_cr_scale);
  return self->handle_scroll(graph_x, graph_y, dx, dy, ctrl_held);
}

/**
 *
 */
gboolean GtkStatsTimeline::
key_press_callback(GtkWidget *widget, GdkEventKey *event, gpointer data) {
  GtkStatsTimeline *self = (GtkStatsTimeline *)data;
  return self->handle_key(true, event->keyval, event->hardware_keycode);
}

/**
 *
 */
gboolean GtkStatsTimeline::
key_release_callback(GtkWidget *widget, GdkEventKey *event, gpointer data) {
  GtkStatsTimeline *self = (GtkStatsTimeline *)data;
  return self->handle_key(false, event->keyval, event->hardware_keycode);
}
