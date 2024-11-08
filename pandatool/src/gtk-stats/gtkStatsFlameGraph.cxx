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
                   int collector_index, int frame_number) :
  PStatFlameGraph(monitor, thread_index, collector_index, frame_number, 0, 0),
  GtkStatsGraph(monitor, false)
{
  // Let's show the units on the guide bar labels.  There's room.
  set_guide_bar_units(get_guide_bar_units() | GBU_show_units);

  std::string window_title = get_title_text();
  gtk_window_set_title(GTK_WINDOW(_window), window_title.c_str());

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

  // Listen for mouse scroll and keyboard events.
  gtk_widget_add_events(_window, GDK_SCROLL_MASK | GDK_KEY_PRESS_MASK);
  g_signal_connect(G_OBJECT(_window), "scroll_event",
                   G_CALLBACK(scroll_callback), this);
  g_signal_connect(G_OBJECT(_window), "key_press_event",
                   G_CALLBACK(key_press_callback), this);

  gtk_widget_set_size_request(_graph_window,
    default_flame_graph_width * monitor->get_resolution() / 96,
    default_flame_graph_height * monitor->get_resolution() / 96);

  gtk_widget_show_all(_window);
  gtk_widget_show(_window);

  // Allow the window to be resized as small as the user likes.  We have to do
  // this after the window has been shown; otherwise, it will affect the
  // window's initial size.
  gtk_widget_set_size_request(_window, 0, 0);

  clear_region();

  if (get_average_mode()) {
    start_animation();
  }
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
  if (_cr) {
    PStatFlameGraph::force_redraw();
  }
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
  push_collector_index(collector_index);
}

/**
 * Called when the user hovers the mouse over a label.
 */
void GtkStatsFlameGraph::
on_enter_label(int collector_index) {
  if (collector_index != _highlighted_index) {
    _highlighted_index = collector_index;

    if (!get_average_mode()) {
      PStatFlameGraph::force_redraw();
    }
  }
}

/**
 * Called when the user's mouse cursor leaves a label.
 */
void GtkStatsFlameGraph::
on_leave_label(int collector_index) {
  if (collector_index == _highlighted_index && collector_index != -1) {
    _highlighted_index = -1;

    if (!get_average_mode()) {
      PStatFlameGraph::force_redraw();
    }
  }
}

/**
 * Calls update_guide_bars with parameters suitable to this kind of graph.
 */
void GtkStatsFlameGraph::
normal_guide_bars() {
  // We want vaguely 100 pixels between guide bars.
  int num_bars = get_xsize() / (_pixel_scale * 25);

  _guide_bars.clear();

  double dist = get_horizontal_scale() / num_bars;

  for (int i = 1; i < num_bars; ++i) {
    _guide_bars.push_back(make_guide_bar(i * dist));
  }

  _guide_bars_changed = true;

  nassertv_always(_scale_area != nullptr);
  gtk_widget_queue_draw(_scale_area);
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
 * Should be overridden by the user class.  Should draw a single bar at the
 * indicated location.
 */
void GtkStatsFlameGraph::
draw_bar(int depth, int from_x, int to_x, int collector_index, int parent_index) {
  double bottom = get_ysize() - depth * _pixel_scale * 5;
  double top = bottom - _pixel_scale * 5;

  bool is_highlighted = collector_index == _highlighted_index;
  cairo_set_source(_cr, get_collector_pattern(collector_index, is_highlighted));

  if (to_x < from_x + 3) {
    // It's just a tiny sliver.  This is a more reliable way to draw it.
    cairo_rectangle(_cr, from_x, top, to_x - from_x, bottom - top);
    cairo_fill(_cr);
  }
  else {
    double radius = std::min((double)_pixel_scale, (to_x - from_x) / 2.0);
    cairo_new_sub_path(_cr);
    cairo_arc(_cr, to_x - radius, top + radius, radius, -0.5 * M_PI, 0.0);
    cairo_arc(_cr, to_x - radius, bottom - radius, radius, 0.0, 0.5 * M_PI);
    cairo_arc(_cr, from_x + radius, bottom - radius, radius, 0.5 * M_PI, M_PI);
    cairo_arc(_cr, from_x + radius, top + radius, radius, M_PI, 1.5 * M_PI);
    cairo_close_path(_cr);
    cairo_fill(_cr);

    if ((to_x - from_x) >= _pixel_scale * 4) {
      // Only bother drawing the text if we've got some space to draw on.
      int left = std::max(from_x, 0) + _pixel_scale / 2;
      int right = std::min(to_x, get_xsize()) - _pixel_scale / 2;

      const PStatClientData *client_data = GtkStatsGraph::_monitor->get_client_data();
      const PStatCollectorDef &def = client_data->get_collector_def(collector_index);

      // Choose a suitable foreground color.
      LRGBColor fg = get_collector_text_color(collector_index, is_highlighted);
      cairo_set_source_rgb(_cr, fg[0], fg[1], fg[2]);

      PangoLayout *layout = gtk_widget_create_pango_layout(_graph_window, def._name.c_str());
      pango_layout_set_attributes(layout, _pango_attrs);
      pango_layout_set_ellipsize(layout, PANGO_ELLIPSIZE_END);
      pango_layout_set_width(layout, (right - left) * PANGO_SCALE);
      pango_layout_set_height(layout, -1);

      if (!pango_layout_is_ellipsized(layout)) {
        // We have room for more.  Show the collector's actual parent, if it's
        // different than the block it's shown above.
        if (def._parent_index > 0 && def._parent_index != parent_index) {
          const PStatCollectorDef &parent_def = client_data->get_collector_def(def._parent_index);
          std::string long_name = parent_def._name + ":" + def._name;
          pango_layout_set_text(layout, long_name.c_str(), long_name.size());
          if (pango_layout_is_ellipsized(layout)) {
            // Nope, it's too long, go back.
            pango_layout_set_text(layout, def._name.c_str(), def._name.size());
          }
        }
        else if (collector_index == 0 && get_frame_number() >= 0) {
          char text[32];
          sprintf(text, "Frame %d", get_frame_number());
          pango_layout_set_text(layout, text, -1);
          if (pango_layout_is_ellipsized(layout)) {
            // Nope, it's too long, go back.
            pango_layout_set_text(layout, def._name.c_str(), def._name.size());
          }
        }
      }

      int width, height;
      pango_layout_get_pixel_size(layout, &width, &height);

      // Center the text vertically in the bar.
      cairo_move_to(_cr, left, top + (bottom - top - height) / 2);
      pango_cairo_show_layout(_cr, layout);

      g_object_unref(layout);
    }
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
 * Overridden by a derived class to implement an animation.  If it returns
 * false, the animation timer is stopped.
 */
bool GtkStatsFlameGraph::
animate(double time, double dt) {
  return PStatFlameGraph::animate(time, dt);
}

/**
 * Returns the current window dimensions.
 */
bool GtkStatsFlameGraph::
get_window_state(int &x, int &y, int &width, int &height,
                 bool &maximized, bool &minimized) const {
  GtkStatsGraph::get_window_state(x, y, width, height, maximized, minimized);
  return true;
}

/**
 * Called to restore the graph window to its previous dimensions.
 */
void GtkStatsFlameGraph::
set_window_state(int x, int y, int width, int height,
                 bool maximized, bool minimized) {
  GtkStatsGraph::set_window_state(x, y, width, height, maximized, minimized);
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
 * Called when the mouse hovers over the graph, and should return the text that
 * should appear on the tooltip.
 */
std::string GtkStatsFlameGraph::
get_graph_tooltip(int mouse_x, int mouse_y) const {
  return get_bar_tooltip(pixel_to_depth(mouse_y), mouse_x);
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
handle_button_press(int graph_x, int graph_y, bool double_click, int button) {
  if (graph_x >= 0 && graph_y >= 0 && graph_x < get_xsize() && graph_y < get_ysize()) {
    int depth = pixel_to_depth(graph_y);
    int collector_index = get_bar_collector(depth, graph_x);
    if (button == 3) {
      if (collector_index >= 0) {
        GtkWidget *menu = gtk_menu_new();
        _popup_index = collector_index;

        std::string label = get_bar_tooltip(depth, graph_x);
        if (!label.empty()) {
          GtkWidget *menu_item = gtk_menu_item_new_with_label(label.c_str());
          gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
          gtk_widget_set_sensitive(menu_item, FALSE);
        }

        {
          GtkWidget *menu_item = gtk_menu_item_new_with_label("Set as Focus");
          gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

          if (collector_index == 0 && get_collector_index() == 0) {
            gtk_widget_set_sensitive(menu_item, FALSE);
          } else {
            g_signal_connect(G_OBJECT(menu_item), "activate",
              G_CALLBACK(+[] (GtkWidget *widget, gpointer data) {
                GtkStatsFlameGraph *self = (GtkStatsFlameGraph *)data;
                self->push_collector_index(self->_popup_index);
              }),
              this);
          }
        }

        {
          const GtkStatsMonitor::MenuDef *menu_def = GtkStatsGraph::_monitor->add_menu({
            GtkStatsMonitor::CT_strip_chart, get_thread_index(), collector_index,
          });

          GtkWidget *menu_item = gtk_menu_item_new_with_label("Open Strip Chart");
          gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
          g_signal_connect(G_OBJECT(menu_item), "activate",
                           G_CALLBACK(GtkStatsMonitor::menu_activate),
                           (void *)menu_def);
        }

        {
          const GtkStatsMonitor::MenuDef *menu_def = GtkStatsGraph::_monitor->add_menu({
            GtkStatsMonitor::CT_flame_graph, get_thread_index(), collector_index,
          });

          GtkWidget *menu_item = gtk_menu_item_new_with_label("Open Flame Graph");
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
            GtkStatsMonitor::CT_choose_color, -1, collector_index,
          });

          GtkWidget *menu_item = gtk_menu_item_new_with_label("Change Color...");
          gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
          g_signal_connect(G_OBJECT(menu_item), "activate",
                           G_CALLBACK(GtkStatsMonitor::menu_activate),
                           (void *)menu_def);
        }

        {
          const GtkStatsMonitor::MenuDef *menu_def = GtkStatsGraph::_monitor->add_menu({
            GtkStatsMonitor::CT_reset_color, -1, collector_index,
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
      if (collector_index >= 0) {
        push_collector_index(collector_index);
      } else {
        // Double-clicking the background goes to the top.
        clear_history();
        set_collector_index(-1);
      }
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

  return GtkStatsGraph::handle_button_press(graph_x, graph_y,
                                            double_click, button);
}

/**
 * Called when the mouse button is released within the graph window.
 */
gboolean GtkStatsFlameGraph::
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
gboolean GtkStatsFlameGraph::
handle_motion(int graph_x, int graph_y) {
  if (_drag_mode == DM_none && _potential_drag_mode == DM_none &&
      graph_x >= 0 && graph_y >= 0 && graph_x < get_xsize() && graph_y < get_ysize()) {
    // When the mouse is over a color bar, highlight it.
    int depth = pixel_to_depth(graph_y);
    int collector_index = get_bar_collector(depth, graph_x);
    on_enter_label(collector_index);
  }
  else {
    // If the mouse is in some drag mode, stop highlighting.
    _label_stack.highlight_label(-1);
    on_leave_label(_highlighted_index);
  }

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

  return GtkStatsGraph::handle_motion(graph_x, graph_y);
}

/**
 * Called when the mouse has left the graph window.
 */
gboolean GtkStatsFlameGraph::
handle_leave() {
  _label_stack.highlight_label(-1);
  on_leave_label(_highlighted_index);
  return TRUE;
}

/**
 * Converts a pixel to a depth index.
 */
int GtkStatsFlameGraph::
pixel_to_depth(int y) const {
  return (get_ysize() - 1 - y) / (_pixel_scale * 5);
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

    default:
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

  default:
    cairo_set_source_rgb(cr, rgb_dark_gray[0], rgb_dark_gray[1], rgb_dark_gray[2]);
    break;
  }

  int x = height_to_pixel(bar._height);
  const std::string &label = bar._label;

  PangoLayout *layout = gtk_widget_create_pango_layout(_scale_area, label.c_str());
  int width, height;
  pango_layout_get_pixel_size(layout, &width, &height);

  if (bar._style != GBS_user) {
    double from_height = pixel_to_height(x - width * _cr_scale);
    double to_height = pixel_to_height(x + width * _cr_scale);
    if (find_user_guide_bar(from_height, to_height) >= 0) {
      // Omit the label: there's a user-defined guide bar in the same space.
      g_object_unref(layout);
      return;
    }
  }

  if (x >= 0 && x < get_xsize()) {
    // Now convert our x to a coordinate within our drawing area.
    int junk_y;

    x /= _cr_scale;

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
  if (active) {
    self->start_animation();
  }
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

/**
 *
 */
gboolean GtkStatsFlameGraph::
scroll_callback(GtkWidget *widget, GdkEventScroll *event, gpointer data) {
  GtkStatsFlameGraph *self = (GtkStatsFlameGraph *)data;
  bool changed = false;
  switch (event->direction) {
  case GDK_SCROLL_LEFT:
    changed = self->prev_frame();
    break;

  case GDK_SCROLL_RIGHT:
    changed = self->next_frame();
    break;
  }

  if (changed) {
    std::string window_title = self->get_title_text();
    gtk_window_set_title(GTK_WINDOW(self->_window), window_title.c_str());
    return TRUE;
  } else {
    return FALSE;
  }
}

/**
 *
 */
gboolean GtkStatsFlameGraph::
key_press_callback(GtkWidget *widget, GdkEventKey *event, gpointer data) {
  GtkStatsFlameGraph *self = (GtkStatsFlameGraph *)data;
  bool changed = false;
  switch (event->keyval) {
  case GDK_KEY_Left:
    if (event->state & GDK_MOD1_MASK) {
      changed = self->pop_collector_index();
    } else {
      changed = self->prev_frame();
    }
    break;

  case GDK_KEY_Right:
    if ((event->state & GDK_MOD1_MASK) == 0) {
      changed = self->next_frame();
    }
    break;

  case GDK_KEY_Home:
    if ((event->state & GDK_MOD1_MASK) == 0) {
      changed = self->first_frame();
    }
    break;

  case GDK_KEY_End:
    if ((event->state & GDK_MOD1_MASK) == 0) {
      changed = self->last_frame();
    }
    break;
  }

  if (changed) {
    std::string window_title = self->get_title_text();
    gtk_window_set_title(GTK_WINDOW(self->_window), window_title.c_str());
    return TRUE;
  } else {
    return FALSE;
  }
}
