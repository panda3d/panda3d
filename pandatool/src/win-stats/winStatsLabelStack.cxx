/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file winStatsLabelStack.cxx
 * @author drose
 * @date 2004-01-07
 */

#include "winStatsLabelStack.h"
#include "winStatsLabel.h"
#include "pnotify.h"

bool WinStatsLabelStack::_window_class_registered = false;
const char * const WinStatsLabelStack::_window_class_name = "stack";

/**
 *
 */
WinStatsLabelStack::
WinStatsLabelStack() {
  _x = 0;
  _y = 0;
  _width = 0;
  _height = 0;
  _ideal_width = 0;

  _highlight_label = -1;
}

/**
 *
 */
WinStatsLabelStack::
~WinStatsLabelStack() {
  clear_labels();
  if (_window) {
    DestroyWindow(_window);
    _window = 0;
  }
}

/**
 * Creates the actual window object.
 */
void WinStatsLabelStack::
setup(HWND parent_window) {
  if (_window) {
    DestroyWindow(_window);
    _window = 0;
  }

  create_window(parent_window);

  _ideal_width = 0;
  for (WinStatsLabel *label : _labels) {
    label->setup(_window);
    _ideal_width = std::max(_ideal_width, label->get_ideal_width());
  }
}

/**
 * Returns true if the label stack has been set up, false otherwise.
 */
bool WinStatsLabelStack::
is_setup() const {
  return (_window != 0);
}

/**
 * Sets the position and size of the label stack on its parent.
 */
void WinStatsLabelStack::
set_pos(int x, int y, int width, int height, int top_margin, int bottom_margin) {
  _x = x;
  _y = y;
  _width = width;
  _height = height;
  _top_margin = top_margin;
  _bottom_margin = bottom_margin;
  SetWindowPos(_window, 0, x, y, _width, _height,
               SWP_NOZORDER | SWP_SHOWWINDOW);

  recalculate_label_positions();
}

/**
 * Returns the x position of the stack on its parent.
 */
int WinStatsLabelStack::
get_x() const {
  return _x;
}

/**
 * Returns the y position of the stack on its parent.
 */
int WinStatsLabelStack::
get_y() const {
  return _y;
}

/**
 * Returns the width of the stack as we requested it.
 */
int WinStatsLabelStack::
get_width() const {
  return _width;
}

/**
 * Returns the height of the stack as we requested it.
 */
int WinStatsLabelStack::
get_height() const {
  return _height;
}

/**
 * Returns the width the stack would really prefer to be.
 */
int WinStatsLabelStack::
get_ideal_width() const {
  return _ideal_width;
}

/**
 * Returns the y position of the indicated label's bottom edge, relative to
 * the label stack's parent window.
 */
int WinStatsLabelStack::
get_label_y(int label_index) const {
  nassertr(label_index >= 0 && label_index < (int)_labels.size(), 0);
  return _labels[label_index]->get_y() + get_y();
}

/**
 * Returns the height of the indicated label.
 */
int WinStatsLabelStack::
get_label_height(int label_index) const {
  nassertr(label_index >= 0 && label_index < (int)_labels.size(), 0);
  return _labels[label_index]->get_height();
}

/**
 * Returns the collector index associated with the indicated label.
 */
int WinStatsLabelStack::
get_label_collector_index(int label_index) const {
  nassertr(label_index >= 0 && label_index < (int)_labels.size(), -1);
  return _labels[label_index]->get_collector_index();
}

/**
 * Removes the set of labels and starts a new set.
 */
void WinStatsLabelStack::
clear_labels() {
  for (WinStatsLabel *label : _labels) {
    delete label;
  }
  _labels.clear();
  _ideal_width = 0;
  _scroll = 0;
}

/**
 * Adds a new label to the top of the stack; returns the new label index.
 */
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
    label->set_pos(0, yp - _scroll, _width);
  }
  _ideal_width = std::max(_ideal_width, label->get_ideal_width());

  int label_index = (int)_labels.size();
  _labels.push_back(label);

  recalculate_label_positions();

  return label_index;
}

/**
 * Replaces the labels with the given collector indices.
 */
void WinStatsLabelStack::
replace_labels(WinStatsMonitor *monitor, WinStatsGraph *graph,
               int thread_index, const vector_int &collector_indices,
               bool use_fullname) {

  _ideal_width = 0;

  // First skip the part of the stack that hasn't changed.
  size_t li = 0;
  size_t ci = 0;
  while (ci < collector_indices.size() && li < _labels.size()) {
    WinStatsLabel *label = _labels[li];
    if (collector_indices[ci] != label->get_collector_index()) {
      // Mismatch.
      break;
    }
    _ideal_width = std::max(_ideal_width, label->get_ideal_width());
    ++ci;
    ++li;
  }

  if (ci == collector_indices.size()) {
    if (ci == _labels.size()) {
      // Perfect, nothing changed.
      return;
    }

    // Simple case, just delete the rest.
    while (li < _labels.size()) {
      delete _labels[li++];
    }
    _labels.resize(ci);
    return;
  }

  int yp = _height;
  if (li > 0) {
    WinStatsLabel *label = _labels[li - 1];
    yp = label->get_y() - label->get_height();
  }

  // Make a map of remaining labels.
  std::map<int, WinStatsLabel *> label_map;
  for (size_t li2 = li; li2 < _labels.size(); ++li2) {
    WinStatsLabel *label = _labels[li2];
    label_map[label->get_collector_index()] = label;
  }

  _labels.resize(collector_indices.size());

  while (ci < collector_indices.size()) {
    int collector_index = collector_indices[ci++];

    WinStatsLabel *label;
    auto it = label_map.find(collector_index);
    if (it == label_map.end()) {
      // It's not in the map.  Create a new label.
      label = new WinStatsLabel(monitor, graph, thread_index, collector_index, use_fullname);
      if (_window) {
        label->setup(_window);
      }
    } else {
      // Erase it from the map, so that it's not deleted.
      label = it->second;
      label_map.erase(it);
    }
    if (_window) {
      label->set_pos(0, yp - _scroll, _width);
    }
    _ideal_width = std::max(_ideal_width, label->get_ideal_width());
    yp -= label->get_height();

    _labels[li++] = label;
  }

  // Anything that's remaining in the label map should be deleted.
  for (auto it = label_map.begin(); it != label_map.end(); ++it) {
    delete it->second;
  }

  recalculate_label_positions();
}

/**
 * Returns the number of labels in the stack.
 */
int WinStatsLabelStack::
get_num_labels() const {
  return _labels.size();
}

/**
 * Draws a highlight around the label representing the indicated collector,
 * and removes the highlight from any other label.  Specify -1 to remove the
 * highlight from all labels.
 */
void WinStatsLabelStack::
highlight_label(int collector_index) {
  if (_highlight_label != collector_index) {
    _highlight_label = collector_index;

    for (WinStatsLabel *label : _labels) {
      label->set_highlight(label->get_collector_index() == _highlight_label);
    }
  }
}

/**
 * Refreshes the color of the label with the given index.
 */
void WinStatsLabelStack::
update_label_color(int collector_index) {
  for (WinStatsLabel *label : _labels) {
    if (label->get_collector_index() == collector_index) {
      label->update_color();
    }
  }
}

/**
 * Called to recalculate the positions of all labels in the stack.
 */
void WinStatsLabelStack::
recalculate_label_positions() {
  int total_height = 0;
  for (WinStatsLabel *label : _labels) {
    total_height += label->get_height();
  }
  total_height += _bottom_margin + _top_margin;
  int yp;
  if (total_height <= _height) {
    // Fits.  Align to bottom and reset scroll.
    yp = _height - _bottom_margin;
    _scroll = 0;
  } else {
    // Doesn't fit.  Align to top.
    yp = total_height - _bottom_margin;
    _scroll = (std::min)(_scroll, total_height - _height);
    _scroll = (std::max)(_scroll, 0);
  }
  for (WinStatsLabel *label : _labels) {
    label->set_pos(0, yp - _scroll, _width);
    yp -= label->get_height();
  }
}

/**
 * Creates the window for this stack.
 */
void WinStatsLabelStack::
create_window(HWND parent_window) {
  if (_window) {
    return;
  }

  HINSTANCE application = GetModuleHandle(nullptr);
  register_window_class(application);

  _window =
    CreateWindow(_window_class_name, "label stack", WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                 0, 0, 0, 0,
                 parent_window, nullptr, application, 0);
  if (!_window) {
    nout << "Could not create Label Stack window!\n";
    exit(1);
  }

  SetWindowLongPtr(_window, 0, (LONG_PTR)this);
}

/**
 * Registers the window class for the label window, if it has not already been
 * registered.
 */
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
  wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
  wc.lpszMenuName = nullptr;
  wc.lpszClassName = _window_class_name;

  // Reserve space to associate the this pointer with the window.
  wc.cbWndExtra = sizeof(WinStatsLabelStack *);

  if (!RegisterClass(&wc)) {
    nout << "Could not register Label Stack window class!\n";
    exit(1);
  }

  _window_class_registered = true;
}

/**
 *
 */
LONG WINAPI WinStatsLabelStack::
static_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  WinStatsLabelStack *self = (WinStatsLabelStack *)GetWindowLongPtr(hwnd, 0);
  if (self != nullptr && self->_window == hwnd) {
    return self->window_proc(hwnd, msg, wparam, lparam);
  } else {
    return DefWindowProc(hwnd, msg, wparam, lparam);
  }
}

/**
 *
 */
LONG WinStatsLabelStack::
window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  switch (msg) {
  case WM_PAINT:
    {
      PAINTSTRUCT ps;
      HDC hdc = BeginPaint(hwnd, &ps);

      RECT rect = { 0, 0, _width, _height };
      FillRect(hdc, &rect, (HBRUSH)COLOR_WINDOW);
      EndPaint(hwnd, &ps);
      return 0;
    }

  case WM_MOUSEWHEEL:
    {
      int total_height = 0;
      for (WinStatsLabel *label : _labels) {
        total_height += label->get_height();
      }
      total_height += _bottom_margin + _top_margin;
      if ((total_height > _height || _scroll != 0) && !_labels.empty()) {
        int delta = GET_WHEEL_DELTA_WPARAM(wparam);
        delta = (delta * _labels[0]->get_height()) / 120;
        int new_scroll = _scroll - delta;
        new_scroll = (std::min)(new_scroll, total_height - _height);
        new_scroll = (std::max)(new_scroll, 0);
        delta = new_scroll - _scroll;
        if (delta != 0) {
          _scroll = new_scroll;
          ScrollWindowEx(_window, 0, -delta, NULL, NULL, NULL, NULL, SW_INVALIDATE | SW_SCROLLCHILDREN);
          int yp = yp = total_height - _bottom_margin;
          for (WinStatsLabel *label : _labels) {
            label->set_y_noupdate(yp - _scroll);
            yp -= label->get_height();
          }
        }
      }
      return 0;
    }

  default:
    break;
  }

  return DefWindowProc(hwnd, msg, wparam, lparam);
}
