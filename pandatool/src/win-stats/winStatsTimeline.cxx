/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file winStatsTimeline.cxx
 * @author rdb
 * @date 2022-02-11
 */

#include "winStatsTimeline.h"
#include "winStatsMonitor.h"
#include "numeric_types.h"

static const int default_timeline_width = 1000;
static const int default_timeline_height = 500;

bool WinStatsTimeline::_window_class_registered = false;
const char * const WinStatsTimeline::_window_class_name = "timeline";

/**
 *
 */
WinStatsTimeline::
WinStatsTimeline(WinStatsMonitor *monitor) :
  PStatTimeline(monitor,
                monitor->get_pixel_scale() * default_timeline_width / 4,
                monitor->get_pixel_scale() * default_timeline_height / 4),
  WinStatsGraph(monitor)
{
  _left_margin = _pixel_scale * 24;
  _right_margin = _pixel_scale * 2;
  _top_margin = _pixel_scale * 5;
  _bottom_margin = _pixel_scale * 2;

  normal_guide_bars();

  create_window();
  clear_region();

  _grid_brush = CreateSolidBrush(RGB(0xdd, 0xdd, 0xdd));
}

/**
 *
 */
WinStatsTimeline::
~WinStatsTimeline() {
}

/**
 * Called as each frame's data is made available.  There is no guarantee the
 * frames will arrive in order, or that all of them will arrive at all.  The
 * monitor should be prepared to accept frames received out-of-order or
 * missing.
 */
void WinStatsTimeline::
new_data(int thread_index, int frame_number) {
  PStatTimeline::new_data(thread_index, frame_number);
}

/**
 * Called when it is necessary to redraw the entire graph.
 */
void WinStatsTimeline::
force_redraw() {
  PStatTimeline::force_redraw();
}

/**
 * Called when the user has resized the window, forcing a resize of the graph.
 */
void WinStatsTimeline::
changed_graph_size(int graph_xsize, int graph_ysize) {
  PStatTimeline::changed_size(graph_xsize, graph_ysize);
}

/**
 * Erases the chart area.
 */
void WinStatsTimeline::
clear_region() {
  RECT rect = { 0, 0, get_xsize(), get_ysize() };
  FillRect(_bitmap_dc, &rect, (HBRUSH)GetStockObject(WHITE_BRUSH));
}

/**
 * Erases the chart area in preparation for drawing a bunch of bars.
 */
void WinStatsTimeline::
begin_draw() {
  SelectObject(_bitmap_dc, WinStatsGraph::_monitor->get_font());
  SelectObject(_bitmap_dc, GetStockObject(NULL_PEN));
  SetBkMode(_bitmap_dc, TRANSPARENT);
  SetTextAlign(_bitmap_dc, TA_LEFT | TA_TOP | TA_NOUPDATECP);
}

/**
 * Draws a horizontal separator.
 */
void WinStatsTimeline::
draw_separator(int row) {
  int y = (row_to_pixel(row) + row_to_pixel(row + 1)) / 2;
  RECT rect = {0, y, get_xsize(), y + _pixel_scale / 3};
  FillRect(_bitmap_dc, &rect, _grid_brush);
}

/**
 * Draws a vertical guide bar.  If the row is -1, draws it in all rows.
 */
void WinStatsTimeline::
draw_guide_bar(int x, GuideBarStyle style) {
  int x1 = x - _pixel_scale / 6;
  int x2 = x1 + _pixel_scale / 3;
  if (style == GBS_frame) {
    ++x2;
  }
  RECT rect = {x1, 0, x2, get_ysize()};
  FillRect(_bitmap_dc, &rect, _grid_brush);
}

/**
 * Draws a single bar in the chart for the indicated row, in the color for the
 * given collector, for the indicated horizontal pixel range.
 */
void WinStatsTimeline::
draw_bar(int row, int from_x, int to_x, int collector_index,
         const std::string &collector_name) {

  int top = row_to_pixel(row);
  int bottom = row_to_pixel(row + 1);

  bool is_highlighted = row == _highlighted_row && _highlighted_x >= from_x && _highlighted_x < to_x;
  HBRUSH brush = get_collector_brush(collector_index, is_highlighted);

  if (to_x < from_x + 2) {
    // It's just a tiny sliver.  This is a more reliable way to draw it.
    RECT rect = {from_x, top + 1, from_x + 1, bottom - 1};
    FillRect(_bitmap_dc, &rect, brush);

    //if (to_x <= from_x + 2) {
    //  // Draw an arrow pointing to it, if it's so small.
    //  POINT vertices[] = {{to_x, bottom}, {to_x - _pixel_scale, bottom + _pixel_scale * 2}, {to_x + _pixel_scale, bottom + _pixel_scale * 2}};
    //  Polygon(_bitmap_dc, vertices, 3);
    //}
  }
  else {
    SelectObject(_bitmap_dc, brush);
    RoundRect(_bitmap_dc,
              std::max(from_x, -_pixel_scale - 1),
              top,
              std::min(std::max(to_x, from_x + 1), get_xsize() + _pixel_scale),
              bottom,
              _pixel_scale,
              _pixel_scale);

    if ((to_x - from_x) >= _pixel_scale * 4) {
      // Only bother drawing the text if we've got some space to draw on.
      // Choose a suitable foreground color.
      SetTextColor(_bitmap_dc, get_collector_text_color(collector_index, is_highlighted));

      // Make sure that the text doesn't run off the chart.
      SIZE size;
      GetTextExtentPoint32(_bitmap_dc, collector_name.data(), collector_name.size(), &size);
      int center = (from_x + to_x) / 2;
      int left = std::max(from_x, 0) + _pixel_scale / 2;
      int right = std::min(to_x, get_xsize()) - _pixel_scale / 2;

      if (size.cx >= right - left) {
        if (right - left < _pixel_scale * 6) {
          // It's a really tiny space.  Draw a single letter.
          RECT rect = {left, top, right, bottom};
          DrawText(_bitmap_dc, collector_name.data(), 1,
                   &rect, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
        } else {
          // It's going to be tricky to fit it, let Windows figure it out via
          // the more expensive DrawText call.
          RECT rect = {left, top, right, bottom};
          DrawText(_bitmap_dc, collector_name.data(), collector_name.size(),
                   &rect, DT_CENTER | DT_END_ELLIPSIS | DT_SINGLELINE | DT_VCENTER);
        }
      }
      else {
        int text_top = top + (bottom - top - size.cy) / 2;
        if (center - size.cx / 2 < 0) {
          // Put it against the left-most edge.
          TextOut(_bitmap_dc, _pixel_scale, text_top,
                  collector_name.data(), collector_name.length());
        }
        else if (center + size.cx / 2 >= get_xsize()) {
          // Put it against the right-most edge.
          TextOut(_bitmap_dc, get_xsize() - _pixel_scale - size.cx, text_top,
                  collector_name.data(), collector_name.length());
        }
        else {
          // It fits just fine, center it.
          TextOut(_bitmap_dc, center - size.cx / 2, text_top,
                  collector_name.data(), collector_name.length());
        }
      }
    }
  }
}

/**
 * Called after all the bars have been drawn, this triggers a refresh event to
 * draw it to the window.
 */
void WinStatsTimeline::
end_draw() {
  InvalidateRect(_graph_window, nullptr, FALSE);

  if (_threads_changed) {
    RECT rect;
    GetClientRect(_window, &rect);
    rect.top = _top_margin;
    rect.right = _left_margin;
    InvalidateRect(_window, &rect, TRUE);
    _threads_changed = false;
  }

  if (_guide_bars_changed) {
    RECT rect;
    GetClientRect(_window, &rect);
    rect.bottom = _top_margin;
    InvalidateRect(_window, &rect, TRUE);
    _guide_bars_changed = false;
  }
}

/**
 * Called at the end of the draw cycle.
 */
void WinStatsTimeline::
idle() {
}

/**
 * Overridden by a derived class to implement an animation.  If it returns
 * false, the animation timer is stopped.
 */
bool WinStatsTimeline::
animate(double time, double dt) {
  return PStatTimeline::animate(time, dt);
}

/**
 * Returns the current window dimensions.
 */
bool WinStatsTimeline::
get_window_state(int &x, int &y, int &width, int &height,
                 bool &maximized, bool &minimized) const {
  WinStatsGraph::get_window_state(x, y, width, height, maximized, minimized);
  return true;
}

/**
 * Called to restore the graph window to its previous dimensions.
 */
void WinStatsTimeline::
set_window_state(int x, int y, int width, int height,
                 bool maximized, bool minimized) {
  WinStatsGraph::set_window_state(x, y, width, height, maximized, minimized);
}

/**
 *
 */
LONG WinStatsTimeline::
window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  switch (msg) {
  case WM_MOUSELEAVE:
    SetFocus(nullptr);
    break;

  default:
    break;
  }

  return WinStatsGraph::window_proc(hwnd, msg, wparam, lparam);
}

/**
 *
 */
LONG WinStatsTimeline::
graph_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  switch (msg) {
  case WM_LBUTTONDOWN:
    if (_potential_drag_mode == DM_none) {
      set_drag_mode(DM_pan);
      int16_t x = LOWORD(lparam);
      _drag_start_x = x;
      _scroll_speed = 0.0;
      _zoom_center = pixel_to_timestamp(x);
      SetCapture(_graph_window);
      return 0;
    }
    break;

  case WM_MOUSEMOVE:
    // Make sure we can accept keyboard events, except if we're inactive.
    if (GetActiveWindow() == _window) {
      SetFocus(hwnd);
    }

    if (_drag_mode == DM_none && _potential_drag_mode == DM_none) {
      // When the mouse is over a color bar, highlight it.
      int x = LOWORD(lparam);
      int y = HIWORD(lparam);
      double time = pixel_to_timestamp(x);

      int row = pixel_to_row(y);

      if (row != _highlighted_row) {
        clear_graph_tooltip();
      }
      else if (_highlighted_row >= 0) {
        // Is the mouse on the same bar?  If not, clear the tooltip.
        ColorBar bar;
        if (find_bar(row, x, bar)) {
          double prev_time = pixel_to_timestamp(_highlighted_x);
          if (prev_time < bar._start || prev_time > bar._end) {
            clear_graph_tooltip();
          }
        } else {
          clear_graph_tooltip();
        }
      }

      std::swap(_highlighted_x, x);
      std::swap(_highlighted_row, row);

      if (row >= 0) {
        PStatTimeline::force_redraw(row, x, x);
      }
      PStatTimeline::force_redraw(_highlighted_row, _highlighted_x, _highlighted_x);

      if ((_keys_held & (F_w | F_s)) != 0) {
        // Update the zoom center if we move the mouse while zooming with the
        // keyboard.
        _zoom_center = time;
      }

      // Now we want to get a WM_MOUSELEAVE when the mouse leaves the graph
      // window.
      TRACKMOUSEEVENT tme = {
        sizeof(TRACKMOUSEEVENT),
        TME_LEAVE,
        _graph_window,
        0
      };
      TrackMouseEvent(&tme);
    }
    else {
      // If the mouse is in some drag mode, stop highlighting.
      if (_highlighted_row != -1) {
        int row = _highlighted_row;
        _highlighted_row = -1;
        PStatTimeline::force_redraw(row, _highlighted_x, _highlighted_x);
        clear_graph_tooltip();
      }
    }

    if (_drag_mode == DM_pan) {
      int16_t x = LOWORD(lparam);
      int delta = _drag_start_x - x;
      set_horizontal_scroll(get_horizontal_scroll() + pixel_to_height(delta));
      _drag_start_x = x;
      return 0;
    }
    break;

  case WM_MOUSELEAVE:
    // When the mouse leaves the graph, stop highlighting.
    if (_highlighted_row != -1) {
      int row = _highlighted_row;
      _highlighted_row = -1;
      PStatTimeline::force_redraw(row, _highlighted_x, _highlighted_x);
      clear_graph_tooltip();
    }
    SetFocus(nullptr);
    break;

  case WM_LBUTTONUP:
    if (_drag_mode == DM_pan) {
      set_drag_mode(DM_none);
      ReleaseCapture();
      return 0;
    }
    break;

  case WM_LBUTTONDBLCLK:
    {
      // Double-clicking on a color bar in the graph will zoom the graph into
      // that collector.
      int16_t x = LOWORD(lparam);
      int16_t y = HIWORD(lparam);
      int row = pixel_to_row(y);
      ColorBar bar;
      if (find_bar(row, x, bar)) {
        double width = bar._end - bar._start;
        zoom_to(width * 1.5, pixel_to_timestamp(x));
        scroll_to(bar._start - width / 4.0);
      } else {
        // Double-clicking the white area zooms out.
        _zoom_speed -= 100.0;
      }
      start_animation();
      return 0;
    }
    break;

  case WM_CONTEXTMENU:
    {
      // Right-clicking a color bar brings up a context menu.
      POINT point;
      if (GetCursorPos(&point)) {
        POINT graph_point =  point;
        if (ScreenToClient(_graph_window, &graph_point)) {
          int row = pixel_to_row(graph_point.y);
          ColorBar bar;
          if (find_bar(row, graph_point.x, bar)) {
            _popup_bar = bar;

            HMENU popup = CreatePopupMenu();

            std::string label = get_bar_tooltip(row, graph_point.x);
            if (!label.empty()) {
              AppendMenu(popup, MF_STRING | MF_DISABLED, 0, label.c_str());
            }
            AppendMenu(popup, MF_STRING, 101, "Zoom To");
            AppendMenu(popup, MF_STRING, 102, "Open Strip Chart");
            AppendMenu(popup, MF_STRING, 103, "Open Flame Graph");
            AppendMenu(popup, MF_STRING, 104, "Open Piano Roll");
            AppendMenu(popup, MF_STRING | MF_SEPARATOR, 0, nullptr);
            AppendMenu(popup, MF_STRING, 105, "Change Color...");
            AppendMenu(popup, MF_STRING, 106, "Reset Color");
            TrackPopupMenu(popup, TPM_LEFTBUTTON, point.x, point.y, 0, _graph_window, nullptr);
          }
        }
      }
      return 0;
    }
    break;

  case WM_COMMAND:
    switch (LOWORD(wparam)) {
    case 101:
      {
        double width = _popup_bar._end - _popup_bar._start;
        zoom_to(width * 1.5, (_popup_bar._end + _popup_bar._start) / 2.0);
        scroll_to(_popup_bar._start - width / 4.0);
        start_animation();
      }
      return 0;

    case 102:
      WinStatsGraph::_monitor->open_strip_chart(_popup_bar._thread_index, _popup_bar._collector_index, false);
      return 0;

    case 103:
      WinStatsGraph::_monitor->open_flame_graph(_popup_bar._thread_index, _popup_bar._collector_index, _popup_bar._frame_number);
      return 0;

    case 104:
      WinStatsGraph::_monitor->open_piano_roll(_popup_bar._thread_index);
      return 0;

    case 105:
      WinStatsGraph::_monitor->choose_collector_color(_popup_bar._collector_index);
      return 0;

    case 106:
      WinStatsGraph::_monitor->reset_collector_color(_popup_bar._collector_index);
      return 0;
    }
    break;

  case WM_MOUSEWHEEL:
    {
      if (GET_KEYSTATE_WPARAM(wparam) & MK_CONTROL) {
        // Zoom in/out around the cursor position.
        POINT point;
        if (GetCursorPos(&point) && ScreenToClient(_graph_window, &point)) {
          int delta = GET_WHEEL_DELTA_WPARAM(wparam);
          zoom_by(delta / 120.0, pixel_to_timestamp(point.x));
          start_animation();
        }
      } else {
        int delta = GET_WHEEL_DELTA_WPARAM(wparam);
        delta = (delta * _pixel_scale * 5) / 120;
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
          _threads_changed = true;
          PStatTimeline::force_redraw();
        }
      }
      return 0;
    }
    break;

  case WM_MOUSEHWHEEL:
    {
      int delta = GET_WHEEL_DELTA_WPARAM(wparam);
      _scroll_speed += delta / 12.0;
      start_animation();
      return 0;
    }
    break;

  case WM_KEYDOWN:
  case WM_KEYUP:
    {
      int flag = 0;
      int vsc = (lparam & 0xff0000) >> 16;
      if ((lparam & 0x1000000) == 0) {
        // Accept WASD based on their position rather than their mapping
        switch (vsc) {
        case 17:
          flag = F_w;
          break;
        case 30:
          flag = F_a;
          break;
        case 31:
          flag = F_s;
          break;
        case 32:
          flag = F_d;
          break;
        }
      }
      if (flag == 0) {
        switch (wparam) {
        case VK_LEFT:
          flag = F_left;
          break;
        case VK_RIGHT:
          flag = F_right;
          break;
        case 'W':
          flag = F_w;
          break;
        case 'A':
          flag = F_a;
          break;
        case 'S':
          flag = F_s;
          break;
        case 'D':
          flag = F_d;
          break;
        }
      }
      if (flag != 0) {
        if (msg == WM_KEYDOWN) {
          if (flag & (F_w | F_s)) {
            POINT point;
            if (GetCursorPos(&point) && ScreenToClient(_graph_window, &point)) {
              _zoom_center = pixel_to_timestamp(point.x);
            } else {
              _zoom_center = get_horizontal_scroll() + get_horizontal_scale() / 2.0;
            }
          }
          if (_keys_held == 0) {
            start_animation();
          }
          _keys_held |= flag;
        }
        else if (_keys_held != 0) {
          _keys_held &= ~flag;
        }
      }
    }
    break;

  case WM_KILLFOCUS:
    _keys_held = 0;
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
void WinStatsTimeline::
additional_window_paint(HDC hdc) {
  // Draw in the labels for the guide bars.
  SelectObject(hdc, WinStatsGraph::_monitor->get_font());
  SetTextAlign(hdc, TA_LEFT | TA_BOTTOM);
  SetBkMode(hdc, TRANSPARENT);

  int y = _top_margin - 2;

  int num_guide_bars = get_num_guide_bars();
  for (int i = 0; i < num_guide_bars; ++i) {
    draw_guide_label(hdc, y, get_guide_bar(i));
  }

  SetTextColor(hdc, _dark_color);
  SetTextAlign(hdc, TA_LEFT | TA_TOP | TA_NOUPDATECP);

  for (const ThreadRow &thread_row : _threads) {
    if (thread_row._visible) {
      draw_thread_label(hdc, thread_row);
    }
  }
}

/**
 * This is called during the servicing of WM_PAINT; it gives a derived class
 * opportunity to do some further painting into the window (the outer window,
 * not the graph window).
 */
void WinStatsTimeline::
additional_graph_window_paint(HDC hdc) {
}

/**
 * Called when the mouse hovers over the graph, and should return the text that
 * should appear on the tooltip.
 */
std::string WinStatsTimeline::
get_graph_tooltip(int mouse_x, int mouse_y) const {
  return PStatTimeline::get_bar_tooltip(pixel_to_row(mouse_y), mouse_x);
}

/**
 * Based on the mouse position within the window's client area, look for
 * draggable things the mouse might be hovering over and return the
 * apprioprate DragMode enum or DM_none if nothing is indicated.
 */
WinStatsGraph::DragMode WinStatsTimeline::
consider_drag_start(int mouse_x, int mouse_y, int width, int height) {
  DragMode mode = WinStatsGraph::consider_drag_start(mouse_x, mouse_y, width, height);
  if (mode == DM_right_margin) {
    mode = DM_none;
  }
  return mode;
}

/**
 * Draws the text for the indicated guide bar label at the top of the graph.
 */
void WinStatsTimeline::
draw_guide_label(HDC hdc, int y, const PStatGraph::GuideBar &bar) {
  const std::string &label = bar._label;
  if (label.empty()) {
    return;
  }

  bool center = true;
  switch (bar._style) {
  case GBS_target:
    SetTextColor(hdc, _light_color);
    break;

  case GBS_user:
    SetTextColor(hdc, _user_guide_bar_color);
    break;

  case GBS_normal:
    SetTextColor(hdc, _light_color);
    break;

  case GBS_frame:
    SetTextColor(hdc, _dark_color);
    center = false;
    break;
  }

  int x = timestamp_to_pixel(bar._height);
  SIZE size;
  GetTextExtentPoint32(hdc, label.data(), label.length(), &size);

  int this_x = _graph_left + x;
  if (center) {
    this_x -= size.cx / 2;
  }
  if (x >= 0 && x < get_xsize()) {
    TextOut(hdc, this_x, y,
            label.data(), label.length());
  }
}

/**
 * Draws the text for the indicated thread on the side of the graph.
 */
void WinStatsTimeline::
draw_thread_label(HDC hdc, const ThreadRow &thread_row) {
  int top = row_to_pixel(thread_row._row_offset + 1);
  int bottom = row_to_pixel(thread_row._row_offset + 2);

  RECT rect = {_pixel_scale * 2, top, _left_margin - _pixel_scale * 2, bottom};
  DrawText(hdc, thread_row._label.data(), thread_row._label.size(),
           &rect, DT_RIGHT | DT_END_ELLIPSIS | DT_SINGLELINE | DT_VCENTER);
}

/**
 * Creates the window for this strip chart.
 */
void WinStatsTimeline::
create_window() {
  if (_window) {
    return;
  }

  HINSTANCE application = GetModuleHandle(nullptr);
  register_window_class(application);

  POINT window_pos = WinStatsGraph::_monitor->get_new_window_pos();

  RECT win_rect = {
    0, 0,
    _left_margin + get_xsize() + _right_margin,
    _top_margin + get_ysize() + _bottom_margin
  };

  // compute window size based on desired client area size
  AdjustWindowRect(&win_rect, graph_window_style, FALSE);

  _window =
    CreateWindowEx(WS_EX_DLGMODALFRAME, _window_class_name,
                   "Timeline", graph_window_style,
                   window_pos.x, window_pos.y,
                   win_rect.right - win_rect.left,
                   win_rect.bottom - win_rect.top,
                   WinStatsGraph::_monitor->get_window(), nullptr, application, 0);
  if (!_window) {
    nout << "Could not create timeline window!\n";
    exit(1);
  }

  SetWindowLongPtr(_window, 0, (LONG_PTR)this);

  // Ensure that the window is on top of the stack.
  SetWindowPos(_window, HWND_TOP, 0, 0, 0, 0,
               SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);

  SetFocus(_window);
}

/**
 * Registers the window class for the Timeline window, if it has not already
 * been registered.
 */
void WinStatsTimeline::
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
  wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
  wc.lpszMenuName = nullptr;
  wc.lpszClassName = _window_class_name;

  // Reserve space to associate the this pointer with the window.
  wc.cbWndExtra = sizeof(WinStatsTimeline *);

  if (!RegisterClass(&wc)) {
    nout << "Could not register Timeline window class!\n";
    exit(1);
  }

  _window_class_registered = true;
}

/**
 *
 */
LONG WINAPI WinStatsTimeline::
static_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  WinStatsTimeline *self = (WinStatsTimeline *)GetWindowLongPtr(hwnd, 0);
  if (self != nullptr && self->_window == hwnd) {
    return self->window_proc(hwnd, msg, wparam, lparam);
  } else {
    return DefWindowProc(hwnd, msg, wparam, lparam);
  }
}
