/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file gtkStatsMonitor.I
 * @author drose
 * @date 2006-01-16
 */

/**
 *
 */
INLINE GtkStatsMonitor::MenuDef::
MenuDef(ChartType chart_type, int thread_index, int collector_index,
        int frame_number, bool show_level) :
  _chart_type(chart_type),
  _thread_index(thread_index),
  _collector_index(collector_index),
  _frame_number(frame_number),
  _show_level(show_level),
  _monitor(nullptr)
{
}

/**
 *
 */
INLINE bool GtkStatsMonitor::MenuDef::
operator < (const MenuDef &other) const {
  if (_chart_type != other._chart_type) {
    return _chart_type < other._chart_type;
  }
  if (_thread_index != other._thread_index) {
    return _thread_index < other._thread_index;
  }
  if (_collector_index != other._collector_index) {
    return _collector_index < other._collector_index;
  }
  if (_frame_number != other._frame_number) {
    return _frame_number < other._frame_number;
  }
  return (int)_show_level < (int)other._show_level;
}
