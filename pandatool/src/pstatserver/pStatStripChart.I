/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pStatStripChart.I
 * @author drose
 * @date 2000-07-15
 */

/**
 * Returns the View this chart represents.
 */
INLINE PStatView &PStatStripChart::
get_view() const {
  return _view;
}

/**
 * Returns the particular collector whose data this strip chart reflects.
 */
INLINE int PStatStripChart::
get_collector_index() const {
  return _collector_index;
}

/**
 * Changes the amount of time the width of the horizontal axis represents.
 * This may force a redraw.
 */
INLINE void PStatStripChart::
set_horizontal_scale(double time_width) {
  if (_time_width != time_width) {
    if (_scroll_mode) {
      _start_time += _time_width - time_width;
    } else {
      force_reset();
    }
    _time_width = time_width;
  }
}

/**
 * Returns the amount of total time the width of the horizontal axis
 * represents.
 */
INLINE double PStatStripChart::
get_horizontal_scale() const {
  return _time_width;
}

/**
 * Changes the value the height of the vertical axis represents.  This may
 * force a redraw.
 */
INLINE void PStatStripChart::
set_vertical_scale(double value_height) {
  if (_value_height != value_height) {
    _value_height = value_height;
    normal_guide_bars();
    force_redraw();
  }
}

/**
 * Returns total value the height of the vertical axis represents.
 */
INLINE double PStatStripChart::
get_vertical_scale() const {
  return _value_height;
}

/**
 * Changes the scroll_mode flag.  When true, the strip chart will update
 * itself by scrolling to the left; when false, the strip chart will wrap
 * around at the right and restart at the left end without scrolling.
 */
INLINE void PStatStripChart::
set_scroll_mode(bool scroll_mode) {
  if (_scroll_mode != scroll_mode) {
    _scroll_mode = scroll_mode;
    _first_data = true;
  }
}

/**
 * Returns the current state of the scroll_mode flag.  When true, the strip
 * chart will update itself by scrolling to the left; when false, the strip
 * chart will wrap around at the right and restart at the left end without
 * scrolling.
 */
INLINE bool PStatStripChart::
get_scroll_mode() const {
  return _scroll_mode;
}

/**
 * Changes the average_mode flag.  When true, the strip chart will average out
 * the color values over pstats_average_time seconds, which hides spikes and
 * makes the overall trends easier to read.  When false, the strip chart shows
 * the actual data as it is happening.
 */
INLINE void PStatStripChart::
set_average_mode(bool average_mode) {
  if (_average_mode != average_mode) {
    _average_mode = average_mode;
    force_redraw();
  }
}

/**
 * Returns the current state of the average_mode flag.  When true, the strip
 * chart will average out the color values over pstats_average_time seconds,
 * which hides spikes and makes the overall trends easier to read.  When
 * false, the strip chart shows the actual data as it is happening.
 */
INLINE bool PStatStripChart::
get_average_mode() const {
  return _average_mode;
}

/**
 * Converts a timestamp to a horizontal pixel offset.
 */
INLINE int PStatStripChart::
timestamp_to_pixel(double time) const {
  return (int)((double)get_xsize() * (time - _start_time) / _time_width);
}

/**
 * Converts a horizontal pixel offset to a timestamp.
 */
INLINE double PStatStripChart::
pixel_to_timestamp(int x) const {
  return _time_width * (double)x / (double)get_xsize() + _start_time;
}

/**
 * Converts a value (i.e.  a "height" in the strip chart) to a vertical pixel
 * offset.
 */
INLINE int PStatStripChart::
height_to_pixel(double value) const {
  return get_ysize() - (int)((double)get_ysize() * value / _value_height);
}

/**
 * Converts a vertical pixel offset to a value (a "height" in the strip
 * chart).
 */
INLINE double PStatStripChart::
pixel_to_height(int x) const {
  return _value_height * (double)(get_ysize() - x) / (double)get_ysize();
}

/**
 * Returns true if the indicated collector appears anywhere on the chart at
 * the current time, false otherwise.
 */
INLINE bool PStatStripChart::
is_label_used(int collector_index) const {
  if (collector_index < (int)_label_usage.size()) {
    return _label_usage[collector_index] > 0;
  }
  return false;
}
