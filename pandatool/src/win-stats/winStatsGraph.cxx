// Filename: winStatsGraph.cxx
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

#include "winStatsGraph.h"
#include "winStatsMonitor.h"
#include "winStatsLabelStack.h"

bool WinStatsGraph::_graph_window_class_registered = false;
const char * const WinStatsGraph::_graph_window_class_name = "graph";

////////////////////////////////////////////////////////////////////
//     Function: WinStatsGraph::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
WinStatsGraph::
WinStatsGraph(WinStatsMonitor *monitor) :
  _monitor(monitor)
{
  _window = 0;
  _graph_window = 0;
  _bitmap = 0;
  _bitmap_dc = 0;
  _bitmap_xsize = 0;
  _bitmap_ysize = 0;
  _left_margin = 96;
  _right_margin = 32;
  _top_margin = 16;
  _bottom_margin = 8;
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsGraph::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
WinStatsGraph::
~WinStatsGraph() {
  _monitor = (WinStatsMonitor *)NULL;
  release_bitmap();
  
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

////////////////////////////////////////////////////////////////////
//     Function: WinStatsGraph::new_collector
//       Access: Public, Virtual
//  Description: Called whenever a new Collector definition is
//               received from the client.
////////////////////////////////////////////////////////////////////
void WinStatsGraph::
new_collector(int new_collector) {
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsGraph::new_data
//       Access: Public, Virtual
//  Description: Called whenever new data arrives.
////////////////////////////////////////////////////////////////////
void WinStatsGraph::
new_data(int thread_index, int frame_number) {
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsGraph::force_redraw
//       Access: Public, Virtual
//  Description: Called when it is necessary to redraw the entire graph.
////////////////////////////////////////////////////////////////////
void WinStatsGraph::
force_redraw() {
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsGraph::changed_graph_size
//       Access: Public, Virtual
//  Description: Called when the user has resized the window, forcing
//               a resize of the graph.
////////////////////////////////////////////////////////////////////
void WinStatsGraph::
changed_graph_size(int graph_xsize, int graph_ysize) {
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsGraph::close
//       Access: Protected
//  Description: Should be called when the user closes the associated
//               window.  This tells the monitor to remove the graph.
////////////////////////////////////////////////////////////////////
void WinStatsGraph::
close() {
  WinStatsMonitor *monitor = _monitor;
  _monitor = (WinStatsMonitor *)NULL;
  if (monitor != (WinStatsMonitor *)NULL) {
    monitor->remove_graph(this);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsGraph::setup_label_stack
//       Access: Protected
//  Description: Sets up the label stack on the left edge of the
//               frame.
////////////////////////////////////////////////////////////////////
void WinStatsGraph::
setup_label_stack() {
  _label_stack.setup(_window);
  move_label_stack();
  /*
  if (_label_stack()->get_ideal_width() > _label_stack->get_width()) {
    _left_margin = _label_stack->get_ideal_width() + 16;
    move_label_stack();
  }
  */
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsGraph::move_label_stack
//       Access: Protected
//  Description: Repositions the label stack if its coordinates or
//               size have changed.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: WinStatsGraph::get_collector_brush
//       Access: Protected
//  Description: Returns a brush suitable for drawing in the indicated
//               collector's color.
////////////////////////////////////////////////////////////////////
HBRUSH WinStatsGraph::
get_collector_brush(int collector_index) {
  Brushes::iterator bi;
  bi = _brushes.find(collector_index);
  if (bi != _brushes.end()) {
    return (*bi).second;
  }

  // Ask the monitor what color this guy should be.
  RGBColorf rgb = _monitor->get_collector_color(collector_index);
  int r = (int)(rgb[0] * 255.0f);
  int g = (int)(rgb[1] * 255.0f);
  int b = (int)(rgb[2] * 255.0f);
  HBRUSH brush = CreateSolidBrush(RGB(r, g, b));

  _brushes[collector_index] = brush;
  return brush;
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsGraph::window_proc
//       Access: Protected
//  Description: This window_proc should be called up to by the
//               derived classes for any messages that are not
//               specifically handled by the derived class.
////////////////////////////////////////////////////////////////////
LONG WinStatsGraph::
window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  switch (msg) {
  case WM_DESTROY:
    close();
    break;

  case WM_SIZE:
    move_label_stack();
    InvalidateRect(hwnd, NULL, FALSE);
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

      EndPaint(hwnd, &ps);
      return 0;
    }

  default:
    break;
  }

  return DefWindowProc(hwnd, msg, wparam, lparam);
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsGraph::setup_bitmap
//       Access: Private
//  Description: Sets up a backing-store bitmap of the indicated size.
////////////////////////////////////////////////////////////////////
void WinStatsGraph::
setup_bitmap(int xsize, int ysize) {
  release_bitmap();
  _bitmap_xsize = max(xsize, 0);
  _bitmap_ysize = max(ysize, 0);

  HDC hdc = GetDC(_graph_window);
  _bitmap_dc = CreateCompatibleDC(hdc);
  _bitmap = CreateCompatibleBitmap(hdc, _bitmap_xsize, _bitmap_ysize);
  SelectObject(_bitmap_dc, _bitmap);
  ReleaseDC(_window, hdc);
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsGraph::release_bitmap
//       Access: Private
//  Description: Frees the backing-store bitmap created by
//               setup_bitmap().
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: WinStatsGraph::move_graph_window
//       Access: Private
//  Description: Repositions the graph child window within the parent
//               window according to the _margin variables.
////////////////////////////////////////////////////////////////////
void WinStatsGraph::
move_graph_window(int graph_left, int graph_top, int graph_xsize, int graph_ysize) {
  if (_graph_window == 0) {
    create_graph_window();
  }

  SetWindowPos(_graph_window, 0, 
               graph_left, graph_top,
               graph_xsize, graph_ysize,
               SWP_NOZORDER | SWP_SHOWWINDOW);

  if (graph_xsize != _bitmap_xsize || graph_ysize != _bitmap_ysize) {
    setup_bitmap(graph_xsize, graph_ysize);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsGraph::create_graph_window
//       Access: Private
//  Description: Creates the child window that actually holds the graph.
////////////////////////////////////////////////////////////////////
void WinStatsGraph::
create_graph_window() {
  if (_graph_window) {
    return;
  }

  HINSTANCE application = GetModuleHandle(NULL);
  register_graph_window_class(application);

  string window_title = "graph";
  DWORD window_style = WS_CHILD;

  _graph_window = 
    CreateWindow(_graph_window_class_name, window_title.c_str(), window_style,
                 0, 0, 0, 0,
                 _window, NULL, application, 0);
  if (!_graph_window) {
    nout << "Could not create graph window!\n";
    exit(1);
  }

  SetWindowLongPtr(_graph_window, 0, (LONG_PTR)this);
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsGraph::register_graph_window_class
//       Access: Private, Static
//  Description: Registers the window class for the stripChart window, if
//               it has not already been registered.
////////////////////////////////////////////////////////////////////
void WinStatsGraph::
register_graph_window_class(HINSTANCE application) {
  if (_graph_window_class_registered) {
    return;
  }

  WNDCLASS wc;

  ZeroMemory(&wc, sizeof(WNDCLASS));
  wc.style = 0;
  wc.lpfnWndProc = (WNDPROC)static_graph_window_proc;
  wc.hInstance = application;
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = NULL;
  wc.lpszMenuName = NULL;
  wc.lpszClassName = _graph_window_class_name;

  // Reserve space to associate the this pointer with the window.
  wc.cbWndExtra = sizeof(WinStatsGraph *);
  
  if (!RegisterClass(&wc)) {
    nout << "Could not register graph window class!\n";
    exit(1);
  }

  _graph_window_class_registered = true;
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsGraph::static_graph_window_proc
//       Access: Private, Static
//  Description: 
////////////////////////////////////////////////////////////////////
LONG WINAPI WinStatsGraph::
static_graph_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  WinStatsGraph *self = (WinStatsGraph *)GetWindowLongPtr(hwnd, 0);
  if (self != (WinStatsGraph *)NULL && self->_graph_window == hwnd) {
    return self->graph_window_proc(hwnd, msg, wparam, lparam);
  } else {
    return DefWindowProc(hwnd, msg, wparam, lparam);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsGraph::graph_window_proc
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
LONG WinStatsGraph::
graph_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  switch (msg) {
  case WM_DISPLAYCHANGE:
    setup_bitmap(_bitmap_xsize, _bitmap_ysize);
    force_redraw();
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

      EndPaint(hwnd, &ps);
      return 0;
    }

  default:
    break;
  }

  return DefWindowProc(hwnd, msg, wparam, lparam);
}
