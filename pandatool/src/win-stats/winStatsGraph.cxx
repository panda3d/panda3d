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

bool WinStatsGraph::_graph_window_class_registered = false;
const char * const WinStatsGraph::_graph_window_class_name = "graph";

DWORD WinStatsGraph::graph_window_style =
WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_OVERLAPPEDWINDOW | WS_VISIBLE;

/**
 *
 */
WinStatsGraph::
WinStatsGraph(WinStatsMonitor *monitor) :
  _monitor(monitor)
{
  _window = 0;
  _graph_window = 0;
  _sizewe_cursor = LoadCursor(nullptr, IDC_SIZEWE);
  _hand_cursor = LoadCursor(nullptr, IDC_HAND);
  _bitmap = 0;
  _bitmap_dc = 0;

  _graph_left = 0;
  _graph_top = 0;
  _bitmap_xsize = 0;
  _bitmap_ysize = 0;

  _dark_color = RGB(51, 51, 51);
  _light_color = RGB(154, 154, 154);
  _user_guide_bar_color = RGB(130, 150, 255);
  _dark_pen = CreatePen(PS_SOLID, 1, _dark_color);
  _light_pen = CreatePen(PS_SOLID, 1, _light_color);
  _user_guide_bar_pen = CreatePen(PS_DASH, 1, _user_guide_bar_color);

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

  Brushes::iterator bi;
  for (bi = _brushes.begin(); bi != _brushes.end(); ++bi) {
    HBRUSH brush = (*bi).second;
    DeleteObject(brush);
  }

  if (_graph_window) {
    DestroyWindow(_graph_window);
    _graph_window = 0;
  }

  if (_window) {
    DestroyWindow(_window);
    _window = 0;
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
 * Called when it is necessary to redraw the entire graph.
 */
void WinStatsGraph::
force_redraw() {
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
clicked_label(int collector_index) {
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

    rect.left += 8;
    rect.right = _left_margin - 8;
    rect.bottom -= _bottom_margin;

    _label_stack.set_pos(rect.left, rect.top,
                         rect.right - rect.left, rect.bottom - rect.top);
  }
}

/**
 * Returns a brush suitable for drawing in the indicated collector's color.
 */
HBRUSH WinStatsGraph::
get_collector_brush(int collector_index) {
  Brushes::iterator bi;
  bi = _brushes.find(collector_index);
  if (bi != _brushes.end()) {
    return (*bi).second;
  }

  // Ask the monitor what color this guy should be.
  LRGBColor rgb = _monitor->get_collector_color(collector_index);
  int r = (int)(rgb[0] * 255.0f);
  int g = (int)(rgb[1] * 255.0f);
  int b = (int)(rgb[2] * 255.0f);
  HBRUSH brush = CreateSolidBrush(RGB(r, g, b));

  _brushes[collector_index] = brush;
  return brush;
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
      _left_margin += (x - _drag_start_x);
      _drag_start_x = x;
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
        DrawEdge(hdc, &rect, EDGE_SUNKEN, BF_RECT | BF_ADJUST);

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

  case WM_PAINT:
    {
      // Repaint the graph by copying the backing pixmap in.
      PAINTSTRUCT ps;
      HDC hdc = BeginPaint(hwnd, &ps);

      BitBlt(hdc, 0, 0,
             _bitmap_xsize, _bitmap_ysize,
             _bitmap_dc, 0, 0,
             SRCCOPY);

      additional_graph_window_paint(hdc);

      EndPaint(hwnd, &ps);
      return 0;
    }

  default:
    break;
  }

  return DefWindowProc(hwnd, msg, wparam, lparam);
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
  register_graph_window_class(application);

  std::string window_title = "graph";
  DWORD window_style = WS_CHILD | WS_CLIPSIBLINGS;

  _graph_window =
    CreateWindow(_graph_window_class_name, window_title.c_str(), window_style,
                 0, 0, 0, 0,
                 _window, nullptr, application, 0);
  if (!_graph_window) {
    nout << "Could not create graph window!\n";
    exit(1);
  }

  SetWindowLongPtr(_graph_window, 0, (LONG_PTR)this);
}

/**
 * Registers the window class for the stripChart window, if it has not already
 * been registered.
 */
void WinStatsGraph::
register_graph_window_class(HINSTANCE application) {
  if (_graph_window_class_registered) {
    return;
  }

  WNDCLASS wc;

  ZeroMemory(&wc, sizeof(WNDCLASS));
  wc.style = CS_DBLCLKS;
  wc.lpfnWndProc = (WNDPROC)static_graph_window_proc;
  wc.hInstance = application;
  wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
  wc.hbrBackground = nullptr;
  wc.lpszMenuName = nullptr;
  wc.lpszClassName = _graph_window_class_name;

  // Reserve space to associate the this pointer with the window.
  wc.cbWndExtra = sizeof(WinStatsGraph *);

  if (!RegisterClass(&wc)) {
    nout << "Could not register graph window class!\n";
    exit(1);
  }

  _graph_window_class_registered = true;
}

/**
 *
 */
LONG WINAPI WinStatsGraph::
static_graph_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  WinStatsGraph *self = (WinStatsGraph *)GetWindowLongPtr(hwnd, 0);
  if (self != nullptr && self->_graph_window == hwnd) {
    return self->graph_window_proc(hwnd, msg, wparam, lparam);
  } else {
    return DefWindowProc(hwnd, msg, wparam, lparam);
  }
}
