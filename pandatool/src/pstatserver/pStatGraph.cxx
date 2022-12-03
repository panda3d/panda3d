/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pStatGraph.cxx
 * @author drose
 * @date 2000-07-19
 */

#include "pStatGraph.h"
#include "pStatServer.h"
#include "pStatFrameData.h"
#include "pStatCollectorDef.h"
#include "string_utils.h"
#include "config_pstatclient.h"

#include <stdio.h>  // for sprintf

using std::string;

/**
 *
 */
PStatGraph::GuideBar::
GuideBar(double height, const string &label, PStatGraph::GuideBarStyle style) :
  _height(height),
  _label(label),
  _style(style)
{
}

/**
 *
 */
PStatGraph::GuideBar::
GuideBar(const PStatGraph::GuideBar &copy) :
  _height(copy._height),
  _label(copy._label),
  _style(copy._style)
{
}

/**
 *
 */
PStatGraph::
PStatGraph(PStatMonitor *monitor, int xsize, int ysize) :
  _monitor(monitor),
  _xsize(xsize),
  _ysize(ysize)
{
  _target_frame_rate = pstats_target_frame_rate;
  _labels_changed = false;
  _guide_bars_changed = false;
  _guide_bar_units = GBU_ms;
}

/**
 *
 */
PStatGraph::
~PStatGraph() {
}

/**
 * Returns the number of horizontal guide bars that should be drawn, based on
 * the indicated target frame rate.  Not all of these may be visible; some may
 * be off the top of the chart because of the vertical scale.
 */
int PStatGraph::
get_num_guide_bars() const {
  return _guide_bars.size();
}

/**
 * Returns the nth horizontal guide bar.  This should be drawn as a horizontal
 * line across the chart at the y pixel location determined by
 * height_to_pixel(bar._height).
 *
 * It is possible that this bar will be off the top of the chart.
 */
const PStatGraph::GuideBar &PStatGraph::
get_guide_bar(int n) const {
#ifndef NDEBUG
  static GuideBar bogus_bar(0.0, "bogus", GBS_normal);
  nassertr(n >= 0 && n < (int)_guide_bars.size(), bogus_bar);
#endif
  return _guide_bars[n];
}

/**
 * Returns the current number of user-defined guide bars.  Not all of these
 * may be visible.
 */
int PStatGraph::
get_num_user_guide_bars() const {
  return _monitor->get_server()->get_num_user_guide_bars();
}

/**
 * Returns the nth user-defined guide bar.
 */
PStatGraph::GuideBar PStatGraph::
get_user_guide_bar(int n) const {
  double height = _monitor->get_server()->get_user_guide_bar_height(n);
  return make_guide_bar(height, GBS_user);
}

/**
 * Adjusts the height of the nth user-defined guide bar.
 */
void PStatGraph::
move_user_guide_bar(int n, double height) {
  _monitor->get_server()->move_user_guide_bar(n, height);
}

/**
 * Creates a new user guide bar and returns its index number.
 */
int PStatGraph::
add_user_guide_bar(double height) {
  return _monitor->get_server()->add_user_guide_bar(height);
}

/**
 * Removes the user guide bar with the indicated index number.  All subsequent
 * index numbers are adjusted down one.
 */
void PStatGraph::
remove_user_guide_bar(int n) {
  _monitor->get_server()->remove_user_guide_bar(n);
}

/**
 * Returns the index number of the first user guide bar found whose height is
 * within the indicated range, or -1 if no user guide bars fall within the
 * range.
 */
int PStatGraph::
find_user_guide_bar(double from_height, double to_height) const {
  return _monitor->get_server()->find_user_guide_bar(from_height, to_height);
}


/**
 * Returns a string representing the value nicely formatted for its range.
 */
string PStatGraph::
format_number(double value) {
  char buffer[128];

  if (value < 0.01) {
    sprintf(buffer, "%0.4f", value);
  } else if (value < 0.1) {
    sprintf(buffer, "%0.3f", value);
  } else if (value < 1.0) {
    sprintf(buffer, "%0.2f", value);
  } else if (value < 10.0) {
    sprintf(buffer, "%0.1f", value);
  } else {
    sprintf(buffer, "%0.0f", value);
  }

  return buffer;
}

/**
 * Returns a string representing the value nicely formatted for its range,
 * including the units as indicated.
 */
string PStatGraph::
format_number(double value, int guide_bar_units, const string &unit_name) {
  string label;

  if ((guide_bar_units & GBU_named) != 0) {
    // Units are whatever is specified by unit_name, not a time unit at all.
    int int_value = (int)value;
    if ((double)int_value == value) {
      // Probably a counter or something, don't display .0 suffix.
      label = format_string(int_value);
    } else {
      label = format_number(value);
    }
    if ((guide_bar_units & GBU_show_units) != 0 && !unit_name.empty()) {
      label += " ";
      label += unit_name;
    }
  }
  else {
    // Units are either milliseconds or hz, or both.
    if ((guide_bar_units & GBU_ms) != 0) {
      if ((guide_bar_units & GBU_show_units) != 0 &&
          value > 0 && value < 0.000001) {
        double ns = value * 1000000000.0;
        label += format_number(ns);
        label += " ns";
      }
      else if ((guide_bar_units & GBU_show_units) != 0 &&
          value > 0 && value < 0.001) {
        double us = value * 1000000.0;
        label += format_number(us);
#ifdef _WIN32
        label += " \xb5s";
#else
        label += " \xc2\xb5s";
#endif
      }
      else if ((guide_bar_units & GBU_show_units) == 0 || value < 1.0) {
        double ms = value * 1000.0;
        label += format_number(ms);
        if ((guide_bar_units & GBU_show_units) != 0) {
          label += " ms";
        }
      }
      else {
        label += format_number(value);
        label += " s";
      }
    }

    if ((guide_bar_units & GBU_hz) != 0) {
      double hz = 1.0 / value;

      if ((guide_bar_units & GBU_show_units) != 0 &&
          (guide_bar_units & GBU_ms) == 0) {
        if (hz >= 1000000000) {
          label += format_number(hz / 1000000000);
          label += " GHz";
        }
        else if (hz >= 1000000) {
          label += format_number(hz / 1000000);
          label += " MHz";
        }
        else if (hz >= 1000) {
          label += format_number(hz / 1000);
          label += " kHz";
        }
        else {
          label += format_number(hz);
          label += " Hz";
        }
      }
      else {
        if ((guide_bar_units & GBU_ms) != 0) {
          label += " (";
        }
        label += format_number(hz);
        if ((guide_bar_units & GBU_show_units) != 0) {
          label += " Hz";
        }
        if ((guide_bar_units & GBU_ms) != 0) {
          label += ")";
        }
      }
    }
  }

  return label;
}

/**
 * Writes the graph state to a datagram.
 */
void PStatGraph::
write_datagram(Datagram &dg) const {
  int x, y, width, height;
  bool minimized, maximized;
  if (get_window_state(x, y, width, height, minimized, maximized)) {
    dg.add_bool(true);
    dg.add_int32(x);
    dg.add_int32(y);
    dg.add_int32(width);
    dg.add_int32(height);
    dg.add_bool(minimized);
    dg.add_bool(maximized);
  }
  else {
    dg.add_bool(false);
  }
}

/**
 * Restores the graph state from a datagram.
 */
void PStatGraph::
read_datagram(DatagramIterator &scan) {
  if (scan.get_bool()) {
    int x = scan.get_int32();
    int y = scan.get_int32();
    int width = scan.get_int32();
    int height = scan.get_int32();
    bool minimized = scan.get_bool();
    bool maximized = scan.get_bool();
    set_window_state(x, y, width, height, minimized, maximized);
  }
}

/**
 * Resets the list of guide bars.
 */
void PStatGraph::
update_guide_bars(int num_bars, double scale) {
  _guide_bars.clear();

  // We'd like to draw about num_bars bars on the chart.  But we also want the
  // bars to be harmonics of the target frame rate, so that the bottom bar is
  // at tfrn or n * tfr, where n is an integer, and the upper bars are even
  // multiples of that.

  // Choose a suitable harmonic of the target frame rate near the bottom part
  // of the chart.

  double bottom = (double)num_bars / scale;

  double harmonic;
  if (_target_frame_rate < bottom) {
    // n * tfr
    harmonic = floor(bottom / _target_frame_rate + 0.5) * _target_frame_rate;

  } else {
    // tfr  n
    harmonic = _target_frame_rate / floor(_target_frame_rate / bottom + 0.5);
  }

  // Now, make a few bars at k  harmonic.
  for (int k = 1; k / harmonic <= scale; k++) {
    _guide_bars.push_back(make_guide_bar(k / harmonic));
  }

  _guide_bars_changed = true;
}

/**
 * Makes a guide bar for the indicated elapsed time or level units.
 */
PStatGraph::GuideBar PStatGraph::
make_guide_bar(double value, PStatGraph::GuideBarStyle style) const {
  string label = format_number(value, _guide_bar_units, _unit_name);

  if ((style == GBS_normal) &&
      (_guide_bar_units & GBU_named) == 0) {
    // If it's a time unit, check to see if it matches our target frame rate.
    double hz = 1.0 / value;
    if (IS_THRESHOLD_EQUAL(hz, _target_frame_rate, 0.001)) {
      style = GBS_target;
    }
  }

  return GuideBar(value, label, style);
}

/**
 * Returns the current window dimensions.
 */
bool PStatGraph::
get_window_state(int &x, int &y, int &width, int &height,
                 bool &maximized, bool &minimized) const {
  return false;
}

/**
 * Called to restore the graph window to its previous dimensions.
 */
void PStatGraph::
set_window_state(int x, int y, int width, int height,
                 bool maximized, bool minimized) {
}
