// Filename: subdivSegment.I
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


////////////////////////////////////////////////////////////////////
//     Function: SubdivSegment::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE SubdivSegment::
SubdivSegment(const double *cint, int f, int t) : 
  _cint(cint), 
  _f(f),
  _t(t) 
{
}

////////////////////////////////////////////////////////////////////
//     Function: SubdivSegment::get_score
//       Access: Public
//  Description: Returns the net score of the segment.
////////////////////////////////////////////////////////////////////
INLINE double SubdivSegment::
get_score() const {
  return _cint[_t] - _cint[_f];
}

////////////////////////////////////////////////////////////////////
//     Function: SubdivSegment::get_need
//       Access: Public
//  Description: Returns a score that indicates how badly the segment
//               needs to be further subdivided.  The greater the
//               number, the greater the need.
////////////////////////////////////////////////////////////////////
INLINE double SubdivSegment::
get_need() const {
  return get_score() / (double)(_num_cuts+1);
}

////////////////////////////////////////////////////////////////////
//     Function: SubdivSegment::operator <
//       Access: Public
//  Description: Sorts the segments in descending order of need.
////////////////////////////////////////////////////////////////////
INLINE bool SubdivSegment::
operator < (const SubdivSegment &other) const {
  return get_need() > other.get_need();
}
