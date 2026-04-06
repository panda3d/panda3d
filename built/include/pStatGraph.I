/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pStatGraph.I
 * @author drose
 * @date 2000-07-19
 */

/**
 * Returns the monitor associated with this chart.
 */
INLINE PStatMonitor *PStatGraph::
get_monitor() const {
  return _monitor;
}

/**
 * Returns the number of labels to be drawn for this chart.
 */
INLINE int PStatGraph::
get_num_labels() const {
  return _labels.size();
}

/**
 * Returns the collector index associated with the nth label.
 */
INLINE int PStatGraph::
get_label_collector(int n) const {
  nassertr(n >= 0 && n < (int)_labels.size(), 0);
  return _labels[n];
}

/**
 * Returns the text associated with the nth label.
 */
INLINE std::string PStatGraph::
get_label_name(int n) const {
  nassertr(n >= 0 && n < (int)_labels.size(), std::string());
  return _monitor->get_client_data()->get_collector_name(_labels[n]);
}

/**
 * Returns the color associated with the nth label.
 */
INLINE LRGBColor PStatGraph::
get_label_color(int n) const {
  nassertr(n >= 0 && n < (int)_labels.size(), LRGBColor(0.0, 0.0, 0.0));
  return _monitor->get_collector_color(_labels[n]);
}

/**
 * Sets the target frame rate of the application in Hz.  This only affects the
 * choice of initial scale and the placement of guide bars.
 */
INLINE void PStatGraph::
set_target_frame_rate(double frame_rate) {
  if (_target_frame_rate != frame_rate) {
    _target_frame_rate = frame_rate;
    normal_guide_bars();
  }
}

/**
 * Returns the indicated target frame rate in Hz.  See
 * set_target_frame_rate().
 */
INLINE double PStatGraph::
get_target_frame_rate() const {
  return _target_frame_rate;
}

/**
 * Returns the width of the chart in pixels.
 */
INLINE int PStatGraph::
get_xsize() const {
  return _xsize;
}

/**
 * Returns the height of the chart in pixels.
 */
INLINE int PStatGraph::
get_ysize() const {
  return _ysize;
}

/**
 * Sets the units that are displayed for the guide bar labels.  This may be a
 * union of one or more members of the GuideBarUnits enum.
 */
INLINE void PStatGraph::
set_guide_bar_units(int guide_bar_units) {
  if (_guide_bar_units != guide_bar_units) {
    _guide_bar_units = guide_bar_units;
    normal_guide_bars();
  }
}

/**
 * Returns the units that are displayed for the guide bar labels.  This may be
 * a union of one or more members of the GuideBarUnits enum.
 */
INLINE int PStatGraph::
get_guide_bar_units() const {
  return _guide_bar_units;
}

/**
 * Sets the name of the units to be used for the guide bars if the units type
 * is set to GBU_named | GBU_show_units.
 */
INLINE void PStatGraph::
set_guide_bar_unit_name(const std::string &unit_name) {
  _unit_name = unit_name;
}

/**
 * Returns the name of the units to be used for the guide bars if the units
 * type is set to GBU_named | GBU_show_units.
 */
INLINE const std::string &PStatGraph::
get_guide_bar_unit_name() const {
  return _unit_name;
}
