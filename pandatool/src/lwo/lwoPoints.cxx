// Filename: lwoPoints.cxx
// Created by:  drose (24Apr01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
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
const LPoint3f &LwoPoints::
get_point(int n) const {
  nassertr(n >= 0 && n < (int)_points.size(), LPoint3f::zero());
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
    LPoint3f point = lin->get_vec3();
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
