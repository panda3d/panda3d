// Filename: winStatsPianoRoll.cxx
// Created by:  drose (03Dec03)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "winStatsPianoRoll.h"
#include "winStatsMonitor.h"
#include "numeric_types.h"

static const int default_piano_roll_width = 400;
static const int default_piano_roll_height = 200;

bool WinStatsPianoRoll::_window_class_registered = false;
const char * const WinStatsPianoRoll::_window_class_name = "piano";

////////////////////////////////////////////////////////////////////
//     Function: WinStatsPianoRoll::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
WinStatsPianoRoll::
WinStatsPianoRoll(WinStatsMonitor *monitor, int thread_index) :
  PStatPianoRoll(monitor, thread_index, 
                 default_piano_roll_width,
                 default_piano_roll_height),
  WinStatsGraph(monitor, thread_index)
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

////////////////////////////////////////////////////////////////////
//     Function: WinStatsPianoRoll::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
WinStatsPianoRoll::
~WinStatsPianoRoll() {
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsPianoRoll::idle
//       Access: Public, Virtual
//  Description: Called as each frame's data is made available.  There
//               is no gurantee the frames will arrive in order, or
//               that all of them will arrive at all.  The monitor
//               should be prepared to accept frames received
//               out-of-order or missing.
////////////////////////////////////////////////////////////////////
void WinStatsPianoRoll::
new_data(int thread_index, int frame_number) {
  update();
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsPianoRoll::force_redraw
//       Access: Public, Virtual
//  Description: Called when it is necessary to redraw the entire graph.
////////////////////////////////////////////////////////////////////
void WinStatsPianoRoll::
force_redraw() {
  PStatPianoRoll::force_redraw();
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsPianoRoll::changed_graph_size
//       Access: Public, Virtual
//  Description: Called when the user has resized the window, forcing
//               a resize of the graph.
////////////////////////////////////////////////////////////////////
void WinStatsPianoRoll::
changed_graph_size(int graph_xsize, int graph_ysize) {
  PStatPianoRoll::changed_size(graph_xsize, graph_ysize);
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsPianoRoll::set_time_units
//       Access: Public, Virtual
//  Description: Called when the user selects a new time units from
//               the monitor pulldown menu, this should adjust the
//               units for the graph to the indicated mask if it is a
//               time-based graph.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: WinStatsPianoRoll::set_horizontal_scale
//       Access: Public
//  Description: Changes the amount of time the width of the
//               horizontal axis represents.  This may force a redraw.
////////////////////////////////////////////////////////////////////
void WinStatsPianoRoll::
set_horizontal_scale(float time_width) {
  PStatPianoRoll::set_horizontal_scale(time_width);

  RECT rect;
  GetClientRect(_window, &rect);
  rect.bottom = _top_margin;
  InvalidateRect(_window, &rect, TRUE);
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsPianoRoll::clear_region
//       Access: Protected
//  Description: Erases the chart area.
////////////////////////////////////////////////////////////////////
void WinStatsPianoRoll::
clear_region() {
  RECT rect = { 0, 0, get_xsize(), get_ysize() };
  FillRect(_bitmap_dc, &rect, (HBRUSH)GetStockObject(WHITE_BRUSH));
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsPianoRoll::begin_draw
//       Access: Protected, Virtual
//  Description: Erases the chart area in preparation for drawing a
//               bunch of bars.
////////////////////////////////////////////////////////////////////
void WinStatsPianoRoll::
begin_draw() {
  clear_region();

  // Draw in the guide bars.
  int num_guide_bars = get_num_guide_bars();
  for (int i = 0; i < num_guide_bars; i++) {
    const GuideBar &bar = get_guide_bar(i);
    int x = height_to_pixel(bar._height);

    if (x > 0 && x < get_xsize() - 1) {
      // Only draw it if it's not too close to either edge.
      if (bar._is_target) {
        SelectObject(_bitmap_dc, _light_pen);
      } else {
        SelectObject(_bitmap_dc, _dark_pen);
      }
      MoveToEx(_bitmap_dc, x, 0, NULL);
      LineTo(_bitmap_dc, x, get_ysize());
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsPianoRoll::draw_bar
//       Access: Protected, Virtual
//  Description: Draws a single bar on the chart.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: WinStatsPianoRoll::end_draw
//       Access: Protected, Virtual
//  Description: Called after all the bars have been drawn, this
//               triggers a refresh event to draw it to the window.
////////////////////////////////////////////////////////////////////
void WinStatsPianoRoll::
end_draw() {
  InvalidateRect(_graph_window, NULL, FALSE);
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsPianoRoll::idle
//       Access: Protected, Virtual
//  Description: Called at the end of the draw cycle.
////////////////////////////////////////////////////////////////////
void WinStatsPianoRoll::
idle() {
  if (_labels_changed) {
    update_labels();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsPianoRoll::window_proc
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
LONG WinStatsPianoRoll::
window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  /*
  switch (msg) {
  default:
    break;
  }
  */

  return WinStatsGraph::window_proc(hwnd, msg, wparam, lparam);
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsPianoRoll::graph_window_proc
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
LONG WinStatsPianoRoll::
graph_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  switch (msg) {
  case WM_LBUTTONDOWN:
    {
      _drag_mode = DM_scale;
      PN_int16 x = LOWORD(lparam);
      _drag_scale_start = pixel_to_height(x);
      SetCapture(_graph_window);
    }
    return 0;

  case WM_MOUSEMOVE: 
    if (_drag_mode == DM_scale) {
      PN_int16 x = LOWORD(lparam);
      float ratio = (float)x / (float)get_xsize();
      if (ratio > 0.0f) {
        set_horizontal_scale(_drag_scale_start / ratio);
      }
      return 0;
    }
    break;

  case WM_LBUTTONUP:
    if (_drag_mode == DM_scale) {
      _drag_mode = DM_none;
      ReleaseCapture();
      return 0;
    }
    break;

  default:
    break;
  }

  return WinStatsGraph::graph_window_proc(hwnd, msg, wparam, lparam);
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsPianoRoll::additional_window_paint
//       Access: Protected, Virtual
//  Description: This is called during the servicing of WM_PAINT; it
//               gives a derived class opportunity to do some further
//               painting into the window (the outer window, not the
//               graph window).
////////////////////////////////////////////////////////////////////
void WinStatsPianoRoll::
additional_window_paint(HDC hdc) {
  // Draw in the labels for the guide bars.
  HFONT hfnt = (HFONT)GetStockObject(ANSI_VAR_FONT); 
  SelectObject(hdc, hfnt);
  SetTextAlign(hdc, TA_LEFT | TA_BOTTOM);
  SetBkMode(hdc, TRANSPARENT);
  SetTextColor(hdc, RGB(0, 0, 0));

  int num_guide_bars = get_num_guide_bars();
  for (int i = 0; i < num_guide_bars; i++) {
    const GuideBar &bar = get_guide_bar(i);
    int x = height_to_pixel(bar._height);

    const string &label = bar._label;
    SIZE size;
    GetTextExtentPoint32(hdc, label.data(), label.length(), &size);
    x -= size.cx / 2;

    TextOut(hdc, x + _graph_left, _top_margin,
            label.data(), label.length()); 
  }
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsPianoRoll::update_labels
//       Access: Private
//  Description: Resets the list of labels.
////////////////////////////////////////////////////////////////////
void WinStatsPianoRoll::
update_labels() {
  _label_stack.clear_labels();
  for (int i = 0; i < get_num_labels(); i++) {
    int label_index = 
      _label_stack.add_label(WinStatsGraph::_monitor, 
                             WinStatsGraph::_thread_index,
                             get_label_collector(i), true);
  }
  _labels_changed = false;
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsPianoRoll::create_window
//       Access: Private
//  Description: Creates the window for this strip chart.
////////////////////////////////////////////////////////////////////
void WinStatsPianoRoll::
create_window() {
  if (_window) {
    return;
  }

  HINSTANCE application = GetModuleHandle(NULL);
  register_window_class(application);

  const PStatClientData *client_data = 
    WinStatsGraph::_monitor->get_client_data();
  string thread_name = client_data->get_thread_name(WinStatsGraph::_thread_index);
  string window_title = thread_name + " thread piano roll";


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
                 WinStatsGraph::_monitor->get_window(), NULL, application, 0);
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

////////////////////////////////////////////////////////////////////
//     Function: WinStatsPianoRoll::register_window_class
//       Access: Private, Static
//  Description: Registers the window class for the pianoRoll window, if
//               it has not already been registered.
////////////////////////////////////////////////////////////////////
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
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)COLOR_BACKGROUND;
  wc.lpszMenuName = NULL;
  wc.lpszClassName = _window_class_name;

  // Reserve space to associate the this pointer with the window.
  wc.cbWndExtra = sizeof(WinStatsPianoRoll *);
  
  if (!RegisterClass(&wc)) {
    nout << "Could not register PianoRoll window class!\n";
    exit(1);
  }

  _window_class_registered = true;
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsPianoRoll::static_window_proc
//       Access: Private, Static
//  Description: 
////////////////////////////////////////////////////////////////////
LONG WINAPI WinStatsPianoRoll::
static_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  WinStatsPianoRoll *self = (WinStatsPianoRoll *)GetWindowLongPtr(hwnd, 0);
  if (self != (WinStatsPianoRoll *)NULL && self->_window == hwnd) {
    return self->window_proc(hwnd, msg, wparam, lparam);
  } else {
    return DefWindowProc(hwnd, msg, wparam, lparam);
  }
}
