/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file winStatsGraph.cxx
 * @author drose
 * @date 2003-12-03
 */

#include "winStatsGraph.h"
#include "winStatsMonitor.h"
#include "winStatsLabelStack.h"
#include "winStatsServer.h"
#include "trueClock.h"
#include "convert_srgb.h"

#include <commctrl.h>

#define IDC_GRAPH 100

DWORD WinStatsGraph::graph_window_style =
  WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_OVERLAPPEDWINDOW | WS_VISIBLE;

/**
 *
 */
WinStatsGraph::
WinStatsGraph(WinStatsMonitor *monitor) :
  _monitor(monitor)
{
  _window = 0;
  _graph_window = 0;
  _tooltip_window = 0;
  _sizewe_cursor = LoadCursor(nullptr, IDC_SIZEWE);
  _hand_cursor = LoadCursor(nullptr, IDC_HAND);
  _bitmap = 0;
  _bitmap_dc = 0;

  _graph_left = 0;
  _graph_top = 0;
  _bitmap_xsize = 0;
  _bitmap_ysize = 0;

  _pixel_scale = monitor->get_pixel_scale();

  // Default margins.
  int margin = _pixel_scale * 2;
  _left_margin = margin;
  _right_margin = margin;
  _top_margin = margin;
  _bottom_margin = margin;
  _top_label_stack_margin = margin;

  _dark_color = RGB(51, 51, 51);
  _light_color = RGB(154, 154, 154);
  _user_guide_bar_color = RGB(130, 150, 255);
  _frame_guide_bar_color = RGB(255, 10, 10);
  _dark_pen = CreatePen(PS_SOLID, 1, _dark_color);
  _light_pen = CreatePen(PS_SOLID, 1, _light_color);
  _user_guide_bar_pen = CreatePen(PS_DASH, 1, _user_guide_bar_color);
  _frame_guide_bar_pen = CreatePen(PS_DASH, 1, _frame_guide_bar_color);

  _drag_mode = DM_none;
  _potential_drag_mode = DM_none;
  _drag_scale_start = 0.0f;

  _pause = false;
}

/**
 *
 */
WinStatsGraph::
~WinStatsGraph() {
  _monitor = nullptr;
  release_bitmap();

  DeleteObject(_dark_pen);
  DeleteObject(_light_pen);
  DeleteObject(_user_guide_bar_pen);
  DeleteObject(_frame_guide_bar_pen);

  for (auto &item : _brushes) {
    DeleteObject(item.second.first);
    DeleteObject(item.second.second);
  }
  _brushes.clear();
  _text_colors.clear();

  if (_graph_window) {
    DestroyWindow(_graph_window);
    _graph_window = 0;
  }

  if (_window) {
    DestroyWindow(_window);
    _window = 0;
  }

  if (_tooltip_window) {
    DestroyWindow(_tooltip_window);
    _tooltip_window = 0;
  }
}

/**
 * Called whenever a new Collector definition is received from the client.
 */
void WinStatsGraph::
new_collector(int new_collector) {
}

/**
 * Called whenever new data arrives.
 */
void WinStatsGraph::
new_data(int thread_index, int frame_number) {
}

/**
 * Called when the user has resized the window, forcing a resize of the graph.
 */
void WinStatsGraph::
changed_graph_size(int graph_xsize, int graph_ysize) {
}

/**
 * Called when the user selects a new time units from the monitor pulldown
 * menu, this should adjust the units for the graph to the indicated mask if
 * it is a time-based graph.
 */
void WinStatsGraph::
set_time_units(int unit_mask) {
}

/**
 * Called when the user selects a new scroll speed from the monitor pulldown
 * menu, this should adjust the speed for the graph to the indicated value.
 */
void WinStatsGraph::
set_scroll_speed(double scroll_speed) {
}

/**
 * Changes the pause flag for the graph.  When this flag is true, the graph
 * does not update in response to new data.
 */
void WinStatsGraph::
set_pause(bool pause) {
  _pause = pause;
}

/**
 * Called when the user guide bars have been changed.
 */
void WinStatsGraph::
user_guide_bars_changed() {
  InvalidateRect(_window, nullptr, TRUE);
  InvalidateRect(_graph_window, nullptr, TRUE);
}

/**
 * Called when the user single-clicks on a label.
 */
void WinStatsGraph::
on_click_label(int collector_index) {
}

/**
 * Called when a pop-up menu should be shown for the label.
 */
void WinStatsGraph::
on_popup_label(int collector_index) {
}

/**
 * Called when the user hovers the mouse over a label.
 */
void WinStatsGraph::
on_enter_label(int collector_index) {
  if (collector_index != _highlighted_index) {
    _highlighted_index = collector_index;
    clear_graph_tooltip();
    force_redraw();
  }
}

/**
 * Called when the user's mouse cursor leaves a label.
 */
void WinStatsGraph::
on_leave_label(int collector_index) {
  if (collector_index == _highlighted_index && collector_index != -1) {
    _highlighted_index = -1;
    clear_graph_tooltip();
    force_redraw();
  }
}

/**
 * Called when the mouse hovers over a label, and should return the text that
 * should appear on the tooltip.
 */
std::string WinStatsGraph::
get_label_tooltip(int collector_index) const {
  return std::string();
}

/**
 * Hides the graph tooltip.
 */
void WinStatsGraph::
clear_graph_tooltip() {
  if (_tooltip_window != 0) {
    SendMessage(_tooltip_window, TTM_POP, 0, 0);
  }
}

/**
 * Returns the window handle of the surrounding window.
 */
HWND WinStatsGraph::
get_window() {
  return _window;
}

/**
 * Should be called when the user closes the associated window.  This tells
 * the monitor to remove the graph.
 */
void WinStatsGraph::
close() {
  WinStatsMonitor *monitor = _monitor;
  _monitor = nullptr;
  if (monitor != nullptr) {
    monitor->remove_graph(this);
  }
}

/**
 * Sets up the label stack on the left edge of the frame.
 */
void WinStatsGraph::
setup_label_stack() {
  _label_stack.setup(_window);
  move_label_stack();
}

/**
 * Repositions the label stack if its coordinates or size have changed.
 */
void WinStatsGraph::
move_label_stack() {
  if (_label_stack.is_setup()) {
    RECT rect;
    GetClientRect(_window, &rect);

    rect.left += _pixel_scale * 2;
    rect.right = _left_margin - _pixel_scale * 2;

    _label_stack.set_pos(rect.left, rect.top,
                         rect.right - rect.left, rect.bottom - rect.top,
                         _top_label_stack_margin, _bottom_margin);
  }
}

/**
 * Turns on the animation timer, if it hasn't already been turned on.
 */
void WinStatsGraph::
start_animation() {
  if (!_timer_running) {
    TrueClock *clock = TrueClock::get_global_ptr();
    _time = clock->get_short_time();
    SetTimer(_window, 0x100, 16, nullptr);
    _timer_running = true;
  }
}

/**
 * Overridden by a derived class to implement an animation.  If it returns
 * false, the animation timer is stopped.
 */
bool WinStatsGraph::
animate(double time, double dt) {
  return false;
}

/**
 * Returns the current window dimensions.
 */
void WinStatsGraph::
get_window_state(int &x, int &y, int &width, int &height,
                 bool &maximized, bool &minimized) const {
  WinStatsServer *server = (WinStatsServer *)_monitor->get_server();
  POINT client_origin = server->get_client_origin();
  WINDOWPLACEMENT wp;
  wp.length = sizeof(WINDOWPLACEMENT);
  GetWindowPlacement(_window, &wp);
  x = wp.rcNormalPosition.left - client_origin.x;
  y = wp.rcNormalPosition.top - client_origin.y;
  width = wp.rcNormalPosition.right - wp.rcNormalPosition.left;
  height = wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;
  maximized = (wp.showCmd == SW_SHOWMAXIMIZED || (wp.flags & WPF_RESTORETOMAXIMIZED) != 0);
  minimized = (wp.showCmd == SW_SHOWMINIMIZED);
}

/**
 * Called to restore the graph window to its previous dimensions.
 */
void WinStatsGraph::
set_window_state(int x, int y, int width, int height,
                 bool maximized, bool minimized) {
  WinStatsServer *server = (WinStatsServer *)_monitor->get_server();
  POINT client_origin = server->get_client_origin();
  WINDOWPLACEMENT wp;
  wp.length = sizeof(WINDOWPLACEMENT);
  wp.flags = maximized ? WPF_RESTORETOMAXIMIZED : 0;
  wp.showCmd = minimized ? SW_SHOWMINIMIZED : (maximized ? SW_SHOWMAXIMIZED : SW_SHOWNORMAL);
  wp.ptMinPosition.x = -1;
  wp.ptMinPosition.y = -1;
  wp.ptMaxPosition.x = -1;
  wp.ptMaxPosition.y = -1;
  wp.rcNormalPosition.left = client_origin.x + x;
  wp.rcNormalPosition.top = client_origin.y + y;
  wp.rcNormalPosition.right = wp.rcNormalPosition.left + width;
  wp.rcNormalPosition.bottom = wp.rcNormalPosition.top + height;

  if (minimized) {
    int x, y;
    _monitor->calc_iconic_graph_window_pos(this, x, y);
    wp.ptMinPosition.x = x;
    wp.ptMinPosition.y = y;
    wp.flags |= WPF_SETMINPOSITION;
  }

  SetWindowPlacement(_window, &wp);
}

/**
 * Returns a brush suitable for drawing in the indicated collector's color.
 */
HBRUSH WinStatsGraph::
get_collector_brush(int collector_index, bool highlight) {
  Brushes::iterator bi;
  bi = _brushes.find(collector_index);
  if (bi != _brushes.end()) {
    return highlight ? (*bi).second.second : (*bi).second.first;
  }

  // Ask the monitor what color this guy should be.
  LRGBColor rgb = _monitor->get_collector_color(collector_index);
  int r = (int)encode_sRGB_uchar((float)rgb[0]);
  int g = (int)encode_sRGB_uchar((float)rgb[1]);
  int b = (int)encode_sRGB_uchar((float)rgb[2]);
  HBRUSH brush = CreateSolidBrush(RGB(r, g, b));

  int hr = (int)encode_sRGB_uchar((float)rgb[0] * 0.75f);
  int hg = (int)encode_sRGB_uchar((float)rgb[1] * 0.75f);
  int hb = (int)encode_sRGB_uchar((float)rgb[2] * 0.75f);
  HBRUSH hbrush = CreateSolidBrush(RGB(hr, hg, hb));

  _brushes[collector_index] = std::make_pair(brush, hbrush);
  return highlight ? hbrush : brush;
}

/**
 * Returns a text color suitable for the given collector.
 */
COLORREF WinStatsGraph::
get_collector_text_color(int collector_index, bool highlight) {
  TextColors::iterator tci;
  tci = _text_colors.find(collector_index);
  if (tci != _text_colors.end()) {
    return highlight ? (*tci).second.second : (*tci).second.first;
  }

  LRGBColor rgb = _monitor->get_collector_color(collector_index);
  double bright =
    rgb[0] * 0.2126 +
    rgb[1] * 0.7152 +
    rgb[2] * 0.0722;
  COLORREF color = bright >= 0.5 ? RGB(0, 0, 0) : RGB(255, 255, 255);
  COLORREF hcolor = bright * 0.75 >= 0.5 ? RGB(0, 0, 0) : RGB(255, 255, 255);

  _text_colors[collector_index] = std::make_pair(color, hcolor);
  return highlight ? hcolor : color;
}

/**
 * Called when the given collector has changed colors.
 */
void WinStatsGraph::
reset_collector_color(int collector_index) {
  _brushes.erase(collector_index);
  _text_colors.erase(collector_index);
  force_redraw();
  _label_stack.update_label_color(collector_index);
}

/**
 * This window_proc should be called up to by the derived classes for any
 * messages that are not specifically handled by the derived class.
 */
LONG WinStatsGraph::
window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  switch (msg) {
  case WM_DESTROY:
    close();
    break;

  case WM_APPCOMMAND:
    if (GET_APPCOMMAND_LPARAM(lparam) == APPCOMMAND_CLOSE) {
      close();
      return TRUE;
    }
    break;

  case WM_GETMINMAXINFO:
    {
      WINDOWINFO winfo;
      GetWindowInfo(hwnd, &winfo);
      MINMAXINFO &minmax = *(MINMAXINFO *)lparam;
      int vminheight = _bottom_margin + _top_margin;
      if (_label_stack.get_num_labels() > 0) {
        // If we have a label, make sure at least one can be shown.
        vminheight = (std::max)(vminheight, _top_label_stack_margin + _label_stack.get_label_height(0) + _bottom_margin);
      }
      minmax.ptMinTrackSize.x = (winfo.rcClient.left - winfo.rcWindow.left)
                              + (winfo.rcWindow.right - winfo.rcClient.right)
                              + (_right_margin + _left_margin);
      minmax.ptMinTrackSize.y = (winfo.rcClient.top - winfo.rcWindow.top)
                              + (winfo.rcWindow.bottom - winfo.rcClient.bottom)
                              + vminheight;
      return 0;
    }

  case WM_WINDOWPOSCHANGING:
    {
      WINDOWPOS &pos = *(WINDOWPOS *)lparam;
      if ((pos.flags & (SWP_NOMOVE | SWP_NOSIZE)) == 0 && IsIconic(hwnd)) {
        _monitor->calc_iconic_graph_window_pos(this, pos.x, pos.y);
      }
    }
    break;

  case WM_SYSCOMMAND:
    if (wparam == SC_MINIMIZE) {
      WINDOWPLACEMENT wp;
      if (GetWindowPlacement(hwnd, &wp)) {
        int x, y;
        _monitor->calc_iconic_graph_window_pos(this, x, y);
        wp.showCmd = SW_SHOWMINIMIZED;
        wp.flags |= WPF_SETMINPOSITION;
        wp.ptMinPosition.x = x;
        wp.ptMinPosition.y = y;
        SetWindowPlacement(hwnd, &wp);
      }
      return 0;
    }
    break;

  case WM_SIZE:
    move_label_stack();
    InvalidateRect(hwnd, nullptr, TRUE);
    break;

  case WM_SIZING:
    set_drag_mode(DM_sizing);
    break;

  case WM_EXITSIZEMOVE:
    set_drag_mode(DM_none);
    break;

  case WM_SETCURSOR:
    {
      // Why is it so hard to ask for the cursor position within the window's
      // client area?
      POINT point;
      GetCursorPos(&point);
      WINDOWINFO winfo;
      GetWindowInfo(hwnd, &winfo);
      const RECT &rect = winfo.rcClient;
      int x = point.x - rect.left;
      int y = point.y - rect.top;
      int width = rect.right - rect.left;
      int height = rect.bottom - rect.top;

      _potential_drag_mode = consider_drag_start(x, y, width, height);

      switch (_potential_drag_mode) {
      case DM_left_margin:
      case DM_right_margin:
        SetCursor(_sizewe_cursor);
        return TRUE;

      case DM_guide_bar:
        SetCursor(_hand_cursor);
        return TRUE;

      default:
      case DM_none:
        break;
      }
    }
    break;

  case WM_LBUTTONDOWN:
    if (_potential_drag_mode != DM_none) {
      set_drag_mode(_potential_drag_mode);
      _drag_start_x = (int16_t)LOWORD(lparam);
      _drag_start_y = (int16_t)HIWORD(lparam);
      SetCapture(_window);
    }
    return 0;

  case WM_MOUSEMOVE:
    if (_drag_mode == DM_left_margin) {
      int16_t x = LOWORD(lparam);
      int new_left_margin = _left_margin + (x - _drag_start_x);
      _left_margin = std::max(new_left_margin, _pixel_scale * 2);
      _drag_start_x = x - (new_left_margin - _left_margin);
      InvalidateRect(hwnd, nullptr, TRUE);
      move_label_stack();
      return 0;

    } else if (_drag_mode == DM_right_margin) {
      int16_t x = LOWORD(lparam);
      _right_margin += (_drag_start_x - x);
      _drag_start_x = x;
      InvalidateRect(hwnd, nullptr, TRUE);
      return 0;
    }
    break;

  case WM_LBUTTONUP:
    set_drag_mode(DM_none);
    ReleaseCapture();
    break;

  case WM_PAINT:
    {
      PAINTSTRUCT ps;
      HDC hdc = BeginPaint(hwnd, &ps);

      // First, draw a frame around the graph.
      RECT rect;
      GetClientRect(hwnd, &rect);

      rect.left += _left_margin;
      rect.top += _top_margin;
      rect.right -= _right_margin;
      rect.bottom -= _bottom_margin;

      if (rect.right > rect.left && rect.bottom > rect.top) {
        int graph_xsize = rect.right - rect.left;
        int graph_ysize = rect.bottom - rect.top;
        if (_bitmap_dc == 0 ||
            graph_xsize != _bitmap_xsize ||
            graph_ysize != _bitmap_ysize) {
          // Oops, we need to change the bitmap (and graph) size.
          changed_graph_size(graph_xsize, graph_ysize);
          move_graph_window(rect.left, rect.top, graph_xsize, graph_ysize);
          force_redraw();
        }
      }

      additional_window_paint(hdc);

      EndPaint(hwnd, &ps);
      return 0;
    }

  case WM_DRAWITEM:
    if (wparam == IDC_GRAPH) {
      const DRAWITEMSTRUCT &dis = *(DRAWITEMSTRUCT *)lparam;

      // Repaint the graph by copying the backing pixmap in.
      BitBlt(dis.hDC, 0, 0,
             _bitmap_xsize, _bitmap_ysize,
             _bitmap_dc, 0, 0,
             SRCCOPY);

      additional_graph_window_paint(dis.hDC);
      return 0;
    }
    break;

  case WM_TIMER:
    {
      TrueClock *clock = TrueClock::get_global_ptr();
      double new_time = clock->get_short_time();
      if (!animate(new_time, new_time - _time)) {
        KillTimer(hwnd, 0x100);
        _timer_running = false;
      }
      _time = new_time;
    }
    return 0;

  case WM_NOTIFY:
    switch (((LPNMHDR)lparam)->code) {
    case TTN_GETDISPINFO:
      {
        NMTTDISPINFO &info = *(NMTTDISPINFO *)lparam;
        POINT point;
        if (GetCursorPos(&point) && ScreenToClient(_graph_window, &point)) {
          _tooltip_text = get_graph_tooltip(point.x, point.y);
          info.lpszText = (char *)_tooltip_text.c_str();
        }
      }
      return 0;
    }
    break;

  default:
    break;
  }

  return DefWindowProc(hwnd, msg, wparam, lparam);
}

/**
 *
 */
LONG WinStatsGraph::
graph_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  switch (msg) {
  case WM_DISPLAYCHANGE:
    setup_bitmap(_bitmap_xsize, _bitmap_ysize);
    force_redraw();
    break;

  case WM_NCHITTEST:
    // Necessary for mouse events to work; default returns HTTRANSPARENT
    return HTCLIENT;

  case WM_LBUTTONDOWN:
    // Vector any uncaught WM_LBUTTONDOWN into the main window, so we can drag
    // margins, etc.
    if (_potential_drag_mode != DM_none) {
      int16_t x = LOWORD(lparam) + _graph_left;
      int16_t y = HIWORD(lparam) + _graph_top;
      return window_proc(_window, msg, wparam, MAKELPARAM(x, y));
    }
    break;

  case WM_LBUTTONUP:
    set_drag_mode(DM_none);
    ReleaseCapture();
    break;

  default:
    break;
  }

  return DefSubclassProc(hwnd, msg, wparam, lparam);
}

/**
 * This is called during the servicing of WM_PAINT; it gives a derived class
 * opportunity to do some further painting into the window (the outer window,
 * not the graph window).
 */
void WinStatsGraph::
additional_window_paint(HDC hdc) {
}

/**
 * This is called during the servicing of WM_PAINT; it gives a derived class
 * opportunity to do some further painting into the graph window.
 */
void WinStatsGraph::
additional_graph_window_paint(HDC hdc) {
}

/**
 * Called when the mouse hovers over the graph, and should return the text that
 * should appear on the tooltip.
 */
std::string WinStatsGraph::
get_graph_tooltip(int mouse_x, int mouse_y) const {
  return std::string();
}

/**
 * Based on the mouse position within the window's client area, look for
 * draggable things the mouse might be hovering over and return the
 * appropriate DragMode enum or DM_none if nothing is indicated.
 */
WinStatsGraph::DragMode WinStatsGraph::
consider_drag_start(int mouse_x, int mouse_y, int width, int height) {
  if (mouse_x >= _left_margin - 2 && mouse_x <= _left_margin + 2) {
    return DM_left_margin;
  } else if (mouse_x >= width - _right_margin - 2 && mouse_x <= width - _right_margin + 2) {
    return DM_right_margin;
  }

  return DM_none;
}

/**
 * This should be called whenever the drag mode needs to change state.  It
 * provides hooks for a derived class to do something special.
 */
void WinStatsGraph::
set_drag_mode(WinStatsGraph::DragMode drag_mode) {
  _drag_mode = drag_mode;
}

/**
 * Repositions the graph child window within the parent window according to
 * the _margin variables.
 */
void WinStatsGraph::
move_graph_window(int graph_left, int graph_top, int graph_xsize, int graph_ysize) {
  if (_graph_window == 0) {
    create_graph_window();
  }

  _graph_left = graph_left;
  _graph_top = graph_top;

  SetWindowPos(_graph_window, 0,
               _graph_left, _graph_top,
               graph_xsize, graph_ysize,
               SWP_NOZORDER | SWP_SHOWWINDOW);

  if (graph_xsize != _bitmap_xsize || graph_ysize != _bitmap_ysize) {
    setup_bitmap(graph_xsize, graph_ysize);
  }
}

/**
 * Sets up a backing-store bitmap of the indicated size.
 */
void WinStatsGraph::
setup_bitmap(int xsize, int ysize) {
  release_bitmap();
  _bitmap_xsize = std::max(xsize, 0);
  _bitmap_ysize = std::max(ysize, 0);

  HDC hdc = GetDC(_graph_window);
  _bitmap_dc = CreateCompatibleDC(hdc);
  _bitmap = CreateCompatibleBitmap(hdc, _bitmap_xsize, _bitmap_ysize);
  SelectObject(_bitmap_dc, _bitmap);

  RECT rect = { 0, 0, _bitmap_xsize, _bitmap_ysize };
  FillRect(_bitmap_dc, &rect, (HBRUSH)GetStockObject(WHITE_BRUSH));

  ReleaseDC(_window, hdc);
}

/**
 * Frees the backing-store bitmap created by setup_bitmap().
 */
void WinStatsGraph::
release_bitmap() {
  if (_bitmap) {
    DeleteObject(_bitmap);
    _bitmap = 0;
  }
  if (_bitmap_dc) {
    DeleteDC(_bitmap_dc);
    _bitmap_dc = 0;
  }
}

/**
 * Creates the child window that actually holds the graph.
 */
void WinStatsGraph::
create_graph_window() {
  if (_graph_window) {
    return;
  }

  HINSTANCE application = GetModuleHandle(nullptr);

  DWORD window_style = WS_CHILD | WS_CLIPSIBLINGS |
                       SS_SUNKEN | SS_OWNERDRAW;

  _graph_window =
    CreateWindow(WC_STATIC, "", window_style, 0, 0, 0, 0,
                 _window, (HMENU)IDC_GRAPH, application, 0);
  if (!_graph_window) {
    nout << "Could not create graph window!\n";
    exit(1);
  }

  EnableWindow(_graph_window, TRUE);

  SetWindowSubclass(_graph_window, &static_graph_subclass_proc, 1234, (DWORD_PTR)this);

  // Create the tooltip window.  This will cause a TTN_GETDISPINFO message to
  // be sent to the window to acquire the tooltip text.
  _tooltip_window = CreateWindow(TOOLTIPS_CLASS, nullptr,
                                 WS_POPUP,
                                 CW_USEDEFAULT, CW_USEDEFAULT,
                                 CW_USEDEFAULT, CW_USEDEFAULT,
                                 _window, nullptr,
                                 application, nullptr);

  if (_tooltip_window != 0) {
    TOOLINFO info = { 0 };
    info.cbSize = sizeof(info);
    info.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
    info.hwnd = _window;
    info.uId = (UINT_PTR)_graph_window;
    info.lpszText = LPSTR_TEXTCALLBACK;
    SendMessage(_tooltip_window, TTM_ADDTOOL, 0, (LPARAM)&info);
  }
}

/**
 *
 */
LRESULT WINAPI WinStatsGraph::
static_graph_subclass_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam, UINT_PTR subclass, DWORD_PTR ref_data) {
  WinStatsGraph *self = (WinStatsGraph *)ref_data;
  if (self != nullptr && self->_graph_window == hwnd) {
    return self->graph_window_proc(hwnd, msg, wparam, lparam);
  } else {
    return DefWindowProc(hwnd, msg, wparam, lparam);
  }
}
