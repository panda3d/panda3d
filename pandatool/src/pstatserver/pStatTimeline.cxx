/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pStatTimeline.cxx
 * @author rdb
 * @date 2022-02-11
 */

#include "pStatTimeline.h"

#include "pStatFrameData.h"
#include "pStatCollectorDef.h"
#include "string_utils.h"
#include "config_pstatclient.h"

#include <algorithm>

/**
 *
 */
PStatTimeline::
PStatTimeline(PStatMonitor *monitor, int xsize, int ysize) :
  PStatGraph(monitor, xsize, ysize)
{
  // Default to 1 millisecond per 10 pixels.
  _time_scale = 1 / 10000.0;
  _target_time_scale = _time_scale;

  _guide_bar_units = GBU_ms | GBU_show_units;

  // Load in the initial data, so that the user can see everything back to the
  // beginning (or as far as pstats-history goes back to).
  const PStatClientData *client_data = monitor->get_client_data();
  if (client_data != nullptr) {
    size_t row_offset = 0;

    // Ignore this nasty "inverted" collector, which messes up everything.
    _app_collector_index = client_data->find_collector("App:Show code:General");

    for (int thread_index = 0; thread_index < client_data->get_num_threads(); ++thread_index) {
      _threads.emplace_back();
      ThreadRow &thread_row = _threads.back();
      thread_row._row_offset = row_offset;

      if (!client_data->has_thread(thread_index)) {
        continue;
      }
      thread_row._visible = true;

      const PStatThreadData *thread_data = client_data->get_thread_data(thread_index);
      if (thread_data != nullptr) {
        _threads_changed = true;

        if (!thread_data->is_empty()) {
          int oldest_frame = thread_data->get_oldest_frame_number();
          int latest_frame = thread_data->get_latest_frame_number();

          double oldest_start_time = thread_data->get_frame(oldest_frame).get_start();
          double latest_end_time = thread_data->get_frame(latest_frame).get_end();

          if (!_have_start_time) {
            _have_start_time = true;
            _lowest_start_time = oldest_start_time;
          }
          else {
            _lowest_start_time = std::min(_lowest_start_time, oldest_start_time);
          }
          _highest_end_time = std::max(_highest_end_time, latest_end_time + _clock_skew);

          for (int frame = oldest_frame; frame <= latest_frame; ++frame) {
            if (thread_data->has_frame(frame)) {
              update_bars(thread_index, frame);
            }
          }
        }
      }

      row_offset += thread_row._rows.size() + 1;
    }
  }

  _start_time = _lowest_start_time;
  _target_start_time = _start_time;

  monitor->_timelines.insert(this);
}

/**
 *
 */
PStatTimeline::
~PStatTimeline() {
  _monitor->_timelines.erase(this);
}

/**
 * Called as each frame's data is made available.  There is no guarantee the
 * frames will arrive in order, or that all of them will arrive at all.  The
 * monitor should be prepared to accept frames received out-of-order or
 * missing.
 */
void PStatTimeline::
new_data(int thread_index, int frame_number) {
  const PStatClientData *client_data = _monitor->get_client_data();

  if (client_data != nullptr) {
    const PStatThreadData *thread_data =
      client_data->get_thread_data(thread_index);

    if (thread_data != nullptr && thread_data->has_frame(frame_number)) {
      const PStatFrameData &frame_data = thread_data->get_frame(frame_number);
      double frame_start = frame_data.get_start() + _clock_skew;
      double frame_end = frame_data.get_end() + _clock_skew;

      if (thread_index == 0 && _app_collector_index == -1) {
        _app_collector_index = client_data->find_collector("App:Show code:General");
      }

      if (!_have_start_time) {
        _start_time = frame_start;
        _have_start_time = true;
        _lowest_start_time = _start_time;
      }
      else if (_start_time < _lowest_start_time) {
        _lowest_start_time = _start_time;
      }
      if (frame_end > _highest_end_time) {
        _highest_end_time = frame_end;
      }

      while (thread_index >= (int)_threads.size()) {
        _threads_changed = true;
        if (_threads.size() == 0) {
          _threads.resize(1);
        } else {
          _threads.resize(_threads.size() + 1);
          _threads[_threads.size() - 1]._row_offset =
            _threads[_threads.size() - 2]._row_offset;
          if (_threads[_threads.size() - 2]._visible) {
            _threads[_threads.size() - 1]._row_offset +=
              _threads[_threads.size() - 2]._rows.size() + 1;
          }
        }
      }

      if (update_bars(thread_index, frame_number)) {
        // The number of rows was changed.
        // Change the offset of all subsequent ThreadRows.
        ThreadRow &thread_row = _threads[thread_index];
        size_t offset = thread_row._row_offset + thread_row._rows.size() + 1;
        for (size_t ti = (size_t)(thread_index + 1); ti < _threads.size(); ++ti) {
          _threads[ti]._row_offset = offset;
          if (_threads[ti]._visible) {
            offset += _threads[ti]._rows.size() + 1;
          }
        }
        _threads_changed = true;
        normal_guide_bars();
        force_redraw();
      }
      else if (frame_end >= _start_time || frame_start <= _start_time + get_horizontal_scale()) {
        normal_guide_bars();
        begin_draw();
        draw_thread(thread_index, frame_start, frame_end);
        end_draw();
      }
    }
  }

  idle();
}

/**
 * Called by new_data().  Updates the bars without doing any drawing.  Returns
 * true if the number of rows was changed (forcing a full redraw), false if
 * only new bars were added on the right side.
 */
bool PStatTimeline::
update_bars(int thread_index, int frame_number) {
  const PStatClientData *client_data = _monitor->get_client_data();
  const PStatThreadData *thread_data = client_data->get_thread_data(thread_index);
  const PStatFrameData &frame_data = thread_data->get_frame(frame_number);
  ThreadRow &thread_row = _threads[thread_index];
  thread_row._label = client_data->get_thread_name(thread_index);
  bool changed_num_rows = false;

  if (!thread_row._visible) {
    thread_row._visible = true;
    changed_num_rows = true;
  }

  // pair<int collector_index, double start_time>
  pvector<std::pair<int, double> > stack;
  bool had_children = false;

  double frame_start = frame_data.get_start() + _clock_skew;
  double prev = frame_start;

  // There may still be open collectors from the previous frame.  Rebuild the
  // stack based on that so we can close them properly.
  for (size_t i = 0; i < thread_row._rows.size(); ++i) {
    Row &row = thread_row._rows[i];

    // Keep iterating from the back until we get to the previous frame.
    for (size_t j = 0; j < row.size(); ++j) {
      ColorBar &bar = row[row.size() - j - 1];
      if (bar._frame_number < frame_number) {
        // Was this collector left unstopped at the end of the last frame?
        if (bar._open_end && bar._frame_number == frame_number - 1) {
          if (i >= stack.size()) {
            stack.resize(i + 1, std::make_pair(-1, 0.0));
          }
          stack[i] = std::make_pair(bar._collector_index, bar._start);

          // Remove this bar for now, we'll recreate it when we close it.
          row.erase(row.begin() + (row.size() - j - 1));
        }
        break;
      }
    }
  }

  size_t num_events = frame_data.get_num_events();
  for (size_t i = 0; i < num_events; ++i) {
    int collector_index = frame_data.get_time_collector(i);
    double time = frame_data.get_time(i) + _clock_skew;

    if (time < prev) {
      // Apparently, it is too hard to ask for monotonically increasing time
      // values!  Don't be tempted to sort the values, or think this is a bug,
      // the client can arbitrarily move back the clock if it feels like it!
      // I had some fancy mechanism that handled clock skew properly for
      // multiple out-of-order frames, but I gave up when I realised it's not
      // possible to solve this problem for multiple threads, so we just assume
      // this is done just once at startup before the other threads exist.
      double delta = prev - time;
      _clock_skew += delta;
      _highest_end_time += delta;
      time = prev;
      nout << "Correcting for clock skew of "
           << format_number(delta, GBU_show_units | GBU_ms)
           << " in frame " << frame_number << " of thread "
           << thread_index << "\n";

      // Move all bars after this frame to the right by this amount.
      for (ThreadRow &thread_row : _threads) {
        for (Row &row : thread_row._rows) {
          for (ColorBar &bar : row) {
            if (bar._frame_number > frame_number) {
              bar._start += delta;
              bar._end += delta;
            }
          }
        }
      }
    }
    prev = time;

    if (collector_index == _app_collector_index) {
      continue;
    }

    if (frame_data.is_start(i)) {
      if (collector_index == 0 && !stack.empty()) {
        // Collector 0 always goes at the top, no matter what.
        stack[0].first = 0;
        stack[0].second = std::max(time, _start_time);
      } else {
        stack.push_back(std::make_pair(collector_index, std::max(time, _start_time)));
        if (stack.size() > thread_row._rows.size()) {
          thread_row._rows.resize(stack.size());
          changed_num_rows = true;
        }
        had_children = false;
      }
    }
    else if (!stack.empty() && stack.back().first == collector_index) {
      // Most likely case, ending the most recent collector that is still open.
      double start_time = stack.back().second;
      stack.pop_back();
      thread_row._rows[stack.size()].push_back({
        start_time, time, collector_index, thread_index, frame_number, false, false});

      // Pop off stack levels for prematurely ended collectors (see below).
      while (!stack.empty() && stack.back().first < 0) {
        stack.pop_back();
      }
      had_children = true;
    }
    else {
      // Unlikely case: ending a collector before a "child" has ended.
      // Go back and clear the row where this collector started.
      // Don't decrement the row index.
      size_t i;
      for (i = 0; i < stack.size(); ++i) {
        auto &item = stack[stack.size() - 1 - i];

        if (item.first == collector_index) {
          thread_row._rows[stack.size() - 1 - i].push_back({
            item.second, time, collector_index, thread_index, frame_number, false, false});
          item.first = -1;
          break;
        }
      }
      if (i == stack.size()) {
        // We stopped a collector that wasn't started at all.  That means it
        // must have already been running when this frame started, which we
        // normally handle by pre-populating the stack with the open collectors
        // from last frame, but maybe we didn't get the last frame (yet).
        // Unfortunately we need to add it above any bars we've already started
        // and stopped, which means we need to shift everything down belonging
        // to this frame, except for the top-level Frame collector.  This is
        // probably pretty inefficient, but it should be pretty rare for frames
        // to arrive out-of-order in this manner.
        nassertd(!stack.empty()) continue;
        stack.insert(stack.begin() + 1, std::make_pair(-1, 0.0));

        const size_t num_rows = thread_row._rows.size();
        for (size_t j = num_rows - 1; j >= 1; --j) {
          size_t row_size = thread_row._rows[j].size();
          for (size_t k = row_size; k > 0; --k) {
            Row &row = thread_row._rows[j];
            ColorBar bar = row[k - 1];
            if (bar._frame_number == frame_number) {
              row.erase(row.begin() + (k - 1));
              if (j + 1 >= thread_row._rows.size()) {
                thread_row._rows.resize(j + 2);
                changed_num_rows = true;
              }

              // Insert it into the row below while retaining sorting.
              Row &row2 = thread_row._rows[j + 1];
              row2.insert(std::upper_bound(row2.begin(), row2.end(), bar), bar);
            }
            else if (bar._frame_number < frame_number) {
              break;
            }
          }
        }

        // Now insert the bar, just below Frame level.
        if (thread_row._rows.size() < 2) {
          thread_row._rows.resize(2);
          changed_num_rows = true;
        }
        thread_row._rows[1].push_back({
          frame_start, time, collector_index, thread_index,
          frame_number, true, false});
      }
      else if (i > 0 && !had_children) {
        // Figure out if the currently active collector could actually be
        // slotted higher.  This prevents the staircase effect where
        // overlapping collectors will cause the number of rows to continue
        // to grow at every overlap.  We only do this if the current
        // collector has not had children yet, that'd be too confusing.
        int j = stack.size() - 3;
        while (j > 0) {
          auto &item = stack[j];
          if (item.first >= 0) {
            // Nope, can't do.
            break;
          }
          // Yes, does this row have enough space?
          Row &row = thread_row._rows[j];
          if (row.empty() || row.back()._end < stack.back().second) {
            // It does.
            item = std::move(stack.back());
            stack.pop_back();
            while (!stack.empty() && stack.back().first < 0) {
              stack.pop_back();
            }
            break;
          }
          --j;
        }
      }
    }
  }

  // Add all unclosed bars, marking them as open-ended, for later gluing.
  while (!stack.empty()) {
    int collector_index = stack.back().first;
    if (collector_index >= 0) {
      double start_time = stack.back().second;
      thread_row._rows[stack.size() - 1].push_back({
        start_time, frame_data.get_end() + _clock_skew,
        collector_index, thread_index, frame_number, false, true,
      });
    }
    stack.pop_back();
  }

  if (thread_row._last_frame >= 0 && frame_number < thread_row._last_frame) {
    // Added a frame out of order.
    for (Row &row : thread_row._rows) {
      // Sort by end time.
      std::sort(row.begin(), row.end());

      // Glue together open ends and beginnings that match up.
      size_t end = row.size() - 1;
      for (size_t i = 0; i < end;) {
        ColorBar &left = row[i];
        ColorBar &right = row[i + 1];
        if (left._collector_index == right._collector_index &&
            left._frame_number + 1 == right._frame_number &&
            left._open_end && right._open_begin) {
          // Erase the left one, to maintain the sorting by end time.
          right._open_begin = false;
          right._start = left._start;
          row.erase(row.begin() + i);
          --end;
        } else {
          ++i;
        }
      }
    }
  } else {
    thread_row._last_frame = frame_number;
  }

  return changed_num_rows;
}

/**
 * Called when the mouse hovers over the graph, and should return the text that
 * should appear on the tooltip.
 */
std::string PStatTimeline::
get_bar_tooltip(int row, int x) const {
  ColorBar bar;
  if (find_bar(row, x, bar)) {
    const PStatClientData *client_data = _monitor->get_client_data();
    if (client_data != nullptr && client_data->has_collector(bar._collector_index)) {
      std::ostringstream text;
      text << client_data->get_collector_fullname(bar._collector_index);
      text << " (";
      if (bar._open_begin || bar._open_end) {
        text << "at least ";
      }
      text << format_number(bar._end - bar._start, GBU_show_units | GBU_ms) << ")";
      return text.str();
    }
  }
  return std::string();
}

/**
 * Writes the graph state to a datagram.
 */
void PStatTimeline::
write_datagram(Datagram &dg) const {
  dg.add_float64(_time_scale);
  dg.add_float64(_start_time);
  dg.add_float64(_lowest_start_time);
  dg.add_float64(_highest_end_time);

  PStatGraph::write_datagram(dg);
}

/**
 * Restores the graph state from a datagram.
 */
void PStatTimeline::
read_datagram(DatagramIterator &scan) {
  _time_scale = scan.get_float64();
  _start_time = scan.get_float64();
  _lowest_start_time = scan.get_float64();
  _highest_end_time = scan.get_float64();

  _scroll_speed = 0.0;
  _zoom_speed = 0.0;

  _have_start_time = true;
  _target_start_time = _start_time;
  _target_time_scale = _time_scale;

  PStatGraph::read_datagram(scan);

  normal_guide_bars();
  force_redraw();
}

/**
 * To be called by the user class when the widget size has changed.  This
 * updates the chart's internal data and causes it to issue redraw commands to
 * reflect the new size.
 */
void PStatTimeline::
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
void PStatTimeline::
force_redraw() {
  clear_region();

  begin_draw();

  for (const GuideBar &bar : _guide_bars) {
    int x = timestamp_to_pixel(bar._height);
    if (x > 0 && x < get_xsize() - 1) {
      draw_guide_bar(x, bar._style);
    }
  }

  double start_time = _start_time;
  double end_time = start_time + get_horizontal_scale();

  int num_rows = 0;

  for (size_t ti = 0; ti < _threads.size(); ++ti) {
    ThreadRow &thread_row = _threads[ti];
    if (thread_row._visible) {
      for (size_t ri = 0; ri < thread_row._rows.size(); ++ri) {
        draw_row((int)ti, (int)ri, start_time, end_time);
        ++num_rows;
      }
      draw_separator(num_rows++);
    }
  }

  end_draw();
}

/**
 * To be called by the user class when the whole thing needs to be redrawn for
 * some reason.
 */
void PStatTimeline::
force_redraw(int row, int from_x, int to_x) {
  double start_time = std::max(_start_time, pixel_to_timestamp(from_x));
  double end_time = std::min(_start_time + get_horizontal_scale(), pixel_to_timestamp(to_x));

  begin_draw();

  for (size_t ti = 0; ti < _threads.size(); ++ti) {
    ThreadRow &thread_row = _threads[ti];
    if (!thread_row._visible || (int)thread_row._row_offset > row) {
      break;
    }

    int row_index = row - (int)thread_row._row_offset;
    if (row_index < (int)thread_row._rows.size()) {
      draw_row((int)ti, row_index, start_time, end_time);
    }
  }

  end_draw();
}

/**
 * Calls update_guide_bars with parameters suitable to this kind of graph.
 */
void PStatTimeline::
normal_guide_bars() {
  double start_time = get_horizontal_scroll();
  double time_width = get_horizontal_scale();
  double end_time = start_time + time_width;

  // We want vaguely 150 pixels between guide bars.
  int max_frames = get_xsize() / 100;
  int l = (int)std::floor(3.0 * log10(pixel_to_height(150)) + 0.5);
  double interval = pow(10.0, std::ceil(l / 3.0));
  if ((l + 3000) % 3 == 1) {
    interval /= 5;
  }
  else if ((l + 3000) % 3 == 2) {
    interval /= 2;
  }

  _guide_bars.clear();

  // Rather than getting the client data, we look in the color bar data for
  // the first row, because the client data gets wiped after a while.
  if (!_threads.empty() && !_threads[0]._rows.empty()) {
    const Row &row = _threads[0]._rows[0];

    // Look for the last Frame bar with end time lower than our start time.
    Row::const_iterator it = std::lower_bound(row.begin(), row.end(), ColorBar {0.0, start_time});
    while (it != row.end() && it->_collector_index != 0) {
      ++it;
    }

    int num_frames = 0;

    while (it != row.end() && it->_start <= end_time) {
      double frame_start = it->_start;
      double frame_end = it->_end;
      int frame_number = it->_frame_number;

      if (frame_start > start_time) {
        if (!_guide_bars.empty() && height_to_pixel(frame_start - _guide_bars.back()._height) < 30) {
          // Get rid of last label, it is in the way.
          _guide_bars.back()._label.clear();
        }
        std::string label = "#";
        label += format_string(frame_number);
        _guide_bars.push_back(GuideBar(frame_start, label, GBS_frame));

        if (++num_frames > max_frames) {
          // Forget it, this is becoming too many lines.
          _guide_bars.clear();
          break;
        }
      }

      do {
        ++it;
      }
      while (it != row.end() && it->_collector_index != 0);

      double frame_width;
      if (it != row.end()) {
        // Only go up to the start of the next frame, limiting to however much
        // fits in the graph.
        frame_width = std::min(frame_end - frame_start, end_time - frame_start);
      } else {
        // Reached the end; just continue to the end of the graph.
        frame_width = end_time - frame_start;
      }

      if (interval > 0.0) {
        int first_bar = std::max((int)((start_time - frame_start) / interval), 1);
        int num_bars = (int)std::round(frame_width / interval);

        for (int i = first_bar; i < num_bars; ++i) {
          double offset = i * interval;
          std::string label = "+";
          label += format_number(offset, GBU_show_units | GBU_ms);
          _guide_bars.push_back(GuideBar(frame_start + offset, label, GBS_normal));
        }
      }

      // If there's a gap between frames, add another line.
      if (it->_start > frame_end && it->_frame_number > frame_number + 1) {
        std::string label;
        if (it->_start - frame_end >= interval) {
          label = "#" + format_string(frame_number + 1);
          if (it->_frame_number > frame_number + 2) {
            label += "-" + format_string(it->_frame_number - 1);
          }
          label += " (dropped)";
        }
        _guide_bars.push_back(GuideBar(frame_end, label, GBS_frame));
      }
    }
  }

  if (_guide_bars.empty() && interval > 0.0) {
    int first_bar = std::max((int)(start_time / interval), 1);
    int num_bars = (int)std::round(end_time / interval);

    for (int i = first_bar; i < num_bars; ++i) {
      double time = i * interval;
      std::string label = format_number(time, GBU_show_units | GBU_ms);
      _guide_bars.push_back(GuideBar(time, label, GBS_frame));
    }
  }

  _guide_bars_changed = true;
}

/**
 * Should be overridden by the user class to wipe out the entire strip chart
 * region.
 */
void PStatTimeline::
clear_region() {
}

/**
 * Should be overridden by the user class.  This hook will be called before
 * drawing any bars in the chart.
 */
void PStatTimeline::
begin_draw() {
}

/**
 *
 */
void PStatTimeline::
draw_thread(int thread_index, double start_time, double end_time) {
  if (thread_index < 0 || (size_t)thread_index > _threads.size()) {
    return;
  }

  ThreadRow &thread_row = _threads[(size_t)thread_index];
  if (thread_row._visible) {
    for (size_t ri = 0; ri < thread_row._rows.size(); ++ri) {
      draw_row(thread_index, (int)ri, start_time, end_time);
    }
  }
}

/**
 *
 */
void PStatTimeline::
draw_row(int thread_index, int row_index, double start_time, double end_time) {
  ThreadRow &thread_row = _threads[thread_index];
  Row &row = thread_row._rows[row_index];

  const PStatClientData *client_data = _monitor->get_client_data();

  // Find the first element whose end time is larger than our start time.
  // Then iterate until at least the end of the frame.
  Row::iterator it = std::lower_bound(row.begin(), row.end(), ColorBar {0.0, start_time});
  if (it == row.end()) {
    return;
  }

  int frame_number = it->_frame_number;
  do {
    ColorBar &bar = *it;

    int from_x = timestamp_to_pixel(bar._start);
    int to_x = timestamp_to_pixel(bar._end);

    if (to_x >= 0 && to_x > from_x && from_x < get_xsize()) {
      if (bar._collector_index != 0) {
        const PStatCollectorDef &def = client_data->get_collector_def(bar._collector_index);
        if (to_x - from_x >= 32 && def._parent_index > 0) {
          // Try including the parent name.
          const PStatCollectorDef &parent_def = client_data->get_collector_def(def._parent_index);
          std::string long_name = parent_def._name + ":" + def._name;
          draw_bar(thread_row._row_offset + row_index, from_x, to_x,
                   bar._collector_index, long_name);
        } else {
          draw_bar(thread_row._row_offset + row_index, from_x, to_x,
                   bar._collector_index, def._name);
        }
      } else {
        draw_bar(thread_row._row_offset + row_index, from_x, to_x,
                 bar._collector_index,
                 std::string("Frame ") + format_string(bar._frame_number));
      }
    }

    ++it;
  }
  while (it != row.end() && (it->_start <= end_time || it->_frame_number == frame_number));
}

/**
 * Draws a horizontal separator.
 */
void PStatTimeline::
draw_separator(int) {
}

/**
 * Draws a vertical guide bar.  If the row is -1, draws it in all rows.
 */
void PStatTimeline::
draw_guide_bar(int x, GuideBarStyle style) {
}

/**
 * Draws a single bar in the chart for the indicated row, in the color for the
 * given collector, for the indicated horizontal pixel range.
 */
void PStatTimeline::
draw_bar(int, int, int, int, const std::string &) {
}

/**
 * Should be overridden by the user class.  This hook will be called after
 * drawing a series of color bars in the chart.
 */
void PStatTimeline::
end_draw() {
}

/**
 * Should be overridden by the user class to perform any other updates might
 * be necessary after the bars have been redrawn.
 */
void PStatTimeline::
idle() {
}

/**
 * Should be called periodically to update any animated values.  Returns false
 * to indicate that the animation is done and no longer needs to be called.
 */
bool PStatTimeline::
animate(double time, double dt) {
  int hmove = ((_keys_held & (F_right | F_d)) != 0)
            - ((_keys_held & (F_left | F_a)) != 0);
  int vmove = ((_keys_held & F_w) != 0)
            - ((_keys_held & F_s) != 0);

  if (hmove > 0) {
    if (_scroll_speed < 0) {
      _scroll_speed = 1.0;
    }
    _scroll_speed += 1.0;
  }
  else if (hmove < 0) {
    if (_scroll_speed > 0) {
      _scroll_speed = -1.0;
    }
    _scroll_speed -= 1.0;
  }
  else if (_scroll_speed != 0.0) {
    _scroll_speed *= std::exp(-12.0 * dt);
    if (std::abs(_scroll_speed) < 0.2) {
      _scroll_speed = 0.0;
    }
  }

  if (vmove > 0) {
    if (_zoom_speed < 0) {
      _zoom_speed = 1.0;
    }
    _zoom_speed += 1.0;
  }
  else if (vmove < 0) {
    if (_zoom_speed > 0) {
      _zoom_speed = -1.0;
    }
    _zoom_speed -= 1.0;
  }
  else if (_zoom_speed != 0.0) {
    _zoom_speed *= std::exp(-12.0 * dt);
    if (std::abs(_zoom_speed) < 0.2) {
      _zoom_speed = 0.0;
    }
  }

  if (_zoom_speed != 0.0) {
    zoom_to(get_horizontal_scale() * pow(0.5, _zoom_speed * dt), _zoom_center);
  }

  if (_scroll_speed != 0.0) {
    scroll_by(_scroll_speed * 300 * _time_scale * dt);
  }

  if (_target_start_time != _start_time) {
    double dist = _target_start_time - _start_time;
    // When the difference is less than 2 pixels, snap to target position.
    if (std::abs(dist) < _time_scale * 2) {
      _start_time = _target_start_time;
    } else {
      dist *= 1.0 - std::exp(-12.0 * dt);
      _start_time += dist;
    }
  }

  if (_target_time_scale != _time_scale) {
    //double dist = std::log(_target_time_scale) - std::log(_time_scale);
    double dist = _target_time_scale - _time_scale;
    if (_target_start_time == _start_time && std::abs(dist) < 0.01) {
      _time_scale = _target_time_scale;
    } else {
      dist *= 1.0 - std::exp(-12.0 * dt);
      //_time_scale *= std::exp(dist);
      _time_scale += dist;
    }
  }

  normal_guide_bars();
  force_redraw();

  // Stop the animation when the speed is 0 and no key is still held.
  return _keys_held != 0
      || _scroll_speed != 0
      || _zoom_speed != 0
      || _target_start_time != _start_time
      || _target_time_scale != _time_scale;
}

/**
 * Return the ColorBar at the indicated position.
 */
bool PStatTimeline::
find_bar(int row, int x, ColorBar &bar) const {
  double time = pixel_to_timestamp(x);

  for (size_t ti = 0; ti < _threads.size(); ++ti) {
    const ThreadRow &thread_row = _threads[ti];
    if ((int)thread_row._row_offset > row) {
      break;
    }

    int row_index = row - (int)thread_row._row_offset;
    if (row_index < (int)thread_row._rows.size()) {
      // Find the first element whose end time is larger than the given time.
      const Row &bars = thread_row._rows[row_index];
      Row::const_iterator it = std::lower_bound(bars.begin(), bars.end(), ColorBar {time, time});
      if (it != bars.end() && it->_start <= time) {
        bar = *it;
        return true;
      }
    }
  }

  return false;
}
