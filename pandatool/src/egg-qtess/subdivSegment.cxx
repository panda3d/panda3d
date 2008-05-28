// Filename: subdivSegment.cxx
// Created by:  drose (14Oct03)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "subdivSegment.h"




////////////////////////////////////////////////////////////////////
//     Function: binary_search
//  Description: Performs a standard binary search.  This utility
//               function is used below.
////////////////////////////////////////////////////////////////////
static int
binary_search(double val, const double *array, int bot, int top) {
  if (top < bot) {
    return bot;
  }
  int mid = (bot + top)/2;

  if (array[mid] < val) {
    return binary_search(val, array, mid+1, top);
  } else {
    return binary_search(val, array, bot, mid-1);
  }
}


////////////////////////////////////////////////////////////////////
//     Function: SubdivSegment::cut
//       Access: Public
//  Description: Applies _num_cuts cuts to the segment.
////////////////////////////////////////////////////////////////////
void SubdivSegment::
cut() {
  int c;
  double ct = get_score();

  _cuts.erase(_cuts.begin(), _cuts.end());
  int last = _f;
  for (c = 1; c < _num_cuts+1; c++) {
    double val = (double)c * ct / (double)(_num_cuts+1) + _cint[_f];
    int i = binary_search(val, _cint, _f, _t);
    if (i != last && i < _t) {
      _cuts.push_back(i);
    }
    last = i;
  }

  while ((int)_cuts.size() < _num_cuts) {
    // Do we have any extra?  Assign them into likely places.
    int last = _f;
    int mc = -1;
    int mv = 0;
    for (c = 0; c < (int)_cuts.size(); c++) {
      if (mc == -1 || _cuts[c] - last > mv) {
        mc = c;
        mv = _cuts[c] - last;
      }
      last = _cuts[c];
    }

    if (mc==-1) {
      // Surrender.
      return;
    }
    if (mc==0) {
      _cuts.insert(_cuts.begin() + mc, (_cuts[mc] + _f) / 2);
    } else {
      _cuts.insert(_cuts.begin() + mc, (_cuts[mc] + _cuts[mc-1]) / 2);
    }
  }
}

