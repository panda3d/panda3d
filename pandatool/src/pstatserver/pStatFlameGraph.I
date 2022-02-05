/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pStatFlameGraph.I
 * @author rdb
 * @date 2022-01-28
 */

/**
 * Returns the View this chart represents.
 */
INLINE PStatView &PStatFlameGraph::
get_view() const {
  return _view;
}

/**
 * Returns the particular collector whose data this strip chart reflects.
 */
INLINE int PStatFlameGraph::
get_collector_index() const {
  return _collector_index;
}

/**
 * Returns the amount of total time the width of the horizontal axis
 * represents.
 */
INLINE double PStatFlameGraph::
get_horizontal_scale() const {
  return _time_width;
}

/**
 * Changes the average_mode flag.  When true, the strip chart will average out
 * the color values over pstats_average_time seconds, which hides spikes and
 * makes the overall trends easier to read.  When false, the strip chart shows
 * the actual data as it is happening.
 */
INLINE void PStatFlameGraph::
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
INLINE bool PStatFlameGraph::
get_average_mode() const {
  return _average_mode;
}

/**
 * Converts a value (i.e.  a "height" in the strip chart) to a horizontal
 * pixel offset.
 */
INLINE int PStatFlameGraph::
height_to_pixel(double value) const {
  return (int)((double)_xsize * value / _time_width);
}

/**
 * Converts a horizontal pixel offset to a value (a "height" in the strip
 * chart).
 */
INLINE double PStatFlameGraph::
pixel_to_height(int x) const {
  return _time_width * (double)x / (double)_xsize;
}

/**
 * Returns true if get_title_text() has never yet returned an answer, false if
 * it has.
 */
INLINE bool PStatFlameGraph::
is_title_unknown() const {
  return _title_unknown;
}
