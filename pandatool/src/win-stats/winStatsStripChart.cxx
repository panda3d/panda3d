// Filename: winStatsStripChart.cxx
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

#include "winStatsStripChart.h"
#include "winStatsMonitor.h"

static const int default_strip_chart_width = 400;
static const int default_strip_chart_height = 100;

bool WinStatsStripChart::_window_class_registered = false;
const char * const WinStatsStripChart::_window_class_name = "strip";

////////////////////////////////////////////////////////////////////
//     Function: WinStatsStripChart::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
WinStatsStripChart::
WinStatsStripChart(WinStatsMonitor *monitor, PStatView &view,
                   int collector_index) :
  PStatStripChart(monitor, view, collector_index, 
                  default_strip_chart_width,
                  default_strip_chart_height),
  WinStatsGraph(monitor)
{
  cerr << "Constructing strip chart " << (void *)this << "\n";
  _brush_origin = 0;

  create_window();
  clear_region();
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsStripChart::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
WinStatsStripChart::
~WinStatsStripChart() {
  cerr << "Destructing strip chart " << (void *)this << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsStripChart::new_collector
//       Access: Public, Virtual
//  Description: Called whenever a new Collector definition is
//               received from the client.
////////////////////////////////////////////////////////////////////
void WinStatsStripChart::
new_collector(int collector_index) {
  if (is_title_unknown()) {
    string window_title = get_title_text();
    if (!is_title_unknown()) {
      SetWindowText(_window, window_title.c_str());
    }
  }

  WinStatsGraph::new_collector(collector_index);
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsStripChart::idle
//       Access: Public, Virtual
//  Description: Called as each frame's data is made available.  There
//               is no gurantee the frames will arrive in order, or
//               that all of them will arrive at all.  The monitor
//               should be prepared to accept frames received
//               out-of-order or missing.
////////////////////////////////////////////////////////////////////
void WinStatsStripChart::
new_data(int thread_index, int frame_number) {
  update();
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsStripChart::force_redraw
//       Access: Public, Virtual
//  Description: Called when it is necessary to redraw the entire graph.
////////////////////////////////////////////////////////////////////
void WinStatsStripChart::
force_redraw() {
  PStatStripChart::force_redraw();
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsStripChart::changed_graph_size
//       Access: Public, Virtual
//  Description: Called when the user has resized the window, forcing
//               a resize of the graph.
////////////////////////////////////////////////////////////////////
void WinStatsStripChart::
changed_graph_size(int graph_xsize, int graph_ysize) {
  PStatStripChart::changed_size(graph_xsize, graph_ysize);
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsStripChart::update_labels
//       Access: Protected, Virtual
//  Description: Resets the list of labels.
////////////////////////////////////////////////////////////////////
void WinStatsStripChart::
update_labels() {
  PStatStripChart::update_labels();

  _label_stack.clear_labels();
  for (int i = 0; i < get_num_labels(); i++) {
    _label_stack.add_label(WinStatsGraph::_monitor, get_label_collector(i));
  }
  _labels_changed = false;
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsStripChart::clear_region
//       Access: Protected, Virtual
//  Description: Erases the chart area.
////////////////////////////////////////////////////////////////////
void WinStatsStripChart::
clear_region() {
  RECT rect = { 0, 0, get_xsize(), get_ysize() };
  FillRect(_bitmap_dc, &rect, (HBRUSH)GetStockObject(WHITE_BRUSH));
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsStripChart::copy_region
//       Access: Protected, Virtual
//  Description: Should be overridden by the user class to copy a
//               region of the chart from one part of the chart to
//               another.  This is used to implement scrolling.
////////////////////////////////////////////////////////////////////
void WinStatsStripChart::
copy_region(int start_x, int end_x, int dest_x) {
  BitBlt(_bitmap_dc, dest_x, 0, 
         end_x - start_x, get_ysize(),
         _bitmap_dc, start_x, 0,
         SRCCOPY);

  // Also shift the brush origin over, so we still get proper
  // dithering.
  _brush_origin += (dest_x - start_x);
  SetBrushOrgEx(_bitmap_dc, _brush_origin, 0, NULL);

  RECT rect = { 
    dest_x, 0, dest_x + end_x - start_x, get_ysize() 
  };
  InvalidateRect(_graph_window, &rect, FALSE);
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsStripChart::draw_slice
//       Access: Protected, Virtual
//  Description: Draws a single vertical slice of the strip chart, at
//               the given pixel position, and corresponding to the
//               indicated level data.
////////////////////////////////////////////////////////////////////
void WinStatsStripChart::
draw_slice(int x, int w, int frame_number) {
  const FrameData &frame = get_frame_data(frame_number);

  // Start by clearing the band first.
  RECT rect = { x, 0, x + w, get_ysize() };
  FillRect(_bitmap_dc, &rect, (HBRUSH)GetStockObject(WHITE_BRUSH));

  float overall_time = 0.0;
  int y = get_ysize();

  FrameData::const_iterator fi;
  for (fi = frame.begin(); fi != frame.end(); ++fi) {
    const ColorData &cd = (*fi);
    overall_time += cd._net_value;
    HBRUSH brush = get_collector_brush(cd._collector_index);

    if (overall_time > get_vertical_scale()) {
      // Off the top.  Go ahead and clamp it by hand, in case it's so
      // far off the top we'd overflow the 16-bit pixel value.
      rect.top = 0;
      rect.bottom = y;
      FillRect(_bitmap_dc, &rect, brush);
      // And we can consider ourselves done now.
      return;
    }

    int top_y = height_to_pixel(overall_time);
    rect.top = top_y;
    rect.bottom = y;
    FillRect(_bitmap_dc, &rect, brush);
    y = top_y;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsStripChart::draw_empty
//       Access: Protected, Virtual
//  Description: Draws a single vertical slice of background color.
////////////////////////////////////////////////////////////////////
void WinStatsStripChart::
draw_empty(int x, int w) {
  RECT rect = { x, 0, x + w, get_ysize() };
  FillRect(_bitmap_dc, &rect, (HBRUSH)GetStockObject(WHITE_BRUSH));
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsStripChart::draw_cursor
//       Access: Protected, Virtual
//  Description: Draws a single vertical slice of foreground color.
////////////////////////////////////////////////////////////////////
void WinStatsStripChart::
draw_cursor(int x) {
  RECT rect = { x, 0, x + 1, get_ysize() };
  FillRect(_bitmap_dc, &rect, (HBRUSH)GetStockObject(BLACK_BRUSH));
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsStripChart::end_draw
//       Access: Protected, Virtual
//  Description: Should be overridden by the user class.  This hook
//               will be called after drawing a series of color bars
//               in the strip chart; it gives the pixel range that
//               was just redrawn.
////////////////////////////////////////////////////////////////////
void WinStatsStripChart::
end_draw(int from_x, int to_x) {
  RECT rect = { 
    from_x, 0, to_x, get_ysize() 
  };
  InvalidateRect(_graph_window, &rect, FALSE);
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsStripChart::create_window
//       Access: Private
//  Description: Creates the window for this strip chart.
////////////////////////////////////////////////////////////////////
void WinStatsStripChart::
create_window() {
  if (_window) {
    return;
  }

  HINSTANCE application = GetModuleHandle(NULL);
  register_window_class(application);

  string window_title = get_title_text();
  DWORD window_style = WS_CHILD | WS_CLIPCHILDREN | WS_OVERLAPPEDWINDOW | WS_VISIBLE;

  RECT win_rect = { 
    0, 0,
    _left_margin + get_xsize() + _right_margin, 
    _top_margin + get_ysize() + _bottom_margin
  };  
  
  // compute window size based on desired client area size
  AdjustWindowRect(&win_rect, window_style, FALSE);

  _window = 
    CreateWindow(_window_class_name, window_title.c_str(), window_style,
                 CW_USEDEFAULT, 0, 
                 win_rect.right - win_rect.left,
                 win_rect.bottom - win_rect.top,
                 WinStatsGraph::_monitor->get_window(), NULL, application, 0);
  if (!_window) {
    nout << "Could not create StripChart window!\n";
    exit(1);
  }

  SetWindowLongPtr(_window, 0, (LONG_PTR)this);
  setup_label_stack();
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsStripChart::register_window_class
//       Access: Private, Static
//  Description: Registers the window class for the stripChart window, if
//               it has not already been registered.
////////////////////////////////////////////////////////////////////
void WinStatsStripChart::
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
  wc.cbWndExtra = sizeof(WinStatsStripChart *);
  
  if (!RegisterClass(&wc)) {
    nout << "Could not register StripChart window class!\n";
    exit(1);
  }

  _window_class_registered = true;
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsStripChart::static_window_proc
//       Access: Private, Static
//  Description: 
////////////////////////////////////////////////////////////////////
LONG WINAPI WinStatsStripChart::
static_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  WinStatsStripChart *self = (WinStatsStripChart *)GetWindowLongPtr(hwnd, 0);
  if (self != (WinStatsStripChart *)NULL && self->_window == hwnd) {
    return self->window_proc(hwnd, msg, wparam, lparam);
  } else {
    return DefWindowProc(hwnd, msg, wparam, lparam);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsStripChart::window_proc
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
LONG WinStatsStripChart::
window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  /*
  switch (msg) {
  default:
    break;
  }
  */

  return WinStatsGraph::window_proc(hwnd, msg, wparam, lparam);
}
