/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pStatTimeline.I
 * @author rdb
 * @date 2022-02-11
 */

/**
 * Changes the amount of time the width of the horizontal axis represents.
 * This may force a redraw.
 */
INLINE void PStatTimeline::
set_horizontal_scale(double time_width) {
  double max_time_width = (_highest_end_time - _lowest_start_time) * 2.0;
  time_width = (std::min)(time_width, max_time_width);

  double scale = time_width / get_xsize();
  if (_time_scale != scale) {
    _time_scale = scale;
    _target_time_scale = scale;
    _zoom_speed = 0.0;
    normal_guide_bars();
    force_redraw();
  }
}

/**
 * Returns the amount of total time the width of the horizontal axis
 * represents.
 */
INLINE double PStatTimeline::
get_horizontal_scale() const {
  return _time_scale * get_xsize();
}

/**
 * This may force a redraw.
 */
INLINE void PStatTimeline::
set_horizontal_scroll(double start_time) {
  start_time = (std::max)((std::min)(start_time, _highest_end_time), _lowest_start_time);
  if (_start_time != start_time) {
    _start_time = start_time;
    _target_start_time = start_time;
    _scroll_speed = 0.0;
    normal_guide_bars();
    force_redraw();
  }
}

/**
 * Returns the amount of total time the width of the horizontal axis
 * represents.
 */
INLINE double PStatTimeline::
get_horizontal_scroll() const {
  return _start_time;
}

/**
 * Smoothly zooms to the given time width, around the given focal point.
 */
INLINE void PStatTimeline::
zoom_to(double time_width, double center) {
  // Don't allow zooming out to beyond 2x the size of the entire timeline.
  // There's a limit of zooming beyond 1 ns per bar, there's just no point...
  double max_time_width = (_highest_end_time - _lowest_start_time) * 2.0;
  time_width = (std::min)((std::max)(1e-7, time_width), max_time_width);
  _target_time_scale = time_width / get_xsize();
  _zoom_center = center;

  double pivot_x = (_zoom_center - _start_time) / _time_scale;
  scroll_to(_zoom_center - pivot_x * _target_time_scale);
}

/**
 * Smoothly zooms by the given amount, where 1.0 is a single "tick" of zooming
 * in and -1.0 is a single "tick" of zooming out.
 */
INLINE void PStatTimeline::
zoom_by(double amount, double center) {
  zoom_to(_target_time_scale * pow(0.8, amount) * get_xsize(), center);
}

/**
 * Smoothly scrolls to the given time point.
 */
INLINE void PStatTimeline::
scroll_to(double start_time) {
  _target_start_time = (std::max)((std::min)(start_time, _highest_end_time), _lowest_start_time);
}

/**
 * Smoothly scrolls by the given amount.
 */
INLINE void PStatTimeline::
scroll_by(double delta) {
  scroll_to(_target_start_time + delta);
}

/**
 * Converts a timestamp to a horizontal pixel offset.
 */
INLINE int PStatTimeline::
timestamp_to_pixel(double time) const {
  return (int)((double)_xsize * (time - _start_time) / get_horizontal_scale());
}

/**
 * Converts a horizontal pixel offset to a timestamp.
 */
INLINE double PStatTimeline::
pixel_to_timestamp(int x) const {
  return _time_scale * (double)x + _start_time;
}

/**
 * Converts a value (i.e.  a "height" in the strip chart) to a horizontal
 * pixel offset.
 */
INLINE int PStatTimeline::
height_to_pixel(double value) const {
  return (int)((double)_xsize * value / get_horizontal_scale());
}

/**
 * Converts a horizontal pixel offset to a value (a "height" in the strip
 * chart).
 */
INLINE double PStatTimeline::
pixel_to_height(int x) const {
  return _time_scale * (double)x;
}

/**
 * Returns the total number of rows.
 */
INLINE int PStatTimeline::
get_num_rows() const {
  if (_threads.empty()) {
    return 0;
  } else {
    return _threads.back()._row_offset + _threads.back()._rows.size();
  }
}
