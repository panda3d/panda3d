/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file winStatsLabel.cxx
 * @author drose
 * @date 2004-01-07
 */

#include "winStatsLabel.h"
#include "winStatsMonitor.h"
#include "winStatsGraph.h"

int WinStatsLabel::_left_margin = 2;
int WinStatsLabel::_right_margin = 2;
int WinStatsLabel::_top_margin = 2;
int WinStatsLabel::_bottom_margin = 2;

bool WinStatsLabel::_window_class_registered = false;
const char * const WinStatsLabel::_window_class_name = "label";

/**
 *
 */
WinStatsLabel::
WinStatsLabel(WinStatsMonitor *monitor, WinStatsGraph *graph,
              int thread_index, int collector_index, bool use_fullname) :
  _monitor(monitor),
  _graph(graph),
  _thread_index(thread_index),
  _collector_index(collector_index)
{
  _window = 0;
  if (use_fullname) {
    _text = _monitor->get_client_data()->get_collector_fullname(_collector_index);
  } else {
    _text = _monitor->get_client_data()->get_collector_name(_collector_index);
  }

  LRGBColor rgb = _monitor->get_collector_color(_collector_index);
  int r = (int)(rgb[0] * 255.0f);
  int g = (int)(rgb[1] * 255.0f);
  int b = (int)(rgb[2] * 255.0f);
  _bg_color = RGB(r, g, b);
  _bg_brush = CreateSolidBrush(RGB(r, g, b));

  // Should our foreground be black or white?
  double bright =
    rgb[0] * 0.299 +
    rgb[1] * 0.587 +
    rgb[2] * 0.114;

  if (bright >= 0.5) {
    _fg_color = RGB(0, 0, 0);
    _highlight_brush = (HBRUSH)GetStockObject(BLACK_BRUSH);
  } else {
    _fg_color = RGB(255, 255, 255);
    _highlight_brush = (HBRUSH)GetStockObject(WHITE_BRUSH);
  }

  _x = 0;
  _y = 0;
  _width = 0;
  _height = 0;
  _ideal_width = 0;
  _highlight = false;
  _mouse_within = false;
}

/**
 *
 */
WinStatsLabel::
~WinStatsLabel() {
  if (_window) {
    DestroyWindow(_window);
    _window = 0;
  }
  DeleteObject(_bg_brush);
}

/**
 * Creates the actual window.
 */
void WinStatsLabel::
setup(HWND parent_window) {
  if (_window) {
    DestroyWindow(_window);
    _window = 0;
  }

  create_window(parent_window);

  HDC hdc = GetDC(_window);
  HFONT hfnt = (HFONT)GetStockObject(ANSI_VAR_FONT);
  SelectObject(hdc, hfnt);

  SIZE size;
  GetTextExtentPoint32(hdc, _text.data(), _text.length(), &size);
  _height = size.cy + _top_margin + _bottom_margin;
  _ideal_width = size.cx + _left_margin + _right_margin;

  ReleaseDC(_window, hdc);
}

/**
 * Sets the position of the label on its parent.  The position describes the
 * lower-left corner of the rectangle, not the upper-left.
 */
void WinStatsLabel::
set_pos(int x, int y, int width) {
  _x = x;
  _y = y;
  _width = width;
  SetWindowPos(_window, 0, x, y - _height, _width, _height,
               SWP_NOZORDER | SWP_SHOWWINDOW);
}

/**
 * Returns the x position of the label on its parent.
 */
int WinStatsLabel::
get_x() const {
  return _x;
}

/**
 * Returns the y position of the label on its parent.
 */
int WinStatsLabel::
get_y() const {
  return _y;
}

/**
 * Returns the width of the label as we requested it.
 */
int WinStatsLabel::
get_width() const {
  return _width;
}

/**
 * Returns the height of the label as we requested it.
 */
int WinStatsLabel::
get_height() const {
  return _height;
}

/**
 * Returns the width the label would really prefer to be.
 */
int WinStatsLabel::
get_ideal_width() const {
  return _ideal_width;
}

/**
 * Returns the collector this label represents.
 */
int WinStatsLabel::
get_collector_index() const {
  return _collector_index;
}

/**
 * Enables or disables the visual highlight for this label.
 */
void WinStatsLabel::
set_highlight(bool highlight) {
  if (_highlight != highlight) {
    _highlight = highlight;
    InvalidateRect(_window, nullptr, TRUE);
  }
}

/**
 * Returns true if the visual highlight for this label is enabled.
 */
bool WinStatsLabel::
get_highlight() const {
  return _highlight;
}

/**
 * Used internally to indicate whether the mouse is within the label's window.
 */
void WinStatsLabel::
set_mouse_within(bool mouse_within) {
  if (_mouse_within != mouse_within) {
    _mouse_within = mouse_within;
    InvalidateRect(_window, nullptr, TRUE);
  }
}

/**
 * Creates the window for this label.
 */
void WinStatsLabel::
create_window(HWND parent_window) {
  if (_window) {
    return;
  }

  HINSTANCE application = GetModuleHandle(nullptr);
  register_window_class(application);

  _window =
    CreateWindow(_window_class_name, _text.c_str(), WS_CHILD | WS_CLIPSIBLINGS,
                 0, 0, 0, 0,
                 parent_window, nullptr, application, 0);
  if (!_window) {
    nout << "Could not create Label window!\n";
    exit(1);
  }

  SetWindowLongPtr(_window, 0, (LONG_PTR)this);
}

/**
 * Registers the window class for the label window, if it has not already been
 * registered.
 */
void WinStatsLabel::
register_window_class(HINSTANCE application) {
  if (_window_class_registered) {
    return;
  }

  WNDCLASS wc;

  ZeroMemory(&wc, sizeof(WNDCLASS));
  wc.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = (WNDPROC)static_window_proc;
  wc.hInstance = application;
  wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
  wc.hbrBackground = nullptr;
  wc.lpszMenuName = nullptr;
  wc.lpszClassName = _window_class_name;

  // Reserve space to associate the this pointer with the window.
  wc.cbWndExtra = sizeof(WinStatsLabel *);

  if (!RegisterClass(&wc)) {
    nout << "Could not register Label window class!\n";
    exit(1);
  }

  _window_class_registered = true;
}

/**
 *
 */
LONG WINAPI WinStatsLabel::
static_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  WinStatsLabel *self = (WinStatsLabel *)GetWindowLongPtr(hwnd, 0);
  if (self != nullptr && self->_window == hwnd) {
    return self->window_proc(hwnd, msg, wparam, lparam);
  } else {
    return DefWindowProc(hwnd, msg, wparam, lparam);
  }
}

/**
 *
 */
LONG WinStatsLabel::
window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  switch (msg) {
  case WM_LBUTTONDBLCLK:
    _graph->clicked_label(_collector_index);
    return 0;

  case WM_MOUSEMOVE:
    {
      // When the mouse enters the label area, highlight the label.
      set_mouse_within(true);

      // Now we want to get a WM_MOUSELEAVE when the mouse leaves the label.
      TRACKMOUSEEVENT tme = {
        sizeof(TRACKMOUSEEVENT),
        TME_LEAVE,
        _window,
        0
      };
      TrackMouseEvent(&tme);
    }
    break;

  case WM_MOUSELEAVE:
    set_mouse_within(false);
    break;

  case WM_PAINT:
    {
      PAINTSTRUCT ps;
      HDC hdc = BeginPaint(hwnd, &ps);

      RECT rect = { 0, 0, _width, _height };
      FillRect(hdc, &rect, _bg_brush);

      if (_highlight || _mouse_within) {
        FrameRect(hdc, &rect, _highlight_brush);
      }

      HFONT hfnt = (HFONT)GetStockObject(ANSI_VAR_FONT);
      SelectObject(hdc, hfnt);
      SetTextAlign(hdc, TA_RIGHT | TA_TOP);

      SetBkColor(hdc, _bg_color);
      SetBkMode(hdc, OPAQUE);
      SetTextColor(hdc, _fg_color);

      TextOut(hdc, _width - _right_margin, _top_margin,
              _text.data(), _text.length());
      EndPaint(hwnd, &ps);
      return 0;
    }

  default:
    break;
  }

  return DefWindowProc(hwnd, msg, wparam, lparam);
}
