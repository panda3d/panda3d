/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file winStatsMonitor.I
 * @author drose
 * @date 2004-01-09
 */

/**
 *
 */
INLINE WinStatsMonitor::MenuDef::
MenuDef(int thread_index, int collector_index, ChartType chart_type, bool show_level) :
  _thread_index(thread_index),
  _collector_index(collector_index),
  _chart_type(chart_type),
  _show_level(show_level)
{
}

/**
 *
 */
INLINE bool WinStatsMonitor::MenuDef::
operator < (const MenuDef &other) const {
  if (_thread_index != other._thread_index) {
    return _thread_index < other._thread_index;
  }
  if (_collector_index != other._collector_index) {
    return _collector_index < other._collector_index;
  }
  if (_chart_type != other._chart_type) {
    return _chart_type < other._chart_type;
  }
  return (int)_show_level < (int)other._show_level;
}
