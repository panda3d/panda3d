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
//     Function: WinStatsGraph::setup_bitmap
//       Access: Protected
//  Description: Sets up a backing-store bitmap of the indicated size.
////////////////////////////////////////////////////////////////////
void WinStatsGraph::
setup_bitmap(int xsize, int ysize) {
  release_bitmap();
  _bitmap_xsize = max(xsize, 0);
  _bitmap_ysize = max(ysize, 0);

  _frame_rect.left = _left_margin;
  _frame_rect.top = _top_margin;
  _frame_rect.right = _left_margin + _bitmap_xsize;
  _frame_rect.bottom = _bottom_margin + _bitmap_ysize;

  HDC hdc = GetDC(_window);
  _bitmap_dc = CreateCompatibleDC(hdc);
  _bitmap = CreateCompatibleBitmap(hdc, _bitmap_xsize, _bitmap_ysize);
  SelectObject(_bitmap_dc, _bitmap);
  ReleaseDC(_window, hdc);
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsGraph::release_bitmap
//       Access: Protected
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
//     Function: WinStatsGraph::setup_label_stack
//       Access: Protected
//  Description: Sets up the label stack on the left edge of the
//               frame.
////////////////////////////////////////////////////////////////////
void WinStatsGraph::
setup_label_stack() {
  _label_stack.setup(_window, 8, 8, _left_margin - 16, _top_margin + _bitmap_ysize + _bottom_margin - 16);
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
  _label_stack.set_pos(8, 8, _left_margin - 16, _top_margin + _bitmap_ysize + _bottom_margin - 16);
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
//     Function: WinStatsGraph::draw_graph
//       Access: Protected
//  Description: Draws the graph into the window by blitting the
//               backing-store bitmap in, along with a suitable frame.
////////////////////////////////////////////////////////////////////
void WinStatsGraph::
draw_graph(HDC hdc) {
  if (_bitmap_xsize == 0 || _bitmap_ysize == 0) {
    // Never mind: nothing to draw.
    return;
  }

  // First, draw a frame around the graph.

  // Windows doesn't seem to have an API to ask how big the outer
  // frame will be before we draw it, only a way to draw the outer
  // frame and return the size of the inner frame afterwards.

  // So we have to make our best guess about the correct size of the
  // outer frame before we draw it, then examine the size of the
  // resulting inner frame.  If it didn't come out to the correct size
  // (that is, exactly large enough to frame our graph), we expand the
  // outer frame by the difference, and redraw it.

  RECT rect = _frame_rect;
  DrawEdge(hdc, &rect, EDGE_SUNKEN, BF_RECT | BF_ADJUST);

  if (rect.left != _left_margin ||
      rect.top != _top_margin ||
      rect.right != _left_margin + _bitmap_xsize ||
      rect.bottom != _top_margin + _bitmap_ysize) {
    _frame_rect.left = _left_margin - (rect.left - _frame_rect.left);
    _frame_rect.top = _top_margin - (rect.top - _frame_rect.top);
    _frame_rect.right = _left_margin + _bitmap_xsize + (_frame_rect.right - rect.right);
    _frame_rect.bottom = _top_margin + _bitmap_ysize + (_frame_rect.bottom - rect.bottom);

    DrawEdge(hdc, &_frame_rect, EDGE_SUNKEN, BF_RECT);
  }

  // Now fill in the graph.
  BitBlt(hdc, _left_margin, _top_margin, 
         _bitmap_xsize, _bitmap_ysize,
         _bitmap_dc, 0, 0,
         SRCCOPY);
}
