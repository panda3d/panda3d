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
PStatFlameGraph(PStatMonitor *monitor, PStatView &view,
                int thread_index, int collector_index, int xsize, int ysize) :
  PStatGraph(monitor, xsize, ysize),
  _thread_index(thread_index),
  _view(view),
  _collector_index(collector_index)
{
  _average_mode = true;
  _average_cursor = 0;

  _time_width = 1.0 / pstats_target_frame_rate;
  _current_frame = -1;

  _title_unknown = true;

  _guide_bar_units = GBU_ms | GBU_hz | GBU_show_units;
  normal_guide_bars();
}

/**
 *
 */
PStatFlameGraph::
~PStatFlameGraph() {
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
      int frame_number = thread_data->get_latest_frame_number();
      if (frame_number != _current_frame) {
        _current_frame = frame_number;

        update_data();
        force_redraw();
        update_labels();
      }
    }
  }

  idle();
}

/**
 * Changes the collector represented by this flame graph.  This may force a
 * redraw.
 */
void PStatFlameGraph::
set_collector_index(int collector_index) {
  if (_collector_index != collector_index) {
    _collector_index = collector_index;
    _title_unknown = true;
    update_data();
    force_redraw();
    update_labels();
  }
}

/**
 * Returns the text suitable for the title label on the top line.
 */
std::string PStatFlameGraph::
get_title_text() {
  std::string text;

  _title_unknown = false;

  const PStatClientData *client_data = _monitor->get_client_data();
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

  return text;
}

/**
 * Called when the mouse hovers over a label, and should return the text that
 * should appear on the tooltip.
 */
std::string PStatFlameGraph::
get_label_tooltip(int collector_index) const {
  const PStatClientData *client_data = _monitor->get_client_data();
  if (!client_data->has_collector(collector_index)) {
    return std::string();
  }

  std::ostringstream text;
  text << client_data->get_collector_fullname(collector_index);

  Data::const_iterator it = _data.find(collector_index);
  if (it != _data.end()) {
    const CollectorData &cd = it->second;
    text << " (" << format_number(cd._net_value, get_guide_bar_units(), get_guide_bar_unit_name()) << ")";
  }

  return text.str();
}

/**
 *
 */
void PStatFlameGraph::
update_data() {
  // First clear the net values, so we'll know which labels should be deleted.
  for (auto it = _data.begin(); it != _data.end(); ++it) {
    it->second._net_value = 0;
  }

  _view.set_to_frame(_current_frame);

  const PStatViewLevel *level = _view.get_level(_collector_index);
  double offset = 0;
  update_data(level, 0, offset);

  _time_width = (offset != 0) ? offset : 1.0 / pstats_target_frame_rate;
  normal_guide_bars();

  // Cycle through the ring buffers.
  _average_cursor = (_average_cursor + 1) % _num_average_frames;
}

/**
 * Recursive helper for get_frame_data.
 */
void PStatFlameGraph::
update_data(const PStatViewLevel *level, int depth, double &offset) {
  double net_value = level->get_net_value();

  Data::iterator it;
  bool inserted;
  std::tie(it, inserted) = _data.insert(std::make_pair(level->get_collector(), CollectorData()));
  CollectorData &cd = it->second;
  cd._offset = offset;
  cd._depth = depth;

  if (inserted || !_average_mode) {
    // Initialize the values array.
    for (double &v : cd._values) {
      v = net_value;
    }
    cd._net_value = net_value;
  } else {
    cd._values[_average_cursor] = net_value;

    // Calculate the average.
    cd._net_value = 0;
    for (double value : cd._values) {
      cd._net_value += value;
    }
    cd._net_value /= _num_average_frames;
  }

  if (cd._net_value != 0.0) {
    cd._net_value = std::max(cd._net_value, 0.0);

    double child_offset = offset;
    offset += cd._net_value;

    int num_children = level->get_num_children();
    for (int i = 0; i < num_children; i++) {
      const PStatViewLevel *child = level->get_child(i);
      update_data(child, depth + 1, child_offset);
    }
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
    update_labels();
  }
}

/**
 * To be called by the user class when the whole thing needs to be redrawn for
 * some reason.
 */
void PStatFlameGraph::
force_redraw() {
  begin_draw();
  end_draw();
}

/**
 * Resets the list of labels.
 */
void PStatFlameGraph::
update_labels() {
  for (auto it = _data.begin(); it != _data.end(); ++it) {
    int collector_index = it->first;
    const CollectorData &cd = it->second;

    update_label(collector_index, cd._depth, height_to_pixel(cd._offset), height_to_pixel(cd._net_value));
  }
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
