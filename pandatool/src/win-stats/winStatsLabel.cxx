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
#include "convert_srgb.h"

#include <commctrl.h>

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
              int thread_index, int collector_index, bool use_fullname,
              bool align_right) :
  _monitor(monitor),
  _graph(graph),
  _thread_index(thread_index),
  _collector_index(collector_index),
  _align_right(align_right),
  _window(0),
  _tooltip_window(0)
{
  update_text(use_fullname);
  update_color();

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
    if (_tooltip_window) {
      DestroyWindow(_tooltip_window);
      _tooltip_window = 0;
    }
    DestroyWindow(_window);
    _window = 0;
  }
  DeleteObject(_bg_brush);
  DeleteObject(_highlight_bg_brush);
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
  HFONT hfnt = _monitor->get_font();
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
  if (x != _x || y != _y || width != _width) {
    _x = x;
    _y = y;
    _width = width;
    SetWindowPos(_window, 0, x, y - _height, _width, _height,
                 SWP_NOZORDER | SWP_SHOWWINDOW);
  }
}

/**
 * Changes the Y attribute without updating the window.
 */
void WinStatsLabel::
set_y_noupdate(int y) {
  _y = y;
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
 * Updates the colors.
 */
void WinStatsLabel::
update_color() {
  if (_bg_brush != 0) {
    DeleteObject(_bg_brush);
  }
  if (_highlight_bg_brush != 0) {
    DeleteObject(_highlight_bg_brush);
  }

  LRGBColor rgb = _monitor->get_collector_color(_collector_index);
  int r = (int)encode_sRGB_uchar((float)rgb[0]);
  int g = (int)encode_sRGB_uchar((float)rgb[1]);
  int b = (int)encode_sRGB_uchar((float)rgb[2]);
  _bg_brush = CreateSolidBrush(RGB(r, g, b));

  // Calculate the color when it is highlighted.
  int hr = (int)encode_sRGB_uchar((float)rgb[0] * 0.75f);
  int hg = (int)encode_sRGB_uchar((float)rgb[1] * 0.75f);
  int hb = (int)encode_sRGB_uchar((float)rgb[2] * 0.75f);
  _highlight_bg_brush = CreateSolidBrush(RGB(hr, hg, hb));

  // Should our foreground be black or white?
  double bright =
    rgb[0] * 0.2126 +
    rgb[1] * 0.7152 +
    rgb[2] * 0.0722;

  if (bright >= 0.5) {
    _fg_color = RGB(0, 0, 0);
  } else {
    _fg_color = RGB(255, 255, 255);
  }
  if (bright * 0.75 >= 0.5) {
    _highlight_fg_color = RGB(0, 0, 0);
  } else {
    _highlight_fg_color = RGB(255, 255, 255);
  }

  if (_window) {
    InvalidateRect(_window, nullptr, TRUE);
  }
}

/**
 * Set to true if the full name of the collector should be shown.
 */
void WinStatsLabel::
update_text(bool use_fullname) {
  const PStatClientData *client_data = _monitor->get_client_data();
  _tooltip_text = client_data->get_collector_fullname(_collector_index);
  if (use_fullname) {
    _text = _tooltip_text;
  } else {
    _text = client_data->get_collector_name(_collector_index);
  }

  // Recalculate the dimensions.
  if (_window) {
    HDC hdc = GetDC(_window);
    HFONT hfnt = _monitor->get_font();
    SelectObject(hdc, hfnt);

    SIZE size;
    GetTextExtentPoint32(hdc, _text.data(), _text.length(), &size);
    _height = size.cy + _top_margin + _bottom_margin;
    _ideal_width = size.cx + _left_margin + _right_margin;
  }
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
    info.uId = (UINT_PTR)_window;
    info.lpszText = LPSTR_TEXTCALLBACK;
    SendMessage(_tooltip_window, TTM_ADDTOOL, 0, (LPARAM)&info);
  }
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
    _graph->on_click_label(_collector_index);
    return 0;

  case WM_CONTEXTMENU:
    _graph->on_popup_label(_collector_index);
    return 0;

  case WM_MOUSEMOVE:
    {
      // When the mouse enters the label area, highlight the label.
      if (!_mouse_within) {
        set_mouse_within(true);
        _graph->on_enter_label(_collector_index);
      }

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
    if (_mouse_within) {
      set_mouse_within(false);
      _graph->on_leave_label(_collector_index);
    }
    break;

  case WM_PAINT:
    {
      PAINTSTRUCT ps;
      HDC hdc = BeginPaint(hwnd, &ps);

      RECT rect = { 0, 0, _width, _height };
      FillRect(hdc, &rect, (_highlight || _mouse_within) ? _highlight_bg_brush : _bg_brush);

      HFONT hfnt = _monitor->get_font();
      SelectObject(hdc, hfnt);
      SetTextAlign(hdc, TA_LEFT | TA_TOP | TA_NOUPDATECP);

      SetBkMode(hdc, TRANSPARENT);
      SetTextColor(hdc, (_highlight || _mouse_within) ? _highlight_fg_color : _fg_color);

      if (_width > 8) {
        UINT format = DT_END_ELLIPSIS | DT_SINGLELINE;
        if (_align_right) {
          format |= DT_RIGHT;
        } else {
          format |= DT_LEFT;
        }

        RECT margins = { _left_margin, _top_margin, _width - _right_margin, _height - _bottom_margin };
        DrawText(hdc, _text.data(), _text.length(), &margins, format);
      }

      EndPaint(hwnd, &ps);
      return 0;
    }

  case WM_NOTIFY:
    switch (((LPNMHDR)lparam)->code) {
    case TTN_GETDISPINFO:
      {
        NMTTDISPINFO &info = *(NMTTDISPINFO *)lparam;
        _tooltip_text = _graph->get_label_tooltip(_collector_index);
        info.lpszText = (char *)_tooltip_text.c_str();
      }
      return 0;
    }
    break;

  default:
    break;
  }

  return DefWindowProc(hwnd, msg, wparam, lparam);
}
