// Filename: geomBinGroup.I
// Created by:  drose (13Apr00)
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
//     Function: GeomBinGroup::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE GeomBinGroup::
GeomBinGroup(const string &name) :
  GeomBin(name)
{
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinGroup::get_num_bins
//       Access: Public
//  Description: Returns the number of available sub bins.
////////////////////////////////////////////////////////////////////
INLINE int GeomBinGroup::
get_num_bins() const {
  return _sub_bins.size();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinGroup::get_bin
//       Access: Public
//  Description: Returns the nth sub bin.
////////////////////////////////////////////////////////////////////
INLINE GeomBin *GeomBinGroup::
get_bin(int n) {
  nassertr(n >= 0 && n < (int)_sub_bins.size(), NULL);
  return _sub_bins[n];
}
