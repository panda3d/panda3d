// Filename: trackerData.cxx
// Created by:  jason (04Aug00)
// 
////////////////////////////////////////////////////////////////////

#include "trackerData.h"

////////////////////////////////////////////////////////////////////
//     Function: TrackerData::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void TrackerData::
operator = (const TrackerData &copy) {
  _flags = copy._flags;

  _time = copy._time;
  _pos = copy._pos;
  _orient = copy._orient;
  _dt = copy._dt;
}
