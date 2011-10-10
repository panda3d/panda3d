// Filename: lwoPoints.cxx
// Created by:  drose (24Apr01)
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

#include "lwoPoints.h"
#include "lwoInputFile.h"

#include "dcast.h"
#include "indent.h"

TypeHandle LwoPoints::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LwoPoints::get_num_points
//       Access: Public
//  Description: Returns the number of points of this group.
////////////////////////////////////////////////////////////////////
int LwoPoints::
get_num_points() const {
  return _points.size();
}

////////////////////////////////////////////////////////////////////
//     Function: LwoPoints::get_point
//       Access: Public
//  Description: Returns the nth point of this group.
////////////////////////////////////////////////////////////////////
const LPoint3 &LwoPoints::
get_point(int n) const {
  nassertr(n >= 0 && n < (int)_points.size(), LPoint3::zero());
  return _points[n];
}

////////////////////////////////////////////////////////////////////
//     Function: LwoPoints::read_iff
//       Access: Public, Virtual
//  Description: Reads the data of the chunk in from the given input
//               file, if possible.  The ID and length of the chunk
//               have already been read.  stop_at is the byte position
//               of the file to stop at (based on the current position
//               at in->get_bytes_read()).  Returns true on success,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool LwoPoints::
read_iff(IffInputFile *in, size_t stop_at) {
  LwoInputFile *lin = DCAST(LwoInputFile, in);

  while (lin->get_bytes_read() < stop_at && !lin->is_eof()) {
    LPoint3 point = lin->get_vec3();
    _points.push_back(point);
  }

  return (lin->get_bytes_read() == stop_at);
}

////////////////////////////////////////////////////////////////////
//     Function: LwoPoints::write
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void LwoPoints::
write(ostream &out, int indent_level) const {
  indent(out, indent_level)
    << get_id() << " { " << _points.size() << " points }\n";
}
