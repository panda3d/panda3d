/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pStatFlameGraph.cxx
 * @author rdb
 * @date 2022-01-28
 */

#include "pStatFlameGraph.h"

#include "pStatFrameData.h"
#include "pStatCollectorDef.h"
#include "string_utils.h"
#include "config_pstatclient.h"

#include <algorithm>
#include <sstream>

/**
 *
 */
PStatFlameGraph::
PStatFlameGraph(PStatMonitor *monitor, int thread_index, int collector_index,
                int frame_number, int xsize, int ysize) :
  PStatGraph(monitor, xsize, ysize),
  _thread_index(thread_index),
  _collector_index(collector_index),
  _orig_collector_index(collector_index),
  _frame_number(frame_number)
{
  _average_mode = true;
  _average_cursor = 0;
  _current_frame = -1;

  _title_unknown = true;

  // NB. This won't call force_redraw() (which we can't do yet) because average
  // mode is true
  update();
  _time_width = _stack.get_net_value(false);
  if (_time_width == 0.0) {
    _time_width = 1.0 / pstats_target_frame_rate;
  }

  _guide_bar_units = GBU_ms | GBU_hz | GBU_show_units;

  monitor->_flame_graphs.insert(this);
}

/**
 *
 */
PStatFlameGraph::
~PStatFlameGraph() {
  _monitor->_flame_graphs.erase(this);
}

/**
 * Updates the chart with the latest data.
 */
void PStatFlameGraph::
update() {
  const PStatClientData *client_data = _monitor->get_client_data();

  // Don't bother to update the thread data until we know at least something
  // about the collectors and threads.
  if (client_data->get_num_collectors() != 0 &&
      client_data->get_num_threads() != 0) {
    const PStatThreadData *thread_data =
      client_data->get_thread_data(_thread_index);
    if (!thread_data->is_empty()) {
      if (_frame_number >= 0) {
        if (thread_data->has_frame(_frame_number)) {
          if (_current_frame != _frame_number) {
            _current_frame = _frame_number;
            update_data();
          }
        }
      } else {
        int frame_number = thread_data->get_latest_frame_number();
        if (frame_number != _current_frame) {
          _current_frame = frame_number;

          update_data();
        }
      }
    }
  }

  idle();
}

/**
 * Changes the collector represented by this flame graph.  This may force a
 * redraw.
 *
 * Leaves the history stack untouched.
 */
void PStatFlameGraph::
set_collector_index(int collector_index) {
  if (collector_index == -1) {
    // First go back to the collector where we originally opened this graph,
    // and only then go back to the root.
    collector_index = _orig_collector_index;
    if (_collector_index == _orig_collector_index) {
      collector_index = -1;
      _orig_collector_index = -1;
    }
  }
  if (_collector_index != collector_index) {
    _collector_index = collector_index;
    _title_unknown = true;
    _stack.clear();
    update_data();

    if (_average_mode) {
      _stack.update_averages(_average_cursor);
      _time_width = _stack.get_net_value(true);
      if (_time_width == 0.0) {
        _time_width = 1.0 / pstats_target_frame_rate;
      }
      normal_guide_bars();
    }
  }
}

/**
 * Goes to a different collector, but remembers the previous collector.
 */
void PStatFlameGraph::
push_collector_index(int collector_index) {
  if (_collector_index != collector_index) {
    _history.push_back(_collector_index);
    set_collector_index(collector_index);
  }
}

/**
 * Goes to the previous visited collector.  Returns true if the history stack
 * was non-empty.
 */
bool PStatFlameGraph::
pop_collector_index() {
  if (!_history.empty()) {
    int collector_index = _history.back();
    _history.pop_back();
    set_collector_index(collector_index);
    return true;
  }
  return false;
}

/**
 * Changes the frame number shown by this flame graph.  This may force a redraw.
 */
void PStatFlameGraph::
set_frame_number(int frame_number) {
  if (_frame_number != frame_number) {
    _frame_number = frame_number;
    _current_frame = frame_number;
    _title_unknown = true;
    _stack.clear();
    update_data();

    if (_average_mode) {
      _stack.update_averages(_average_cursor);
      _time_width = _stack.get_net_value(true);
      if (_time_width == 0.0) {
        _time_width = 1.0 / pstats_target_frame_rate;
      }
      normal_guide_bars();
    }
  }
}

/**
 * Sets the frame number to the oldest available frame.
 */
bool PStatFlameGraph::
first_frame() {
  const PStatClientData *client_data = _monitor->get_client_data();
  if (client_data == nullptr) {
    return false;
  }

  const PStatThreadData *thread_data = client_data->get_thread_data(_thread_index);
  if (thread_data == nullptr || thread_data->is_empty()) {
    return false;
  }

  int oldest = thread_data->get_oldest_frame_number();
  if (_frame_number != oldest && thread_data->has_frame(oldest)) {
    set_frame_number(oldest);
    return true;
  }
  return false;
}

/**
 * Advances to the next available frame.  Returns true if the frame number was
 * changed after a call to this method.
 */
bool PStatFlameGraph::
next_frame() {
  const PStatClientData *client_data = _monitor->get_client_data();
  if (client_data == nullptr) {
    return false;
  }

  const PStatThreadData *thread_data = client_data->get_thread_data(_thread_index);
  if (thread_data == nullptr || thread_data->is_empty()) {
    return false;
  }

  int latest = thread_data->get_latest_frame_number();
  if (_frame_number < 0 || _frame_number > latest) {
    set_frame_number(latest);
    return true;
  }

  for (int i = _frame_number + 1; i <= latest; ++i) {
    if (thread_data->has_frame(i)) {
      set_frame_number(i);
      return true;
    }
  }

  return false;
}

/**
 * Reverts to the previous frame.  Returns true if the frame number was changed
 * after a call to this method.
 */
bool PStatFlameGraph::
prev_frame() {
  const PStatClientData *client_data = _monitor->get_client_data();
  if (client_data == nullptr) {
    return false;
  }

  const PStatThreadData *thread_data = client_data->get_thread_data(_thread_index);
  if (thread_data == nullptr || thread_data->is_empty()) {
    return false;
  }

  int oldest = thread_data->get_oldest_frame_number();

  int i;
  if (_frame_number < 0) {
    i = thread_data->get_latest_frame_number();
  } else {
    i = _frame_number - 1;
  }
  while (i >= oldest) {
    if (thread_data->has_frame(i)) {
      set_frame_number(i);
      return true;
    }
    --i;
  }

  return false;
}

/**
 * Sets the frame number to the latest available frame.
 */
bool PStatFlameGraph::
last_frame() {
  const PStatClientData *client_data = _monitor->get_client_data();
  if (client_data == nullptr) {
    return false;
  }

  const PStatThreadData *thread_data = client_data->get_thread_data(_thread_index);
  if (thread_data == nullptr || thread_data->is_empty()) {
    return false;
  }

  int latest = thread_data->get_latest_frame_number();
  if (_frame_number != latest && thread_data->has_frame(latest)) {
    set_frame_number(latest);
    return true;
  }
  return false;
}

/**
 * Returns the text suitable for the title label on the top line.
 */
std::string PStatFlameGraph::
get_title_text() {
  std::string text;

  _title_unknown = false;

  const PStatClientData *client_data = _monitor->get_client_data();
  if (_collector_index >= 0) {
    if (client_data->has_collector(_collector_index)) {
      text = client_data->get_collector_fullname(_collector_index);
      text += " flame graph";
    } else {
      _title_unknown = true;
    }

    if (_thread_index != 0) {
      if (client_data->has_thread(_thread_index)) {
        text += " (" + client_data->get_thread_name(_thread_index) + " thread)";
      } else {
        _title_unknown = true;
      }
    }
  }
  else if (client_data->has_thread(_thread_index)) {
    text += client_data->get_thread_name(_thread_index) + " thread flame graph";
  }
  else {
    _title_unknown = true;
  }

  if (_frame_number >= 0) {
    text += " (frame " + format_string(_frame_number) + ")";
  }

  return text;
}

/**
 * Called when the mouse hovers over the graph, and should return the text that
 * should appear on the tooltip.
 */
std::string PStatFlameGraph::
get_bar_tooltip(int depth, int x) const {
  const StackLevel *level = _stack.locate(depth, pixel_to_height(x), _average_mode);
  if (level != nullptr) {
    const PStatClientData *client_data = _monitor->get_client_data();
    if (client_data != nullptr && client_data->has_collector(level->_collector_index)) {
      std::ostringstream text;
      text << client_data->get_collector_fullname(level->_collector_index);
      text << " (" << format_number(level->get_net_value(_average_mode), GBU_show_units | GBU_ms);
      if (level->_count > 1) {
        text << " / " << level->_count << "x";
      }
      text << ")";
      return text.str();
    }
  }
  return std::string();
}

/**
 * Returns the collector index corresponding to the bar at the given location.
 */
int PStatFlameGraph::
get_bar_collector(int depth, int x) const {
  const StackLevel *level = _stack.locate(depth, pixel_to_height(x), _average_mode);
  if (level != nullptr) {
    return level->_collector_index;
  }
  return -1;
}

/**
 * Writes the graph state to a datagram.
 */
void PStatFlameGraph::
write_datagram(Datagram &dg) const {
  dg.add_int16(_orig_collector_index);
  dg.add_float64(_time_width);
  dg.add_bool(_average_mode);

  PStatGraph::write_datagram(dg);
}

/**
 * Restores the graph state from a datagram.
 */
void PStatFlameGraph::
read_datagram(DatagramIterator &scan) {
  _orig_collector_index = scan.get_int16();
  _time_width = scan.get_float64();
  _average_mode = scan.get_bool();

  PStatGraph::read_datagram(scan);

  _current_frame = -1;
  _stack.clear();
  update();
  if (_average_mode) {
    _time_width = _stack.get_net_value(false);
    if (_time_width == 0.0) {
      _time_width = 1.0 / pstats_target_frame_rate;
    }
    normal_guide_bars();
    force_redraw();
  }
}

/**
 *
 */
void PStatFlameGraph::
update_data() {
  const PStatClientData *client_data = _monitor->get_client_data();
  if (client_data == nullptr) {
    return;
  }

  const PStatThreadData *thread_data = client_data->get_thread_data(_thread_index);
  if (thread_data == nullptr || thread_data->is_empty()) {
    return;
  }

  const PStatFrameData &frame_data = thread_data->get_frame(_current_frame);

  bool first_time = _stack._children.empty();

  StackLevel *top = &_stack;
  top->reset();

  size_t num_events = frame_data.get_num_events();
  for (size_t ei = 0; ei < num_events; ++ei) {
    int collector_index = frame_data.get_time_collector(ei);
    double time = frame_data.get_time(ei);

    if (frame_data.is_start(ei)) {
      // If we have a collector index, use it to determine which bottom-level
      // stack frames we are interested in.
      if (_collector_index < 0 ||
          _collector_index == collector_index ||
          top != &_stack) {
        top = top->start(collector_index, time);
      }
      else {
        // Check whether one of the parents matches, perhaps.
        int parent_index = collector_index;
        do {
          const PStatCollectorDef &def = client_data->get_collector_def(parent_index);
          if (parent_index == def._parent_index) {
            break;
          }
          parent_index = def._parent_index;
          if (parent_index == _collector_index) {
            // Yes, let it through.
            top = top->start(collector_index, time);
            break;
          }
        }
        while (parent_index >= 0 && client_data->has_collector(parent_index));
      }
    }
    else {
      top = top->stop(collector_index, time);
    }
  }
  top = top->stop_all(frame_data.get_end());
  nassertv(top == &_stack);

  if (first_time) {
    _stack.reset_averages();
  }

  if (!_average_mode) {
    // Redraw right away, except in average mode, where it's done in animate().
    _time_width = _stack.get_net_value(false);
    if (_time_width == 0.0) {
      _time_width = 1.0 / pstats_target_frame_rate;
    }
    normal_guide_bars();
    force_redraw();
  }
}

/**
 * To be called by the user class when the widget size has changed.  This
 * updates the chart's internal data and causes it to issue redraw commands to
 * reflect the new size.
 */
void PStatFlameGraph::
changed_size(int xsize, int ysize) {
  if (xsize != _xsize || ysize != _ysize) {
    _xsize = xsize;
    _ysize = ysize;

    normal_guide_bars();
    force_redraw();
  }
}

/**
 * To be called by the user class when the whole thing needs to be redrawn for
 * some reason.
 */
void PStatFlameGraph::
force_redraw() {
  begin_draw();
  r_draw_level(_stack);
  end_draw();
}

/**
 * Calls update_guide_bars with parameters suitable to this kind of graph.
 */
void PStatFlameGraph::
normal_guide_bars() {
  // We want vaguely 100 pixels between guide bars.
  int num_bars = get_xsize() / 100;

  _guide_bars.clear();

  double dist = _time_width / num_bars;

  for (int i = 1; i < num_bars; ++i) {
    _guide_bars.push_back(make_guide_bar(i * dist));
  }

  _guide_bars_changed = true;
}

/**
 * Should be overridden by the user class.  This hook will be called before
 * drawing any bars in the chart.
 */
void PStatFlameGraph::
begin_draw() {
}

/**
 * Should be overridden by the user class.  Should draw a single bar at the
 * indicated location.
 */
void PStatFlameGraph::
draw_bar(int depth, int from_x, int to_x, int collector_index, int parent_index) {
}

/**
 * Should be overridden by the user class.  This hook will be called after
 * drawing a series of color bars in the chart.
 */
void PStatFlameGraph::
end_draw() {
}

/**
 * Should be overridden by the user class to perform any other updates might
 * be necessary after the bars have been redrawn.
 */
void PStatFlameGraph::
idle() {
}

/**
 * Should be called periodically to update any animated values.  Returns false
 * to indicate that the animation is done and no longer needs to be called.
 */
bool PStatFlameGraph::
animate(double time, double dt) {
  if (!_average_mode) {
    return false;
  }

  if (_stack.update_averages(_average_cursor)) {
    _time_width = _stack.get_net_value(true);
    if (_time_width == 0.0) {
      _time_width = 1.0 / pstats_target_frame_rate;
    }
    normal_guide_bars();
  }

  // Always use force_redraw, since the mouse position may have changed.
  force_redraw();

  // Cycle through the ring buffers.
  _average_cursor = (_average_cursor + 1) % _num_average_frames;
  return true;
}

/**
 * Resets all the nodes by setting their _net_value to 0.0.
 */
void PStatFlameGraph::StackLevel::
reset() {
  _start_time = 0.0;
  _net_value = 0.0;
  _count = 0;
  _started = false;

  for (auto &item : _children) {
    item.second.reset();
  }
}

/**
 * Starts the given collector, which starts a new stack frame as a child of
 * the current one.  Returns the new child, which is the new stack top.
 */
PStatFlameGraph::StackLevel *PStatFlameGraph::StackLevel::
start(int collector_index, double time) {
  StackLevel &child = _children[collector_index];
  child._parent = this;
  child._collector_index = collector_index;
  child._start_time = std::max(_start_time, time);
  child._count++;
  child._started = true;
  return &child;
}

/**
 * Stops the given collector, which is assumed to be somewhere up the
 * hierarchy.  Should only be called on the top of the stack, usually.
 * Returns the new top of the stack.
 */
PStatFlameGraph::StackLevel *PStatFlameGraph::StackLevel::
stop(int collector_index, double time) {
  StackLevel *new_top = r_stop(collector_index, time);
  if (new_top != nullptr) {
    return new_top;
  }
  // We have a stop event without a preceding start event.  Measure the
  // the time from the start of the current stack frame to the stop time.
  // Actually, don't do this, because it means that a child may end up with
  // more time than the parent.  Need a better solution for this - or not?
  //start(collector_index, _start_time)->stop(collector_index, time);
  return this;
}

/**
 * Stops all still started collectors.  Returns the bottom of the stack,
 * which is also the new top of the stack.
 */
PStatFlameGraph::StackLevel *PStatFlameGraph::StackLevel::
stop_all(double time) {
  if (_parent != nullptr) {
    nassertr(_started, this);
    _net_value += time - _start_time;
    return _parent->stop_all(time);
  } else {
    return this;
  }
}

/**
 * Resets the average calculator, used when first enabling average mode.
 */
void PStatFlameGraph::StackLevel::
reset_averages() {
  double net_value = get_net_value(false);

  for (double &value : _values) {
    value = net_value;
  }
  _avg_net_value = net_value;

  for (auto &item : _children) {
    item.second.reset_averages();
  }
}

/**
 * Recursively calculates the averages.  Returns true if any value was changed.
 */
bool PStatFlameGraph::StackLevel::
update_averages(size_t cursor) {
  _values[cursor] = get_net_value(false);

  bool changed = false;

  double sum = 0;
  for (double value : _values) {
    sum += value;
  }
  double avg = sum / _num_average_frames;
  if (avg != _avg_net_value) {
    _avg_net_value = avg;
    changed = true;
  }

  for (auto &item : _children) {
    if (item.second.update_averages(cursor)) {
      changed = true;
    }
  }

  return changed;
}

/**
 * Locates a stack level at the given depth and the given time offset.
 */
const PStatFlameGraph::StackLevel *PStatFlameGraph::StackLevel::
locate(int depth, double time, bool average) const {
  if (time < 0.0) {
    return nullptr;
  }
  for (const auto &item : _children) {
    double value = item.second.get_net_value(average);
    if (time < value) {
      if (depth == 0) {
        // This is it.
        return &item.second;
      } else {
        // Recurse.
        return item.second.locate(depth - 1, time, average);
      }
    }
    time -= value;
  }
  return nullptr;
}

/**
 * Clears everything.
 */
void PStatFlameGraph::StackLevel::
clear() {
  _children.clear();
  _count = 0;
  _net_value = 0.0;
}

/**
 * Recursive helper used by stop().
 */
PStatFlameGraph::StackLevel *PStatFlameGraph::StackLevel::
r_stop(int collector_index, double time) {
  if (_collector_index == collector_index) {
    // Found it.
    nassertr(_started, nullptr);
    _net_value += time - _start_time;
    _started = false;
    nassertr(_parent != nullptr, nullptr);
    return _parent;
  }
  else if (_parent != nullptr) {
    StackLevel *level = _parent->r_stop(collector_index, time);
    if (level != nullptr) {
      nassertr(_started, nullptr);
      _net_value += time - _start_time;
      _started = false;
      return level;
    }
  }
  return nullptr;
}

/**
 * Recursively draws a level.
 */
void PStatFlameGraph::
r_draw_level(const StackLevel &level, int depth, double offset) {
  for (const auto &item : level._children) {
    const StackLevel &child = item.second;

    double value = child.get_net_value(_average_mode);

    int from_x = height_to_pixel(offset);
    int to_x = height_to_pixel(offset + value);

    // No need to recurse if the bars have become smaller than a pixel.
    if (to_x > from_x) {
      draw_bar(depth, from_x, to_x, child._collector_index, level._collector_index);
      r_draw_level(child, depth + 1, offset);
    }

    offset += value;
  }
}
