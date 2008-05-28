// Filename: eggMesherFanMaker.I
// Created by:  drose (22Mar05)
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
//     Function: EggMesherFanMaker::operator <
//       Access: Public
//  Description: Provides a unique ordering between different fan
//               makers based on the leading edge.
////////////////////////////////////////////////////////////////////
INLINE bool EggMesherFanMaker::
operator < (const EggMesherFanMaker &other) const {
  nassertr(!_edges.empty() && !other._edges.empty(), false);
  return _edges.front() < other._edges.front();
}

////////////////////////////////////////////////////////////////////
//     Function: EggMesherFanMaker::operator !=
//       Access: Public
//  Description: Provides a unique ordering between different fan
//               makers based on the leading edge.
////////////////////////////////////////////////////////////////////
INLINE bool EggMesherFanMaker::
operator != (const EggMesherFanMaker &other) const {
  return !operator == (other);
}

////////////////////////////////////////////////////////////////////
//     Function: EggMesherFanMaker::operator ==
//       Access: Public
//  Description: Provides a unique ordering between different fan
//               makers based on the leading edge.
////////////////////////////////////////////////////////////////////
INLINE bool EggMesherFanMaker::
operator == (const EggMesherFanMaker &other) const {
  return _edges.front() == other._edges.front();
}

////////////////////////////////////////////////////////////////////
//     Function: EggMesherFanMaker::is_empty
//       Access: Public
//  Description: Returns true if the fan maker has no edges, false
//               otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool EggMesherFanMaker::
is_empty() const {
  return (_edges.empty());
}

////////////////////////////////////////////////////////////////////
//     Function: EggMesherFanMaker::is_valid
//       Access: Public
//  Description: Returns true if the fan maker has enough edges to
//               define at least one fan, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool EggMesherFanMaker::
is_valid() const {
  return (_edges.size() > 2);
}

////////////////////////////////////////////////////////////////////
//     Function: EggMesherFanMaker::is_coplanar_with
//       Access: Public
//  Description: Returns true if the strip and the other strip are
//               coplanar.
////////////////////////////////////////////////////////////////////
INLINE bool EggMesherFanMaker::
is_coplanar_with(const EggMesherFanMaker &other) const {
  return _planar && other._planar &&
    _strips.front()->is_coplanar_with(*other._strips.front(),
                                      egg_coplanar_threshold);
}

INLINE ostream &
operator << (ostream &out, const EggMesherFanMaker &fm) {
  fm.output(out);
  return out;
}
