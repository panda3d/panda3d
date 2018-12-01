/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pStatPianoRoll.cxx
 * @author drose
 * @date 2000-07-18
 */

#include "pStatPianoRoll.h"

#include "pStatFrameData.h"
#include "pStatCollectorDef.h"
#include "string_utils.h"
#include "config_pstatclient.h"

#include <algorithm>

/**
 * This class is used internally to build up the set of color bars defined by
 * a frame's worth of data.
 */
PStatPianoRoll::BarBuilder::
BarBuilder() {
  _is_new = true;
}

/**
 * Resets the data in the BarBuilder for a new frame.
 */
void PStatPianoRoll::BarBuilder::
clear() {
  _is_new = false;
  _color_bars.clear();
}

/**
 * Adds a new data point.  The first data point for a given collector turns in
 * on (starts the bar), the second data point turns it off (ends the bar).
 */
void PStatPianoRoll::BarBuilder::
add_data_point(double time, bool is_start) {
  if (is_start) {
    // This is a "start" data point: start the bar.
    if (_color_bars.empty() || _color_bars.back()._end >= 0.0) {
      ColorBar bar;
      bar._start = time;
      bar._end = -1.0;
      _color_bars.push_back(bar);
    }

  } else {
    // This is a "stop" data point: end the bar.
    if (_color_bars.empty()) {
      // A "stop" in the middle of the frame implies a "start" at time 0.
      ColorBar bar;
      bar._start = 0.0;
      bar._end = time;
      _color_bars.push_back(bar);

    } else {
      _color_bars.back()._end = time;
    }
  }
}

/**
 * Makes sure that each start-bar data point was matched by a corresponding
 * end-bar data point.
 */
void PStatPianoRoll::BarBuilder::
finish(double time) {
  if (!_color_bars.empty() && _color_bars.back()._end < 0.0) {
    _color_bars.back()._end = time;
  }
}

/**
 *
 */
PStatPianoRoll::
PStatPianoRoll(PStatMonitor *monitor, int thread_index, int xsize, int ysize) :
  PStatGraph(monitor, xsize, ysize),
  _thread_index(thread_index)
{
  _time_width = 1.0 / pstats_target_frame_rate;
  _start_time = 0.0;

  _current_frame = -1;
  _guide_bar_units = GBU_ms | GBU_hz | GBU_show_units;
  normal_guide_bars();
}

/**
 *
 */
PStatPianoRoll::
~PStatPianoRoll() {
}

/**
 * Updates the chart with the latest data.
 */
void PStatPianoRoll::
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
        compute_page(thread_data->get_frame(frame_number));
        _current_frame = frame_number;
        force_redraw();
      }
    }
  }

  idle();
}

/**
 * To be called by the user class when the widget size has changed.  This
 * updates the chart's internal data and causes it to issue redraw commands to
 * reflect the new size.
 */
void PStatPianoRoll::
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
void PStatPianoRoll::
force_redraw() {
  if (!_labels.empty()) {
    begin_draw();
    for (int i = 0; i < (int)_labels.size(); i++) {
      int collector_index = _labels[i];
      const ColorBars &bars = _page_data[collector_index]._color_bars;

      begin_row(i);
      ColorBars::const_iterator bi;
      for (bi = bars.begin(); bi != bars.end(); ++bi) {
        const ColorBar &bar = (*bi);
        draw_bar(i, timestamp_to_pixel(bar._start), timestamp_to_pixel(bar._end));
      }
      end_row(i);
    }
    end_draw();
  }
}

/**
 * Calls update_guide_bars with parameters suitable to this kind of graph.
 */
void PStatPianoRoll::
normal_guide_bars() {
  // We want vaguely 100 pixels between guide bars.
  update_guide_bars(get_xsize() / 100, _time_width);
}

/**
 * Should be overridden by the user class.  This hook will be called before
 * drawing any bars in the chart.
 */
void PStatPianoRoll::
begin_draw() {
}

/**
 * Should be overridden by the user class.  This hook will be called before
 * drawing any one row of bars.  These bars correspond to the collector whose
 * index is get_row_collector(row), and in the color get_row_color(row).
 */
void PStatPianoRoll::
begin_row(int) {
}

/**
 * Draws a single bar in the chart for the indicated row, in the color
 * get_row_color(row), for the indicated horizontal pixel range.
 */
void PStatPianoRoll::
draw_bar(int, int, int) {
}

/**
 * Should be overridden by the user class.  This hook will be called after
 * drawing a series of color bars for a single row.
 */
void PStatPianoRoll::
end_row(int) {
}

/**
 * Should be overridden by the user class.  This hook will be called after
 * drawing a series of color bars in the chart.
 */
void PStatPianoRoll::
end_draw() {
}

/**
 * Should be overridden by the user class to perform any other updates might
 * be necessary after the bars have been redrawn.
 */
void PStatPianoRoll::
idle() {
}


// STL function object for sorting labels in order by the collector's sort
// index, used in compute_page(), below.
class SortCollectorLabels1 {
public:
  SortCollectorLabels1(const PStatClientData *client_data) :
    _client_data(client_data) {
  }
  bool operator () (int a, int b) const {
    return
      _client_data->get_collector_def(a)._sort >
      _client_data->get_collector_def(b)._sort;
  }
  const PStatClientData *_client_data;
};

/**
 * Examines the given frame data and rebuilds the _page_data to match it.
 */
void PStatPianoRoll::
compute_page(const PStatFrameData &frame_data) {
  _start_time = frame_data.get_start();

  // Clear out the page data and copy it to previous, so we can fill it up
  // again and then check to see if we changed the set of bars this frame.
  PageData previous;
  _page_data.swap(previous);

  int num_events = frame_data.get_num_events();
  for (int i = 0; i < num_events; i++) {
    int collector_index = frame_data.get_time_collector(i);
    double time = frame_data.get_time(i);
    bool is_start = frame_data.is_start(i);
    _page_data[collector_index].add_data_point(time, is_start);
  }

  // Now check to see if the set of bars has changed.
  bool changed_bars = (_page_data.size() != previous.size());

  if (!changed_bars) {
    PageData::const_iterator ai, bi;
    ai = _page_data.begin();
    bi = previous.begin();
    while (ai != _page_data.end() && !changed_bars) {
      changed_bars = ((*ai).first == (*bi).first);
      ++ai;
      ++bi;
    }
  }

  if (changed_bars) {
    // If we added or removed some new bars this time, we'll have to update
    // our list.
    const PStatClientData *client_data = _monitor->get_client_data();

    _labels.clear();
    PageData::const_iterator pi;
    for (pi = _page_data.begin(); pi != _page_data.end(); ++pi) {
      int collector_index = (*pi).first;
      if (client_data->has_collector(collector_index)) {
        _labels.push_back(collector_index);
      }
    }

    SortCollectorLabels1 sort_labels(client_data);
    sort(_labels.begin(), _labels.end(), sort_labels);

    _labels_changed = true;
  }

  // Finally, make sure all of the bars are closed.
  double time = frame_data.get_end();
  PageData::iterator pi;
  for (pi = _page_data.begin(); pi != _page_data.end(); ++pi) {
    (*pi).second.finish(time);
  }
}
