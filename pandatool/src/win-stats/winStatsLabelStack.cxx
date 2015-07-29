// Filename: winStatsLabelStack.cxx
// Created by:  drose (07Jan04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "winStatsLabelStack.h"
#include "winStatsLabel.h"
#include "pnotify.h"

bool WinStatsLabelStack::_window_class_registered = false;
const char * const WinStatsLabelStack::_window_class_name = "stack";

////////////////////////////////////////////////////////////////////
//     Function: WinStatsLabelStack::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
WinStatsLabelStack::
WinStatsLabelStack() {
  _x = 0;
  _y = 0;
  _width = 0;
  _height = 0;
  _ideal_width = 0;

  _highlight_label = -1;
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsLabelStack::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
WinStatsLabelStack::
~WinStatsLabelStack() {
  clear_labels();
  if (_window) {
    DestroyWindow(_window);
    _window = 0;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsLabelStack::setup
//       Access: Public
//  Description: Creates the actual window object.
////////////////////////////////////////////////////////////////////
void WinStatsLabelStack::
setup(HWND parent_window) {
  if (_window) {
    DestroyWindow(_window);
    _window = 0;
  }

  create_window(parent_window);

  _ideal_width = 0;
  Labels::iterator li;
  for (li = _labels.begin(); li != _labels.end(); ++li) {
    WinStatsLabel *label = (*li);
    label->setup(_window);
    _ideal_width = max(_ideal_width, label->get_ideal_width());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsLabelStack::is_setup
//       Access: Public
//  Description: Returns true if the label stack has been set up,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool WinStatsLabelStack::
is_setup() const {
  return (_window != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsLabelStack::set_pos
//       Access: Public
//  Description: Sets the position and size of the label stack on its parent.
////////////////////////////////////////////////////////////////////
void WinStatsLabelStack::
set_pos(int x, int y, int width, int height) {
  _x = x;
  _y = y;
  _width = width;
  _height = height;
  SetWindowPos(_window, 0, x, y, _width, _height, 
               SWP_NOZORDER | SWP_SHOWWINDOW);
  
  Labels::iterator li;
  int yp = height;
  for (li = _labels.begin(); li != _labels.end(); ++li) {
    WinStatsLabel *label = (*li);
    label->set_pos(0, yp, _width);
    yp -= label->get_height();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsLabelStack::get_x
//       Access: Public
//  Description: Returns the x position of the stack on its parent.
////////////////////////////////////////////////////////////////////
int WinStatsLabelStack::
get_x() const {
  return _x;
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsLabelStack::get_y
//       Access: Public
//  Description: Returns the y position of the stack on its parent.
////////////////////////////////////////////////////////////////////
int WinStatsLabelStack::
get_y() const {
  return _y;
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsLabelStack::get_width
//       Access: Public
//  Description: Returns the width of the stack as we requested it.
////////////////////////////////////////////////////////////////////
int WinStatsLabelStack::
get_width() const {
  return _width;
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsLabelStack::get_height
//       Access: Public
//  Description: Returns the height of the stack as we requested it.
////////////////////////////////////////////////////////////////////
int WinStatsLabelStack::
get_height() const {
  return _height;
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsLabelStack::get_ideal_width
//       Access: Public
//  Description: Returns the width the stack would really prefer to be.
////////////////////////////////////////////////////////////////////
int WinStatsLabelStack::
get_ideal_width() const {
  return _ideal_width;
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsLabelStack::get_label_y
//       Access: Public
//  Description: Returns the y position of the indicated label's bottom
//               edge, relative to the label stack's parent window.
////////////////////////////////////////////////////////////////////
int WinStatsLabelStack::
get_label_y(int label_index) const {
  nassertr(label_index >= 0 && label_index < (int)_labels.size(), 0);
  return _labels[label_index]->get_y() + get_y();
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsLabelStack::get_label_height
//       Access: Public
//  Description: Returns the height of the indicated label.
////////////////////////////////////////////////////////////////////
int WinStatsLabelStack::
get_label_height(int label_index) const {
  nassertr(label_index >= 0 && label_index < (int)_labels.size(), 0);
  return _labels[label_index]->get_height();
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsLabelStack::get_label_collector_index
//       Access: Public
//  Description: Returns the collector index associated with the
//               indicated label.
////////////////////////////////////////////////////////////////////
int WinStatsLabelStack::
get_label_collector_index(int label_index) const {
  nassertr(label_index >= 0 && label_index < (int)_labels.size(), -1);
  return _labels[label_index]->get_collector_index();
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsLabelStack::clear_labels
//       Access: Public
//  Description: Removes the set of labels and starts a new set.
////////////////////////////////////////////////////////////////////
void WinStatsLabelStack::
clear_labels() {
  Labels::iterator li;
  for (li = _labels.begin(); li != _labels.end(); ++li) {
    delete (*li);
  }
  _labels.clear();
  _ideal_width = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsLabelStack::add_label
//       Access: Public
//  Description: Adds a new label to the top of the stack; returns the
//               new label index.
////////////////////////////////////////////////////////////////////
int WinStatsLabelStack::
add_label(WinStatsMonitor *monitor, WinStatsGraph *graph,
          int thread_index, int collector_index, bool use_fullname) {
  int yp = _height;
  if (!_labels.empty()) {
    WinStatsLabel *top_label = _labels.back();
    yp = top_label->get_y() - top_label->get_height();
  }
  WinStatsLabel *label = 
    new WinStatsLabel(monitor, graph, thread_index, collector_index, use_fullname);
  if (_window) {
    label->setup(_window);
    label->set_pos(0, yp, _width);
  }
  _ideal_width = max(_ideal_width, label->get_ideal_width());

  int label_index = (int)_labels.size();
  _labels.push_back(label);

  return label_index;
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsLabelStack::get_num_labels
//       Access: Public
//  Description: Returns the number of labels in the stack.
////////////////////////////////////////////////////////////////////
int WinStatsLabelStack::
get_num_labels() const {
  return _labels.size();
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsLabelStack::highlight_label
//       Access: Public
//  Description: Draws a highlight around the label representing the
//               indicated collector, and removes the highlight from
//               any other label.  Specify -1 to remove the highlight
//               from all labels.
////////////////////////////////////////////////////////////////////
void WinStatsLabelStack::
highlight_label(int collector_index) {
  if (_highlight_label != collector_index) {
    _highlight_label = collector_index;
    Labels::iterator li;
    for (li = _labels.begin(); li != _labels.end(); ++li) {
      WinStatsLabel *label = (*li);
      label->set_highlight(label->get_collector_index() == _highlight_label);
    }
  }
}


////////////////////////////////////////////////////////////////////
//     Function: WinStatsLabelStack::create_window
//       Access: Private
//  Description: Creates the window for this stack.
////////////////////////////////////////////////////////////////////
void WinStatsLabelStack::
create_window(HWND parent_window) {
  if (_window) {
    return;
  }

  HINSTANCE application = GetModuleHandle(NULL);
  register_window_class(application);

  _window = 
    CreateWindow(_window_class_name, "label stack", WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                 0, 0, 0, 0,
                 parent_window, NULL, application, 0);
  if (!_window) {
    nout << "Could not create Label Stack window!\n";
    exit(1);
  }

  SetWindowLongPtr(_window, 0, (LONG_PTR)this);
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsLabelStack::register_window_class
//       Access: Private, Static
//  Description: Registers the window class for the label window, if
//               it has not already been registered.
////////////////////////////////////////////////////////////////////
void WinStatsLabelStack::
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
  wc.lpszMenuName = NULL;
  wc.lpszClassName = _window_class_name;

  // Reserve space to associate the this pointer with the window.
  wc.cbWndExtra = sizeof(WinStatsLabelStack *);
  
  if (!RegisterClass(&wc)) {
    nout << "Could not register Label Stack window class!\n";
    exit(1);
  }

  _window_class_registered = true;
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsLabelStack::static_window_proc
//       Access: Private, Static
//  Description: 
////////////////////////////////////////////////////////////////////
LONG WINAPI WinStatsLabelStack::
static_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  WinStatsLabelStack *self = (WinStatsLabelStack *)GetWindowLongPtr(hwnd, 0);
  if (self != (WinStatsLabelStack *)NULL && self->_window == hwnd) {
    return self->window_proc(hwnd, msg, wparam, lparam);
  } else {
    return DefWindowProc(hwnd, msg, wparam, lparam);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsLabelStack::window_proc
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
LONG WinStatsLabelStack::
window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  switch (msg) {
  case WM_PAINT:
    {
      PAINTSTRUCT ps;
      HDC hdc = BeginPaint(hwnd, &ps);

      RECT rect = { 0, 0, _width, _height };
      FillRect(hdc, &rect, (HBRUSH)COLOR_BACKGROUND);
      EndPaint(hwnd, &ps);
      return 0;
    }

  default:
    break;
  }

  return DefWindowProc(hwnd, msg, wparam, lparam);
}
