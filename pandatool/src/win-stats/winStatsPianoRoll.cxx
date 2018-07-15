/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file winStatsPianoRoll.cxx
 * @author drose
 * @date 2003-12-03
 */

#include "winStatsPianoRoll.h"
#include "winStatsMonitor.h"
#include "numeric_types.h"

static const int default_piano_roll_width = 400;
static const int default_piano_roll_height = 200;

bool WinStatsPianoRoll::_window_class_registered = false;
const char * const WinStatsPianoRoll::_window_class_name = "piano";

/**
 *
 */
WinStatsPianoRoll::
WinStatsPianoRoll(WinStatsMonitor *monitor, int thread_index) :
  PStatPianoRoll(monitor, thread_index,
                 default_piano_roll_width,
                 default_piano_roll_height),
  WinStatsGraph(monitor)
{
  _left_margin = 128;
  _right_margin = 8;
  _top_margin = 16;
  _bottom_margin = 8;

  // Let's show the units on the guide bar labels.  There's room.
  set_guide_bar_units(get_guide_bar_units() | GBU_show_units);

  create_window();
  clear_region();
}

/**
 *
 */
WinStatsPianoRoll::
~WinStatsPianoRoll() {
}

/**
 * Called as each frame's data is made available.  There is no gurantee the
 * frames will arrive in order, or that all of them will arrive at all.  The
 * monitor should be prepared to accept frames received out-of-order or
 * missing.
 */
void WinStatsPianoRoll::
new_data(int thread_index, int frame_number) {
  if (!_pause) {
    update();
  }
}

/**
 * Called when it is necessary to redraw the entire graph.
 */
void WinStatsPianoRoll::
force_redraw() {
  PStatPianoRoll::force_redraw();
}

/**
 * Called when the user has resized the window, forcing a resize of the graph.
 */
void WinStatsPianoRoll::
changed_graph_size(int graph_xsize, int graph_ysize) {
  PStatPianoRoll::changed_size(graph_xsize, graph_ysize);
}

/**
 * Called when the user selects a new time units from the monitor pulldown
 * menu, this should adjust the units for the graph to the indicated mask if
 * it is a time-based graph.
 */
void WinStatsPianoRoll::
set_time_units(int unit_mask) {
  int old_unit_mask = get_guide_bar_units();
  if ((old_unit_mask & (GBU_hz | GBU_ms)) != 0) {
    unit_mask = unit_mask & (GBU_hz | GBU_ms);
    unit_mask |= (old_unit_mask & GBU_show_units);
    set_guide_bar_units(unit_mask);

    RECT rect;
    GetClientRect(_window, &rect);
    rect.left = _right_margin;
    InvalidateRect(_window, &rect, TRUE);
  }
}

/**
 * Called when the user single-clicks on a label.
 */
void WinStatsPianoRoll::
clicked_label(int collector_index) {
  if (collector_index >= 0) {
    WinStatsGraph::_monitor->open_strip_chart(_thread_index, collector_index, false);
  }
}

/**
 * Changes the amount of time the width of the horizontal axis represents.
 * This may force a redraw.
 */
void WinStatsPianoRoll::
set_horizontal_scale(double time_width) {
  PStatPianoRoll::set_horizontal_scale(time_width);

  RECT rect;
  GetClientRect(_window, &rect);
  rect.bottom = _top_margin;
  InvalidateRect(_window, &rect, TRUE);
}

/**
 * Erases the chart area.
 */
void WinStatsPianoRoll::
clear_region() {
  RECT rect = { 0, 0, get_xsize(), get_ysize() };
  FillRect(_bitmap_dc, &rect, (HBRUSH)GetStockObject(WHITE_BRUSH));
}

/**
 * Erases the chart area in preparation for drawing a bunch of bars.
 */
void WinStatsPianoRoll::
begin_draw() {
  clear_region();

  // Draw in the guide bars.
  int num_guide_bars = get_num_guide_bars();
  for (int i = 0; i < num_guide_bars; i++) {
    draw_guide_bar(_bitmap_dc, get_guide_bar(i));
  }
}

/**
 * Draws a single bar on the chart.
 */
void WinStatsPianoRoll::
draw_bar(int row, int from_x, int to_x) {
  if (row >= 0 && row < _label_stack.get_num_labels()) {
    int y = _label_stack.get_label_y(row) - _graph_top;
    int height = _label_stack.get_label_height(row);

    RECT rect = {
      from_x, y - height + 2,
      to_x, y - 2,
    };
    int collector_index = get_label_collector(row);
    HBRUSH brush = get_collector_brush(collector_index);
    FillRect(_bitmap_dc, &rect, brush);
  }
}

/**
 * Called after all the bars have been drawn, this triggers a refresh event to
 * draw it to the window.
 */
void WinStatsPianoRoll::
end_draw() {
  InvalidateRect(_graph_window, nullptr, FALSE);
}

/**
 * Called at the end of the draw cycle.
 */
void WinStatsPianoRoll::
idle() {
  if (_labels_changed) {
    update_labels();
  }
}

/**
 *
 */
LONG WinStatsPianoRoll::
window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  switch (msg) {
  case WM_LBUTTONDOWN:
    if (_potential_drag_mode == DM_new_guide_bar) {
      set_drag_mode(DM_new_guide_bar);
      SetCapture(_graph_window);
      return 0;
    }
    break;

  default:
    break;
  }

  return WinStatsGraph::window_proc(hwnd, msg, wparam, lparam);
}

/**
 *
 */
LONG WinStatsPianoRoll::
graph_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  switch (msg) {
  case WM_LBUTTONDOWN:
    if (_potential_drag_mode == DM_none) {
      set_drag_mode(DM_scale);
      int16_t x = LOWORD(lparam);
      _drag_scale_start = pixel_to_height(x);
      SetCapture(_graph_window);
      return 0;

    } else if (_potential_drag_mode == DM_guide_bar && _drag_guide_bar >= 0) {
      set_drag_mode(DM_guide_bar);
      int16_t x = LOWORD(lparam);
      _drag_start_x = x;
      SetCapture(_graph_window);
      return 0;
    }
    break;

  case WM_MOUSEMOVE:
    if (_drag_mode == DM_none && _potential_drag_mode == DM_none) {
      // When the mouse is over a color bar, highlight it.
      int16_t x = LOWORD(lparam);
      int16_t y = HIWORD(lparam);
      _label_stack.highlight_label(get_collector_under_pixel(x, y));

      // Now we want to get a WM_MOUSELEAVE when the mouse leaves the graph
      // window.
      TRACKMOUSEEVENT tme = {
        sizeof(TRACKMOUSEEVENT),
        TME_LEAVE,
        _graph_window,
        0
      };
      TrackMouseEvent(&tme);

    } else {
      // If the mouse is in some drag mode, stop highlighting.
      _label_stack.highlight_label(-1);
    }

    if (_drag_mode == DM_scale) {
      int16_t x = LOWORD(lparam);
      double ratio = (double)x / (double)get_xsize();
      if (ratio > 0.0f) {
        set_horizontal_scale(_drag_scale_start / ratio);
      }
      return 0;

    } else if (_drag_mode == DM_new_guide_bar) {
      // We haven't created the new guide bar yet; we won't until the mouse
      // comes within the graph's region.
      int16_t x = LOWORD(lparam);
      if (x >= 0 && x < get_xsize()) {
        set_drag_mode(DM_guide_bar);
        _drag_guide_bar = add_user_guide_bar(pixel_to_height(x));
        return 0;
      }

    } else if (_drag_mode == DM_guide_bar) {
      int16_t x = LOWORD(lparam);
      move_user_guide_bar(_drag_guide_bar, pixel_to_height(x));
      return 0;
    }
    break;

  case WM_MOUSELEAVE:
    // When the mouse leaves the graph, stop highlighting.
    _label_stack.highlight_label(-1);
    break;

  case WM_LBUTTONUP:
    if (_drag_mode == DM_scale) {
      set_drag_mode(DM_none);
      ReleaseCapture();
      return 0;

    } else if (_drag_mode == DM_guide_bar) {
      int16_t x = LOWORD(lparam);
      if (x < 0 || x >= get_xsize()) {
        remove_user_guide_bar(_drag_guide_bar);
      } else {
        move_user_guide_bar(_drag_guide_bar, pixel_to_height(x));
      }
      set_drag_mode(DM_none);
      ReleaseCapture();
      return 0;
    }
    break;

  case WM_LBUTTONDBLCLK:
    {
      // Double-clicking on a color bar in the graph is the same as double-
      // clicking on the corresponding label.
      int16_t x = LOWORD(lparam);
      int16_t y = HIWORD(lparam);
      clicked_label(get_collector_under_pixel(x, y));
      return 0;
    }
    break;

  default:
    break;
  }

  return WinStatsGraph::graph_window_proc(hwnd, msg, wparam, lparam);
}

/**
 * This is called during the servicing of WM_PAINT; it gives a derived class
 * opportunity to do some further painting into the window (the outer window,
 * not the graph window).
 */
void WinStatsPianoRoll::
additional_window_paint(HDC hdc) {
  // Draw in the labels for the guide bars.
  HFONT hfnt = (HFONT)GetStockObject(ANSI_VAR_FONT);
  SelectObject(hdc, hfnt);
  SetTextAlign(hdc, TA_LEFT | TA_BOTTOM);
  SetBkMode(hdc, TRANSPARENT);

  int y = _top_margin;

  int i;
  int num_guide_bars = get_num_guide_bars();
  for (i = 0; i < num_guide_bars; i++) {
    draw_guide_label(hdc, y, get_guide_bar(i));
  }

  int num_user_guide_bars = get_num_user_guide_bars();
  for (i = 0; i < num_user_guide_bars; i++) {
    draw_guide_label(hdc, y, get_user_guide_bar(i));
  }
}

/**
 * This is called during the servicing of WM_PAINT; it gives a derived class
 * opportunity to do some further painting into the window (the outer window,
 * not the graph window).
 */
void WinStatsPianoRoll::
additional_graph_window_paint(HDC hdc) {
  int num_user_guide_bars = get_num_user_guide_bars();
  for (int i = 0; i < num_user_guide_bars; i++) {
    draw_guide_bar(hdc, get_user_guide_bar(i));
  }
}

/**
 * Based on the mouse position within the window's client area, look for
 * draggable things the mouse might be hovering over and return the
 * apprioprate DragMode enum or DM_none if nothing is indicated.
 */
WinStatsGraph::DragMode WinStatsPianoRoll::
consider_drag_start(int mouse_x, int mouse_y, int width, int height) {
  if (mouse_y >= _graph_top && mouse_y < _graph_top + get_ysize()) {
    if (mouse_x >= _graph_left && mouse_x < _graph_left + get_xsize()) {
      // See if the mouse is over a user-defined guide bar.
      int x = mouse_x - _graph_left;
      double from_height = pixel_to_height(x - 2);
      double to_height = pixel_to_height(x + 2);
      _drag_guide_bar = find_user_guide_bar(from_height, to_height);
      if (_drag_guide_bar >= 0) {
        return DM_guide_bar;
      }

    } else if (mouse_x < _left_margin - 2 ||
               mouse_x > width - _right_margin + 2) {
      // The mouse is left or right of the graph; maybe create a new guide
      // bar.
      return DM_new_guide_bar;
    }
  }

  return WinStatsGraph::consider_drag_start(mouse_x, mouse_y, width, height);
}

/**
 * Returns the collector index associated with the indicated vertical row, or
 * -1.
 */
int WinStatsPianoRoll::
get_collector_under_pixel(int xpoint, int ypoint) {
  if (_label_stack.get_num_labels() == 0) {
    return -1;
  }

  // Assume all of the labels are the same height.
  int height = _label_stack.get_label_height(0);
  int row = (get_ysize() - ypoint) / height;
  if (row >= 0 && row < _label_stack.get_num_labels()) {
    return _label_stack.get_label_collector_index(row);
  } else {
    return -1;
  }
}

/**
 * Resets the list of labels.
 */
void WinStatsPianoRoll::
update_labels() {
  _label_stack.clear_labels();
  for (int i = 0; i < get_num_labels(); i++) {
    int label_index =
      _label_stack.add_label(WinStatsGraph::_monitor, this,
                             _thread_index,
                             get_label_collector(i), true);
  }
  _labels_changed = false;
}

/**
 * Draws the line for the indicated guide bar on the graph.
 */
void WinStatsPianoRoll::
draw_guide_bar(HDC hdc, const PStatGraph::GuideBar &bar) {
  int x = height_to_pixel(bar._height);

  if (x > 0 && x < get_xsize() - 1) {
    // Only draw it if it's not too close to either edge.
    switch (bar._style) {
    case GBS_target:
      SelectObject(hdc, _light_pen);
      break;

    case GBS_user:
      SelectObject(hdc, _user_guide_bar_pen);
      break;

    case GBS_normal:
      SelectObject(hdc, _dark_pen);
      break;
    }
    MoveToEx(hdc, x, 0, nullptr);
    LineTo(hdc, x, get_ysize());
  }
}

/**
 * Draws the text for the indicated guide bar label at the top of the graph.
 */
void WinStatsPianoRoll::
draw_guide_label(HDC hdc, int y, const PStatGraph::GuideBar &bar) {
  switch (bar._style) {
  case GBS_target:
    SetTextColor(hdc, _light_color);
    break;

  case GBS_user:
    SetTextColor(hdc, _user_guide_bar_color);
    break;

  case GBS_normal:
    SetTextColor(hdc, _dark_color);
    break;
  }

  int x = height_to_pixel(bar._height);
  const std::string &label = bar._label;
  SIZE size;
  GetTextExtentPoint32(hdc, label.data(), label.length(), &size);

  if (bar._style != GBS_user) {
    double from_height = pixel_to_height(x - size.cx);
    double to_height = pixel_to_height(x + size.cx);
    if (find_user_guide_bar(from_height, to_height) >= 0) {
      // Omit the label: there's a user-defined guide bar in the same space.
      return;
    }
  }

  int this_x = _graph_left + x - size.cx / 2;
  if (x >= 0 && x < get_xsize()) {
    TextOut(hdc, this_x, y,
            label.data(), label.length());
  }
}

/**
 * Creates the window for this strip chart.
 */
void WinStatsPianoRoll::
create_window() {
  if (_window) {
    return;
  }

  HINSTANCE application = GetModuleHandle(nullptr);
  register_window_class(application);

  const PStatClientData *client_data =
    WinStatsGraph::_monitor->get_client_data();
  std::string thread_name = client_data->get_thread_name(_thread_index);
  std::string window_title = thread_name + " thread piano roll";


  RECT win_rect = {
    0, 0,
    _left_margin + get_xsize() + _right_margin,
    _top_margin + get_ysize() + _bottom_margin
  };

  // compute window size based on desired client area size
  AdjustWindowRect(&win_rect, graph_window_style, FALSE);

  _window =
    CreateWindow(_window_class_name, window_title.c_str(), graph_window_style,
                 CW_USEDEFAULT, CW_USEDEFAULT,
                 win_rect.right - win_rect.left,
                 win_rect.bottom - win_rect.top,
                 WinStatsGraph::_monitor->get_window(), nullptr, application, 0);
  if (!_window) {
    nout << "Could not create PianoRoll window!\n";
    exit(1);
  }

  SetWindowLongPtr(_window, 0, (LONG_PTR)this);
  setup_label_stack();

  // Ensure that the window is on top of the stack.
  SetWindowPos(_window, HWND_TOP, 0, 0, 0, 0,
               SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
}

/**
 * Registers the window class for the pianoRoll window, if it has not already
 * been registered.
 */
void WinStatsPianoRoll::
register_window_class(HINSTANCE application) {
  if (_window_class_registered) {
    return;
  }

  WNDCLASS wc;

  ZeroMemory(&wc, sizeof(WNDCLASS));
  wc.style = 0;
  wc.lpfnWndProc = (WNDPROC)static_window_proc;
  wc.hInstance = application;
  wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)COLOR_BACKGROUND;
  wc.lpszMenuName = nullptr;
  wc.lpszClassName = _window_class_name;

  // Reserve space to associate the this pointer with the window.
  wc.cbWndExtra = sizeof(WinStatsPianoRoll *);

  if (!RegisterClass(&wc)) {
    nout << "Could not register PianoRoll window class!\n";
    exit(1);
  }

  _window_class_registered = true;
}

/**
 *
 */
LONG WINAPI WinStatsPianoRoll::
static_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  WinStatsPianoRoll *self = (WinStatsPianoRoll *)GetWindowLongPtr(hwnd, 0);
  if (self != nullptr && self->_window == hwnd) {
    return self->window_proc(hwnd, msg, wparam, lparam);
  } else {
    return DefWindowProc(hwnd, msg, wparam, lparam);
  }
}
