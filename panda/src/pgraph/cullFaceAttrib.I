// Filename: cullFaceAttrib.I
// Created by:  drose (27Feb02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: CullFaceAttrib::Constructor
//       Access: Private
//  Description: Use CullFaceAttrib::make() to construct a new
//               CullFaceAttrib object.
////////////////////////////////////////////////////////////////////
INLINE CullFaceAttrib::
CullFaceAttrib(CullFaceAttrib::Mode mode, bool reverse) :
  _mode(mode),
  _reverse(reverse)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CullFaceAttrib::get_actual_mode
//       Access: Published
//  Description: Returns the actual culling mode, without considering
//               the effects of the reverse flag.  See also
//               get_effective_mode().
////////////////////////////////////////////////////////////////////
INLINE CullFaceAttrib::Mode CullFaceAttrib::
get_actual_mode() const {
  return _mode;
}

////////////////////////////////////////////////////////////////////
//     Function: CullFaceAttrib::get_reverse
//       Access: Published
//  Description: Returns the 'reverse' flag.  If this is true, the
//               actual cull direction (clockwise
//               vs. counterclockwise) is the reverse of what is
//               specified here.  This allows support for
//               make_reverse(), which defines a CullFaceAttrib that
//               reverses whatever the sense of culling would have
//               been.
////////////////////////////////////////////////////////////////////
INLINE bool CullFaceAttrib::
get_reverse() const {
  return _reverse;
}
