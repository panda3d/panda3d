// Filename: LOD.I
// Created by:  mike (09Jan97)
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

#include <math.h>

////////////////////////////////////////////////////////////////////
//     Function: LODSwitch::Default Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE LODSwitch::
LODSwitch() {
}

////////////////////////////////////////////////////////////////////
//     Function: LODSwitch::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE LODSwitch::
LODSwitch(float in, float out) {
  set_range(in, out);
}

////////////////////////////////////////////////////////////////////
//     Function: LODSwitch::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE LODSwitch::
LODSwitch(const LODSwitch &copy) :
  _in(copy._in),
  _out(copy._out)
{
}

////////////////////////////////////////////////////////////////////
//     Function: LODSwitch::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void LODSwitch::
operator = (const LODSwitch &copy) {
  _in = copy._in;
  _out = copy._out;
}

////////////////////////////////////////////////////////////////////
//     Function: LODSwitch::get_range
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void LODSwitch::
get_range(float &in, float &out) const {
  in = get_in();
  out = get_out();
}

////////////////////////////////////////////////////////////////////
//     Function: LODSwitch::get_in
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE float LODSwitch::
get_in() const {
  return sqrtf(_in);
}

////////////////////////////////////////////////////////////////////
//     Function: LODSwitch::get_out
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE float LODSwitch::
get_out() const {
  return sqrtf(_out);
}

////////////////////////////////////////////////////////////////////
//     Function: LODSwitch::set_range
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void LODSwitch::
set_range(float in, float out) {
  // We actually store the square of the switching distances.  This
  // makes the LOD computation a little simpler.
  _in = in * in;
  _out = out * out;
}

////////////////////////////////////////////////////////////////////
//     Function: LODSwitch::in_range
//       Access: Public
//  Description: Computes the distance between two points and returns
//               true if the result is within the range for the LOD.
////////////////////////////////////////////////////////////////////
INLINE bool LODSwitch::
in_range(float dist_squared) const {
  return (dist_squared >= _out && dist_squared < _in);
}

////////////////////////////////////////////////////////////////////
//     Function: LODSwitch::rescale
//       Access: Public
//  Description: Scales the switching distances by the square root of
//               the indicated factor.
////////////////////////////////////////////////////////////////////
INLINE void LODSwitch::
rescale(float factor_squared) {
  _in *= factor_squared;
  _out *= factor_squared;
}

////////////////////////////////////////////////////////////////////
//     Function: LODSwitch::operator ==
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool LODSwitch::
operator == (const LODSwitch &) const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: LODSwitch::operator !=
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool LODSwitch::
operator != (const LODSwitch &) const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: LODSwitch::operator <
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool LODSwitch::
operator < (const LODSwitch &) const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: LODSwitch::write_datagram
//       Access: Public
//  Description: Writes the contents of the LODSwitch out to the
//               datagram, presumably in preparation to writing to a
//               Bam file.
////////////////////////////////////////////////////////////////////
INLINE void LODSwitch::
write_datagram(Datagram &destination) const {
  destination.add_float32(_in);
  destination.add_float32(_out);
}

////////////////////////////////////////////////////////////////////
//     Function: LODSwitch::read_datagram
//       Access: Public
//  Description: Reads the contents of the LODSwitch from the
//               datagram, presumably in response to reading a Bam
//               file.
////////////////////////////////////////////////////////////////////
INLINE void LODSwitch::
read_datagram(DatagramIterator &source) {
  _in = source.get_float32();
  _out = source.get_float32();
}
