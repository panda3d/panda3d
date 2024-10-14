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
PStatStripChart(PStatMonitor *monitor,
                int thread_index, int collector_index, bool show_level,
                int xsize, int ysize) :
  PStatGraph(monitor, xsize, ysize),
  _thread_index(thread_index),
  _view(show_level ? monitor->get_level_view(0, thread_index) : monitor->get_view(thread_index)),
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

  set_auto_vertical_scale();

  monitor->_strip_charts.insert(this);
}

/**
 *
 */
PStatStripChart::
~PStatStripChart() {
  _monitor->_strip_charts.erase(this);
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
    set_auto_vertical_scale();
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

  // Calculate the median value.
  std::vector<double> values;

  if (thread_data != nullptr && !thread_data->is_empty()) {
    // Find the oldest visible frame.
    double start_time = pixel_to_timestamp(0);
    int oldest_frame = std::max(
      thread_data->get_frame_number_at_time(start_time),
      thread_data->get_oldest_frame_number());
    int latest_frame = thread_data->get_latest_frame_number();

    for (int frame_number = oldest_frame; frame_number <= latest_frame; ++frame_number) {
      if (thread_data->has_frame(frame_number)) {
        const FrameData &frame = get_frame_data(frame_number);
        values.push_back(frame._net_value);
      }
    }
  }

  if (values.empty()) {
    set_default_vertical_scale();
    return;
  }

  double median;
  size_t half = values.size() / 2;
  if (values.size() % 2 == 0) {
    std::sort(values.begin(), values.end());
    median = (values[half] + values[half + 1]) / 2.0;
  } else {
    std::nth_element(values.begin(), values.begin() + half, values.end());
    median = values[half];
  }

  if (median > 0.0) {
    // Take 1.5 times the median value as the vertical scale.
    set_vertical_scale(median * 1.5);
  } else {
    set_default_vertical_scale();
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

  if (time < thread_data->get_oldest_time()) {
    return -1;
  }

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
 * Returns the text suitable for the total label above the graph.
 */
std::string PStatStripChart::
get_total_text() {
  std::string text = format_number(get_average_net_value(), get_guide_bar_units(), get_guide_bar_unit_name());
  if (get_collector_index() != 0 && !_view.get_show_level()) {
    const PStatViewLevel *level = _view.get_level(get_collector_index());
    if (level != nullptr && level->get_count() > 0) {
      text += " / " + format_string(level->get_count()) + "x";
    }
  }
  return text;
}

/**
 * Called when the mouse hovers over a label, and should return the text that
 * should appear on the tooltip.
 */
std::string PStatStripChart::
get_label_tooltip(int collector_index) const {
  const PStatClientData *client_data = _monitor->get_client_data();
  if (!client_data->has_collector(collector_index)) {
    return std::string();
  }

  const PStatThreadData *thread_data = _view.get_thread_data();

  std::ostringstream text;
  text << client_data->get_collector_fullname(collector_index);

  double value;
  if (collector_index == _collector_index) {
    value = get_average_net_value();
  }
  else {
    int now_i, then_i;
    if (!thread_data->get_elapsed_frames(then_i, now_i)) {
      return text.str();
    }
    double now = _time_width + _start_time;
    double then = now - pstats_average_time;

    double net_value = 0.0;
    double net_time = 0.0;

    // We start with just the portion of frame then_i that actually does fall
    // within our "then to now" window (usually some portion of it will).
    const PStatFrameData &frame_data = thread_data->get_frame(then_i);
    if (frame_data.get_end() > then) {
      double this_time = (frame_data.get_end() - then);
      for (const ColorData &cd : get_frame_data(then_i)) {
        if (cd._collector_index == collector_index) {
          net_value += cd._net_value * this_time;
          net_time += this_time;
          break;
        }
      }
    }
    // Then we get all of each of the remaining frames.
    for (int frame_number = then_i + 1;
         frame_number <= now_i;
         frame_number++) {
      const PStatFrameData &frame_data = thread_data->get_frame(frame_number);
      for (const ColorData &cd : get_frame_data(frame_number)) {
        if (cd._collector_index == collector_index) {
          double this_time = frame_data.get_net_time();
          net_value += cd._net_value * this_time;
          net_time += this_time;
          break;
        }
      }
    }

    if (net_time == 0) {
      return text.str();
    }
    value = net_value / net_time;
  }

  text << " (" << format_number(value, get_guide_bar_units(), get_guide_bar_unit_name());

  if (collector_index != 0) {
    const FrameData &frame = get_frame_data(thread_data->get_latest_frame_number());

    for (const ColorData &cd : frame) {
      if (cd._collector_index == collector_index) {
        if (cd._count > 0) {
          text << " / " << cd._count << "x";
        }
        break;
      }
    }
  }

  text << ")";
  return text.str();
}

/**
 * Writes the graph state to a datagram.
 */
void PStatStripChart::
write_datagram(Datagram &dg) const {
  dg.add_bool(_scroll_mode);
  dg.add_bool(_average_mode);
  dg.add_float64(_time_width);
  dg.add_float64(_start_time);
  dg.add_float64(_value_height);

  // Not really necessary, we reconstructed this from the client data.
  //for (const auto &item : _data) {
  //  dg.add_int32(item.first);
  //  dg.add_uint32(item.second.size());
  //
  //  for (const ColorData &cd : item.second) {
  //    dg.add_uint16(cd._collector_index);
  //    dg.add_uint16(cd._i);
  //    dg.add_float64(cd._net_value);
  //  }
  //}
  //dg.add_int32(-1);

  PStatGraph::write_datagram(dg);
}

/**
 * Restores the graph state from a datagram.
 */
void PStatStripChart::
read_datagram(DatagramIterator &scan) {
  _next_frame = 0;
  force_reset();

  _scroll_mode = scan.get_bool();
  _average_mode = scan.get_bool();
  _time_width = scan.get_float64();
  _start_time = scan.get_float64();
  _value_height = scan.get_float64();

  //int key;
  //while ((key = scan.get_int32()) != -1) {
  //  FrameData &fdata = _data[key];
  //  fdata.resize(scan.get_uint32());
  //
  //  for (ColorData &cd : fdata) {
  //    cd._collector_index = scan.get_uint16();
  //    cd._i = scan.get_uint16();
  //    cd._net_value = scan.get_float64();
  //  }
  //}

  PStatGraph::read_datagram(scan);

  normal_guide_bars();
  update();
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
      scaled._count = 0;
      scaled._net_value = (*bi)._net_value * weight;
      result.push_back(scaled);
      ++bi;

    } else {
      // Here's a data value that's in both.
      ColorData combined;
      combined._collector_index = (*ai)._collector_index;
      combined._i = (*bi)._i;
      combined._count = 0;
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
    scaled._count = 0;
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
get_frame_data(int frame_number) const {
  Data::const_iterator di;
  di = _data.find(frame_number);
  if (di != _data.end()) {
    return (*di).second;
  }

  const PStatThreadData *thread_data = _view.get_thread_data();
  //assert(thread_data->has_frame(frame_number));
  const PStatFrameData &frame_data = thread_data->get_frame(frame_number);
  _view.set_to_frame(frame_data);

  FrameData &fdata = _data[frame_number];
  fdata._start = frame_data.get_start();
  fdata._end = frame_data.get_end();
  fdata._net_time = frame_data.get_net_time();

  double net_value = 0.0;

  const PStatViewLevel *level = _view.get_level(_collector_index);
  int num_children = level->get_num_children();
  for (int i = 0; i < num_children; i++) {
    const PStatViewLevel *child = level->get_child(i);
    ColorData cd;
    cd._collector_index = (unsigned short)child->get_collector();
    cd._i = (unsigned short)i;
    cd._count = child->get_count();
    cd._net_value = child->get_net_value();
    if (cd._net_value != 0.0) {
      fdata.push_back(cd);
      net_value += cd._net_value;
    }
  }

  // Also, there might be some value in the overall Collector that wasn't
  // included in all of the children.
  ColorData cd;
  cd._collector_index = (unsigned short)level->get_collector();
  cd._i = (unsigned short)num_children;
  cd._count = level->get_count();
  cd._net_value = level->get_value_alone();
  if (cd._net_value > 0.0) {
    fdata.push_back(cd);
    net_value += cd._net_value;
  }

  fdata._net_value = net_value;

  ((PStatStripChart *)this)->inc_label_usage(fdata);

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
  then_i = thread_data->get_frame_number_after(then, then_i);
  now_i = thread_data->get_frame_number_after(now, now_i);

  const FrameData *fdata = &get_frame_data(then_i);
  then = max(then, fdata->_start);
  double then_end = fdata->_end;

  // Sum up a weighted average of all of the individual frames we pass.

  // We start with just the portion of frame then_i that actually does fall
  // within our "then to now" window.
  accumulate_frame_data(result, *fdata, then_end - then);
  double last = then_end;

  // Then we get all of each of the middle frames.
  double weight = 0.0;
  for (int frame_number = then_i + 1;
       frame_number < now_i;
       frame_number++) {
    if (thread_data->has_frame(frame_number)) {
      if (weight > 0.0) {
        accumulate_frame_data(result, *fdata, weight);
        weight = 0.0;
      }
      fdata = &get_frame_data(frame_number);
    }
    weight += fdata->_end - last;
    last = fdata->_end;
  }

  if (weight > 0.0) {
    accumulate_frame_data(result, *fdata, weight);
  }

  // And finally, we get the remainder as now_i.
  if (last <= now) {
    accumulate_frame_data(result, get_frame_data(now_i), now - last);
  }

  scale_frame_data(result, 1.0f / (now - then));
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
    return 0.0;
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

    double net_value = 0.0;
    double net_time = 0.0;

    // We start with just the portion of frame then_i that actually does fall
    // within our "then to now" window (usually some portion of it will).
    const PStatFrameData &frame_data = thread_data->get_frame(then_i);
    const FrameData *frame = &get_frame_data(then_i);
    if (frame_data.get_end() > then) {
      double this_time = (frame->_end - then);
      net_value += frame->_net_value * this_time;
      net_time += this_time;
    }
    // Then we get all of each of the remaining frames.
    for (int frame_number = then_i + 1;
         frame_number <= now_i;
         frame_number++) {
      if (thread_data->has_frame(frame_number)) {
        frame = &get_frame_data(frame_number);
      }
      double this_time = frame->_net_time;
      net_value += frame->_net_value * this_time;
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
      // This is really slow on macOS, just redraw instead
#ifdef __APPLE__
      draw_pixels(0, first_pixel - slide_pixels);
#else
      copy_region(slide_pixels, first_pixel, 0);
#endif
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
