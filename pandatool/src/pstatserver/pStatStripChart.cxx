/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pStatStripChart.cxx
 * @author drose
 * @date 2000-07-15
 */

#include "pStatStripChart.h"
#include "pStatClientData.h"
#include "pStatMonitor.h"

#include "pStatFrameData.h"
#include "pStatCollectorDef.h"
#include "string_utils.h"
#include "config_pstatclient.h"

#include <algorithm>

using std::max;
using std::min;

/**
 *
 */
PStatStripChart::
PStatStripChart(PStatMonitor *monitor, PStatView &view,
                int thread_index, int collector_index, int xsize, int ysize) :
  PStatGraph(monitor, xsize, ysize),
  _thread_index(thread_index),
  _view(view),
  _collector_index(collector_index)
{
  _scroll_mode = pstats_scroll_mode;
  _average_mode = false;

  _next_frame = 0;
  _first_data = true;
  _cursor_pixel = 0;

  _time_width = 20.0;
  _value_height = 1.0/10.0;
  _start_time = 0.0;

  _level_index = -1;
  _title_unknown = true;

  const PStatClientData *client_data = _monitor->get_client_data();
  if (client_data->has_collector(_collector_index)) {
    const PStatCollectorDef &def = client_data->get_collector_def(_collector_index);
    _unit_name = def._level_units;
  }

  set_default_vertical_scale();
}

/**
 *
 */
PStatStripChart::
~PStatStripChart() {
}

/**
 * Indicates that new data has become available.
 */
void PStatStripChart::
new_data(int frame_number) {
  // If the new frame is older than the last one we've drawn, we'll need to
  // back up and redraw it.  This can happen when frames arrive out of order
  // from the client.
  _next_frame = min(frame_number, _next_frame);
}

/**
 * Updates the chart with the latest data.
 */
void PStatStripChart::
update() {
  const PStatClientData *client_data = get_monitor()->get_client_data();

  // Don't bother to update the thread data until we know at least something
  // about the collectors and threads.
  if (client_data->get_num_collectors() != 0 &&
      client_data->get_num_threads() != 0) {
    const PStatThreadData *thread_data = _view.get_thread_data();
    if (!thread_data->is_empty()) {
      int latest = thread_data->get_latest_frame_number();

      if (latest > _next_frame) {
        draw_frames(_next_frame, latest);
      }
      _next_frame = latest;

      // Clean out the old data.
      double oldest_time =
        thread_data->get_frame(latest).get_start() - _time_width;

      Data::iterator di;
      di = _data.begin();
      while (di != _data.end() &&
             thread_data->get_frame((*di).first).get_start() < oldest_time) {
        dec_label_usage((*di).second);
        _data.erase(di);
        di = _data.begin();
      }
    }
  }

  if (_level_index != _view.get_level_index()) {
    update_labels();
  }

  idle();
}

/**
 * Returns true if the chart has seen its first data appear on it, false if it
 * is still a virgin chart.
 */
bool PStatStripChart::
first_data() const {
  return _first_data;
}

/**
 * Changes the collector represented by this strip chart.  This may force a
 * redraw.
 */
void PStatStripChart::
set_collector_index(int collector_index) {
  if (_collector_index != collector_index) {
    _collector_index = collector_index;
    _title_unknown = true;
    _data.clear();
    clear_label_usage();
    force_redraw();
    update_labels();
  }
}

/**
 * Sets the vertical scale according to the suggested scale of the base
 * collector, if any, or to center the target frame rate bar otherwise.
 */
void PStatStripChart::
set_default_vertical_scale() {
  const PStatClientData *client_data = _monitor->get_client_data();
  if (client_data->has_collector(_collector_index)) {
    const PStatCollectorDef &def =
      client_data->get_collector_def(_collector_index);
    if (def._suggested_scale != 0.0) {
      set_vertical_scale(def._suggested_scale);
      return;
    }
  }

  set_vertical_scale(2.0 / get_target_frame_rate());
}

/**
 * Sets the vertical scale to make all the data visible.
 */
void PStatStripChart::
set_auto_vertical_scale() {
  const PStatThreadData *thread_data = _view.get_thread_data();

  double max_value = 0.0;

  int frame_number = -1;
  for (int x = 0; x <= _xsize; x++) {
    double time = pixel_to_timestamp(x);
    frame_number =
      thread_data->get_frame_number_at_time(time, frame_number);

    if (thread_data->has_frame(frame_number)) {
      double net_value = get_net_value(frame_number);
      max_value = max(max_value, net_value);
    }
  }

  // Ok, now we know what the max value visible in the chart is.  Choose a
  // scale that will show all of this sensibly.
  if (max_value == 0.0) {
    set_vertical_scale(1.0);
  } else {
    set_vertical_scale(max_value * 1.1);
  }
}

/**
 * Return the collector index associated with the particular band of color at
 * the indicated pixel location, or -1 if no band of color was at the pixel.
 */
int PStatStripChart::
get_collector_under_pixel(int xpoint, int ypoint) {
  // First, we need to know what frame it was; to know that, we need to
  // determine the time corresponding to the x pixel.
  double time = pixel_to_timestamp(xpoint);

  // Now use that time to determine the frame.
  const PStatThreadData *thread_data = _view.get_thread_data();

  // And now we can determine which collector within the frame, based on the
  // value height.
  if (_average_mode) {
    double start_time = pixel_to_timestamp(xpoint);
    int then_i = thread_data->get_frame_number_at_time(start_time - pstats_average_time);
    int now_i = thread_data->get_frame_number_at_time(start_time, then_i);

    FrameData fdata;
    compute_average_pixel_data(fdata, then_i, now_i, start_time);
    double overall_value = 0.0;
    int y = get_ysize();

    FrameData::const_iterator fi;
    for (fi = fdata.begin(); fi != fdata.end(); ++fi) {
      const ColorData &cd = (*fi);
      overall_value += cd._net_value;
      y = height_to_pixel(overall_value);
      if (y <= ypoint) {
        return cd._collector_index;
      }
    }

  } else {
    int frame_number = thread_data->get_frame_number_at_time(time);
    const FrameData &fdata = get_frame_data(frame_number);
    double overall_value = 0.0;
    int y = get_ysize();

    FrameData::const_iterator fi;
    for (fi = fdata.begin(); fi != fdata.end(); ++fi) {
      const ColorData &cd = (*fi);
      overall_value += cd._net_value;
      y = height_to_pixel(overall_value);
      if (y <= ypoint) {
        return cd._collector_index;
      }
    }
  }

  return -1;
}

/**
 * Returns the text suitable for the title label on the top line.
 */
std::string PStatStripChart::
get_title_text() {
  std::string text;

  _title_unknown = false;

  const PStatClientData *client_data = _monitor->get_client_data();
  if (client_data->has_collector(_collector_index)) {
    text = client_data->get_collector_fullname(_collector_index);
    const PStatCollectorDef &def = client_data->get_collector_def(_collector_index);
    if (_view.get_show_level()) {
      if (!def._level_units.empty()) {
        text += " (" + def._level_units + ")";
      }
    } else {
      text += " time";
    }
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
 * Returns true if get_title_text() has never yet returned an answer, false if
 * it has.
 */
bool PStatStripChart::
is_title_unknown() const {
  return _title_unknown;
}

/**
 * Adds the data from additional into the data from fdata, after applying the
 * scale weight.
 */
void PStatStripChart::
accumulate_frame_data(FrameData &fdata, const FrameData &additional,
                      double weight) {
  FrameData::iterator ai;
  FrameData::const_iterator bi;

  ai = fdata.begin();
  bi = additional.begin();

  FrameData result;

  if (fdata.size() == additional.size()) {
    // Start out assuming that fdata and additional contain exactly the same
    // set of collectors.  If we discover otherwise, we'll have to bail at
    // that point.
    while (ai != fdata.end() &&
           (*ai)._collector_index == (*bi)._collector_index) {
      (*ai)._net_value += ((*bi)._net_value * weight);
      ++ai;
      ++bi;
    }

    if (ai == fdata.end()) {
      // If we successfully reached the end of the list, great!  We're done
      // without any merging.
      return;
    }

    // Otherwise, the two lists weren't identical.  In that case, copy the
    // accumulated data so far and continue from this point with the full-
    // blown merge.
    result.reserve(max(fdata.size(), additional.size()));
    FrameData::const_iterator ci;
    for (ci = fdata.begin(); ci != ai; ++ci) {
      result.push_back(*ci);
    }

  } else {
    // If the two lists had different lengths, clearly they aren't identical.
    result.reserve(max(fdata.size(), additional.size()));
  }

  while (ai != fdata.end() && bi != additional.end()) {
    if ((*ai)._i < (*bi)._i) {
      // Here's a data value that's in data, but not in additional.
      result.push_back(*ai);
      ++ai;

    } else if ((*bi)._i < (*ai)._i) {
      // Here's a data value that's in additional, but not in data.
      ColorData scaled;
      scaled._collector_index = (*bi)._collector_index;
      scaled._i = (*bi)._i;
      scaled._net_value = (*bi)._net_value * weight;
      result.push_back(scaled);
      ++bi;

    } else {
      // Here's a data value that's in both.
      ColorData combined;
      combined._collector_index = (*ai)._collector_index;
      combined._i = (*bi)._i;
      combined._net_value = (*ai)._net_value + (*bi)._net_value * weight;
      result.push_back(combined);
      ++ai;
      ++bi;
    }
  }

  while (ai != fdata.end()) {
    // Here's a data value that's in data, but not in additional.
    result.push_back(*ai);
    ++ai;
  }

  while (bi != additional.end()) {
    // Here's a data value that's in additional, but not in data.
    ColorData scaled;
    scaled._collector_index = (*bi)._collector_index;
    scaled._i = (*bi)._i;
    scaled._net_value = (*bi)._net_value * weight;
    result.push_back(scaled);
    ++bi;
  }

  fdata.swap(result);
}

/**
 * Applies the indicated scale to all collector values in data.
 */
void PStatStripChart::
scale_frame_data(FrameData &fdata, double factor) {
  FrameData::iterator fi;
  for (fi = fdata.begin(); fi != fdata.end(); ++fi) {
    (*fi)._net_value *= factor;
  }
}


/**
 * Returns the cached FrameData associated with the given frame number.  This
 * describes the lengths of the color bands for a single vertical stripe in
 * the chart.
 */
const PStatStripChart::FrameData &PStatStripChart::
get_frame_data(int frame_number) {
  Data::const_iterator di;
  di = _data.find(frame_number);
  if (di != _data.end()) {
    return (*di).second;
  }

  const PStatThreadData *thread_data = _view.get_thread_data();
  _view.set_to_frame(thread_data->get_frame(frame_number));

  FrameData &fdata = _data[frame_number];

  const PStatViewLevel *level = _view.get_level(_collector_index);
  int num_children = level->get_num_children();
  for (int i = 0; i < num_children; i++) {
    const PStatViewLevel *child = level->get_child(i);
    ColorData cd;
    cd._collector_index = (unsigned short)child->get_collector();
    cd._i = (unsigned short)i;
    cd._net_value = child->get_net_value();
    if (cd._net_value != 0.0) {
      fdata.push_back(cd);
    }
  }

  // Also, there might be some value in the overall Collector that wasn't
  // included in all of the children.
  ColorData cd;
  cd._collector_index = (unsigned short)level->get_collector();
  cd._i = (unsigned short)num_children;
  cd._net_value = level->get_value_alone();
  if (cd._net_value > 0.0) {
    fdata.push_back(cd);
  }

  inc_label_usage(fdata);

  return fdata;
}

/**
 * Fills the indicated FrameData structure with the color data for the
 * indicated pixel, averaged over the past pstats_average_time seconds.
 *
 * now is the timestamp for which we are computing the data; then_i and now_i
 * are the frame numbers that bound (now - pstats_average_time) and now.  At
 * function initialization time, these should be at or below the actual
 * values; they will be incremented as needed by this function.  This allows
 * the function to be called repeatedly for successive pixels.
 */
void PStatStripChart::
compute_average_pixel_data(PStatStripChart::FrameData &result,
                           int &then_i, int &now_i, double now) {
  result.clear();

  const PStatThreadData *thread_data = _view.get_thread_data();
  if (thread_data->is_empty() || thread_data->get_oldest_time() > now) {
    // No data.
    return;
  }

  double then = now - pstats_average_time;

  int latest_frame = thread_data->get_latest_frame_number();
  while (then_i <= latest_frame &&
         thread_data->get_frame(then_i).get_end() < then) {
    then_i++;
  }
  while (now_i <= latest_frame &&
         thread_data->get_frame(now_i).get_end() < now) {
    now_i++;
  }

  then = max(then, thread_data->get_frame(then_i).get_start());

  // Sum up a weighted average of all of the individual frames we pass.

  // We start with just the portion of frame then_i that actually does fall
  // within our "then to now" window.
  accumulate_frame_data(result, get_frame_data(then_i),
                        thread_data->get_frame(then_i).get_end() - then);
  double last = thread_data->get_frame(then_i).get_end();

  // Then we get all of each of the middle frames.
  for (int frame_number = then_i + 1;
       frame_number < now_i;
       frame_number++) {
    accumulate_frame_data(result, get_frame_data(frame_number),
                          thread_data->get_frame(frame_number).get_end() - last);
    last = thread_data->get_frame(frame_number).get_end();
  }

  // And finally, we get the remainder as now_i.
  if (last <= now) {
    accumulate_frame_data(result, get_frame_data(now_i), now - last);
  }

  scale_frame_data(result, 1.0f / (now - then));
}

/**
 * Returns the net value of the chart's collector for the indicated frame
 * number.
 */
double PStatStripChart::
get_net_value(int frame_number) const {
  const FrameData &frame =
    ((PStatStripChart *)this)->get_frame_data(frame_number);

  double net_value = 0.0;
  FrameData::const_iterator fi;
  for (fi = frame.begin(); fi != frame.end(); ++fi) {
    const ColorData &cd = (*fi);
    net_value += cd._net_value;
  }

  return net_value;
}

/**
 * Computes the average value of the chart's collector over the past
 * pstats_average_time number of seconds.
 */
double PStatStripChart::
get_average_net_value() const {
  const PStatThreadData *thread_data = _view.get_thread_data();
  int now_i, then_i;
  if (!thread_data->get_elapsed_frames(then_i, now_i)) {
    return 0.0f;
  }
  double now = _time_width + _start_time;
  double then = now - pstats_average_time;

  int num_frames = now_i - then_i + 1;

  if (_collector_index == 0 && !_view.get_show_level()) {
    // If we're showing the time for the whole frame, compute this from the
    // total elapsed time, rather than summing up individual frames.  This is
    // more accurate and exactly matches what is reported by
    // thread_data->get_frame_rate().

    const PStatFrameData &now_frame_data = thread_data->get_frame(now_i);
    const PStatFrameData &then_frame_data = thread_data->get_frame(then_i);
    double now = now_frame_data.get_end();
    double elapsed_time = (now - then_frame_data.get_start());
    return elapsed_time / (double)num_frames;

  } else {
    // On the other hand, if we're showing the time for some sub-frame, we
    // have to do it the less-accurate way of summing up individual frames,
    // which might introduce errors if we are missing data for some frames,
    // but what can you do?

    const PStatThreadData *thread_data = _view.get_thread_data();

    double net_value = 0.0f;
    double net_time = 0.0f;

    // We start with just the portion of frame then_i that actually does fall
    // within our "then to now" window (usually some portion of it will).
    if (thread_data->get_frame(then_i).get_end() > then) {
      double this_time = (thread_data->get_frame(then_i).get_end() - then);
      net_value += get_net_value(then_i) * this_time;
      net_time += this_time;
    }
    // Then we get all of each of the remaining frames.
    for (int frame_number = then_i + 1;
         frame_number <= now_i;
         frame_number++) {
      double this_time = thread_data->get_frame(frame_number).get_net_time();
      net_value += get_net_value(frame_number) * this_time;
      net_time += this_time;
    }

    return net_value / net_time;
  }
}

/**
 * To be called by the user class when the widget size has changed.  This
 * updates the chart's internal data and causes it to issue redraw commands to
 * reflect the new size.
 */
void PStatStripChart::
changed_size(int xsize, int ysize) {
  if (xsize != _xsize || ysize != _ysize) {
    _xsize = xsize;
    _ysize = ysize;
    if (_xsize > 0 && _ysize > 0) {
      _cursor_pixel = xsize * _cursor_pixel / _xsize;

      if (!_first_data) {
        if (_scroll_mode) {
          draw_pixels(0, _xsize);

        } else {
          // Redraw the stats that were there before.
          double old_start_time = _start_time;

          // Back up a bit to draw the stuff to the right of the cursor.
          _start_time -= _time_width;
          draw_pixels(_cursor_pixel, _xsize);

          // Now draw the stuff to the left of the cursor.
          _start_time = old_start_time;
          draw_pixels(0, _cursor_pixel);
        }
      }
    }
  }
}

/**
 * To be called by the user class when the whole thing needs to be redrawn for
 * some reason.
 */
void PStatStripChart::
force_redraw() {
  if (!_first_data) {
    draw_pixels(0, _xsize);
  }
}

/**
 * To be called by the user class to cause the chart to reset to empty and
 * start filling again.
 */
void PStatStripChart::
force_reset() {
  clear_region();
  _first_data = true;
}


/**
 * Should be overridden by the user class to wipe out the entire strip chart
 * region.
 */
void PStatStripChart::
clear_region() {
}

/**
 * Should be overridden by the user class to copy a region of the chart from
 * one part of the chart to another.  This is used to implement scrolling.
 */
void PStatStripChart::
copy_region(int, int, int) {
}

/**
 * Should be overridden by the user class.  This hook will be called before
 * drawing any color bars in the strip chart; it gives the pixel range that's
 * about to be redrawn.
 */
void PStatStripChart::
begin_draw(int, int) {
}

/**
 * Should be overridden by the user class to draw a single vertical slice in
 * the strip chart at the indicated pixel, with the data for the indicated
 * frame.
 */
void PStatStripChart::
draw_slice(int, int, const PStatStripChart::FrameData &fdata) {
}

/**
 * This is similar to draw_slice(), except it should draw a vertical line of
 * the background color to represent a portion of the chart that has no data.
 */
void PStatStripChart::
draw_empty(int, int) {
}

/**
 * This is similar to draw_slice(), except that it should draw the black
 * vertical stripe that represents the current position when not in scrolling
 * mode.
 */
void PStatStripChart::
draw_cursor(int) {
}

/**
 * Should be overridden by the user class.  This hook will be called after
 * drawing a series of color bars in the strip chart; it gives the pixel range
 * that was just redrawn.
 */
void PStatStripChart::
end_draw(int, int) {
}

/**
 * Should be overridden by the user class to perform any other updates might
 * be necessary after the color bars have been redrawn.  For instance, it
 * could check the state of _labels_changed, and redraw the labels if it is
 * true.
 */
void PStatStripChart::
idle() {
}


// STL function object for sorting labels in order by the collector's sort
// index, used in update_labels(), below.
class SortCollectorLabels2 {
public:
  SortCollectorLabels2(const PStatClientData *client_data) :
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
 * Resets the list of labels.
 */
void PStatStripChart::
update_labels() {
  const PStatViewLevel *level = _view.get_level(_collector_index);
  _labels.clear();

  int num_children = level->get_num_children();
  for (int i = 0; i < num_children; i++) {
    const PStatViewLevel *child = level->get_child(i);
    int collector_index = child->get_collector();
    if (is_label_used(collector_index)) {
      _labels.push_back(collector_index);
    }
  }

  SortCollectorLabels2 sort_labels(get_monitor()->get_client_data());
  sort(_labels.begin(), _labels.end(), sort_labels);

  int collector_index = level->get_collector();
  _labels.push_back(collector_index);

  _labels_changed = true;
  _level_index = _view.get_level_index();
}

/**
 * Calls update_guide_bars with parameters suitable to this kind of graph.
 */
void PStatStripChart::
normal_guide_bars() {
  update_guide_bars(4, _value_height);
}


/**
 * Draws the levels for the indicated frame range.
 */
void PStatStripChart::
draw_frames(int first_frame, int last_frame) {
  const PStatThreadData *thread_data = _view.get_thread_data();

  last_frame = min(last_frame, thread_data->get_latest_frame_number());

  if (_first_data) {
    if (_scroll_mode) {
      _start_time =
        thread_data->get_frame(last_frame).get_start() - _time_width;
    } else {
      _start_time = thread_data->get_frame(first_frame).get_start();
      _cursor_pixel = 0;
    }
  }

  int first_pixel;
  if (thread_data->has_frame(first_frame)) {
    first_pixel =
      timestamp_to_pixel(thread_data->get_frame(first_frame).get_start());
  } else {
    first_pixel = 0;
  }

  int last_pixel =
    timestamp_to_pixel(thread_data->get_frame(last_frame).get_start());

  if (_first_data && !_scroll_mode) {
    first_pixel = min(_cursor_pixel, first_pixel);
  }
  _first_data = false;

  if (last_pixel - first_pixel >= _xsize) {
    // If we're drawing the whole thing all in this one swoop, just start
    // over.
    _start_time = thread_data->get_frame(last_frame).get_start() - _time_width;
    first_pixel = 0;
    last_pixel = _xsize;
  }

  if (last_pixel <= _xsize) {
    // It all fits in one block.
    _cursor_pixel = last_pixel;
    draw_pixels(first_pixel, last_pixel);

  } else {
    if (_scroll_mode) {
      // In scrolling mode, slide the world back.
      int slide_pixels = last_pixel - _xsize;
      copy_region(slide_pixels, first_pixel, 0);
      first_pixel -= slide_pixels;
      last_pixel -= slide_pixels;
      _start_time += (double)slide_pixels / (double)_xsize * _time_width;
      draw_pixels(first_pixel, last_pixel);

    } else {
      // In wrapping mode, do it in two blocks.
      _cursor_pixel = -1;
      draw_pixels(first_pixel, _xsize);
      _start_time = pixel_to_timestamp(_xsize);
      last_pixel -= _xsize;
      _cursor_pixel = last_pixel;
      draw_pixels(0, last_pixel);
    }
  }
}

/**
 * Draws the levels for the indicated pixel range.
 */
void PStatStripChart::
draw_pixels(int first_pixel, int last_pixel) {
  begin_draw(first_pixel, last_pixel);
  const PStatThreadData *thread_data = _view.get_thread_data();

  if (_average_mode && !thread_data->is_empty()) {
    // In average mode, we have to calculate the average value for each pixel.
    double start_time = pixel_to_timestamp(first_pixel);
    int then_i = thread_data->get_frame_number_at_time(start_time - pstats_average_time);
    int now_i = thread_data->get_frame_number_at_time(start_time, then_i);
    for (int x = first_pixel; x <= last_pixel; x++) {
      if (x == _cursor_pixel && !_scroll_mode) {
        draw_cursor(x);
      } else {
        FrameData fdata;
        compute_average_pixel_data(fdata, then_i, now_i, pixel_to_timestamp(x));
        draw_slice(x, 1, fdata);
      }
    }

  } else {
    // When average mode is false, we are in frame mode; just show the actual
    // frame data.
    int frame_number = -1;
    int x = first_pixel;
    while (x <= last_pixel) {
      if (x == _cursor_pixel && !_scroll_mode) {
        draw_cursor(x);
        x++;

      } else {
        double time = pixel_to_timestamp(x);
        frame_number = thread_data->get_frame_number_at_time(time, frame_number);
        int w = 1;
        int stop_pixel = last_pixel;
        if (!_scroll_mode) {
          stop_pixel = min(stop_pixel, _cursor_pixel);
        }
        while (x + w < stop_pixel &&
               thread_data->get_frame_number_at_time(pixel_to_timestamp(x + w), frame_number) == frame_number) {
          w++;
        }
        if (thread_data->has_frame(frame_number)) {
          draw_slice(x, w, get_frame_data(frame_number));
        } else {
          draw_empty(x, w);
        }
        x += w;
      }
    }
  }

  end_draw(first_pixel, last_pixel);
}

/**
 * Erases all elements from the label usage data.
 */
void PStatStripChart::
clear_label_usage() {
  _label_usage.clear();
}

/**
 * Erases the indicated frame data from the current label usage.  This
 * indicates that the given FrameData has fallen off the end of the chart.
 * This must have been proceeded by an earlier call to inc_label_usage() for
 * the same FrameData
 */
void PStatStripChart::
dec_label_usage(const FrameData &fdata) {
  FrameData::const_iterator fi;
  for (fi = fdata.begin(); fi != fdata.end(); ++fi) {
    const ColorData &cd = (*fi);
    nassertv(cd._collector_index < (int)_label_usage.size());
    nassertv(_label_usage[cd._collector_index] > 0);
    _label_usage[cd._collector_index]--;
    if (_label_usage[cd._collector_index] == 0) {
      // If a label drops out of usage, it's time to regenerate labels.
      _level_index = -1;
    }
  }
}

/**
 * Records the labels named in the indicated FrameData in the table of current
 * labels in use.  This should be called when the given FrameData has been
 * added to the chart; it will increment the reference count for each
 * collector named in the FrameData.  The reference count will eventually be
 * decremented when dec_label_usage() is called later.
 */
void PStatStripChart::
inc_label_usage(const FrameData &fdata) {
  FrameData::const_iterator fi;
  for (fi = fdata.begin(); fi != fdata.end(); ++fi) {
    const ColorData &cd = (*fi);
    while (cd._collector_index >= (int)_label_usage.size()) {
      _label_usage.push_back(0);
    }
    nassertv(_label_usage[cd._collector_index] >= 0);
    _label_usage[cd._collector_index]++;
    if (_label_usage[cd._collector_index] == 1) {
      // If a label appears for the first time, it's time to regenerate
      // labels.
      _level_index = -1;
    }
  }
}
