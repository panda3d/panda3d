// Filename: pStatGraph.cxx
// Created by:  drose (19Jul00)
// 
////////////////////////////////////////////////////////////////////

#include "pStatGraph.h"

#include <pStatFrameData.h>
#include <pStatCollectorDef.h>
#include <string_utils.h>
#include <config_pstats.h>

#include <stdio.h>  // for sprintf

////////////////////////////////////////////////////////////////////
//     Function: PStatGraph::GuideBar::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PStatGraph::GuideBar::
GuideBar(double height, const string &label, bool is_target) :
  _height(height),
  _label(label),
  _is_target(is_target)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PStatGraph::GuideBar::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PStatGraph::GuideBar::
GuideBar(const PStatGraph::GuideBar &copy) :
  _height(copy._height),
  _label(copy._label),
  _is_target(copy._is_target)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PStatGraph::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: PStatGraph::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
PStatGraph::
~PStatGraph() {
}

////////////////////////////////////////////////////////////////////
//     Function: PStatGraph::get_num_guide_bars
//       Access: Public
//  Description: Returns the number of horizontal guide bars that
//               should be drawn, based on the indicated target frame
//               rate.  Not all of these may be visible; some may be
//               off the top of the chart because of the vertical
//               scale.
////////////////////////////////////////////////////////////////////
int PStatGraph::
get_num_guide_bars() const {
  return _guide_bars.size();
}

////////////////////////////////////////////////////////////////////
//     Function: PStatGraph::get_guide_bar
//       Access: Public
//  Description: Returns the nth horizontal guide bar.  This should be
//               drawn as a horizontal line across the chart at the y
//               pixel location determined by height_to_pixel(bar._height).
//
//               It is possible that this bar will be off the top of
//               the chart.
////////////////////////////////////////////////////////////////////
const PStatGraph::GuideBar &PStatGraph::
get_guide_bar(int n) const {
#ifndef NDEBUG
  static GuideBar bogus_bar(0.0, "bogus", false);
  nassertr(n >= 0 && n < _guide_bars.size(), bogus_bar);
#endif
  return _guide_bars[n];
}


// STL function object for sorting labels in order by the collector's
// sort index, used in update_labels(), below.
class SortCollectorLabels {
public:
  SortCollectorLabels(const PStatClientData *client_data) :
    _client_data(client_data) {
  }
  bool operator () (int a, int b) const {
    // By casting the sort numbers to unsigned ints, we cheat and make
    // -1 appear to be a very large positive integer, thus placing
    // collectors with a -1 sort value at the very end.
    return 
      (unsigned int)_client_data->get_collector_def(a)._sort <
      (unsigned int)_client_data->get_collector_def(b)._sort;
  }
  const PStatClientData *_client_data;
};

////////////////////////////////////////////////////////////////////
//     Function: PStatGraph::update_guide_bars
//       Access: Protected
//  Description: Resets the list of guide bars.
////////////////////////////////////////////////////////////////////
void PStatGraph::
update_guide_bars(int num_bars, double scale) {
  _guide_bars.clear();

  // We'd like to draw about num_bars bars on the chart.  But we also
  // want the bars to be harmonics of the target frame rate, so that
  // the bottom bar is at tfr/n or n * tfr, where n is an integer, and
  // the upper bars are even multiples of that.

  // Choose a suitable harmonic of the target frame rate near the
  // bottom part of the chart.

  double bottom = (double)num_bars / scale;

  double harmonic;
  if (_target_frame_rate < bottom) {
    // n * tfr
    harmonic = floor(bottom / _target_frame_rate + 0.5) * _target_frame_rate;

  } else {
    // tfr / n
    harmonic = _target_frame_rate / floor(_target_frame_rate / bottom + 0.5);
  }

  // Now, make a few bars at k / harmonic.
  for (int k = 1; k / harmonic <= scale; k++) {
    _guide_bars.push_back(make_guide_bar(k / harmonic));
  }

  _guide_bars_changed = true;
}


////////////////////////////////////////////////////////////////////
//     Function: PStatGraph::make_guide_bar
//       Access: Protected
//  Description: Makes a guide bar for the indicated frame rate.
////////////////////////////////////////////////////////////////////
PStatGraph::GuideBar PStatGraph::
make_guide_bar(double time) const {
  string label;

  char buffer[128];

  if ((_guide_bar_units & GBU_ms) != 0) {
    double ms = time * 1000.0;
    if (ms < 10.0) {
      sprintf(buffer, "%0.1f", ms);
    } else {
      sprintf(buffer, "%0.0f", ms);
    }
    label += buffer;
    if ((_guide_bar_units & GBU_show_units) != 0) {
      label += " ms";
    }
  }

  if ((_guide_bar_units & GBU_hz) != 0) {
    double frame_rate = 1.0 / time;
    if (frame_rate < 10.0) {
      sprintf(buffer, "%0.1f", frame_rate);
    } else {
      sprintf(buffer, "%0.0f", frame_rate);
    }
    if ((_guide_bar_units & GBU_ms) != 0) {
      label += " (";
    }
    label += buffer;
    if ((_guide_bar_units & GBU_show_units) != 0) {
      label += " Hz";
    }
    if ((_guide_bar_units & GBU_ms) != 0) {
      label += ")";
    }
  }

  return GuideBar(time, label,
		  IS_NEARLY_EQUAL(1.0 / time, _target_frame_rate));
}
