// Filename: cullResult.I
// Created by:  drose (28Feb02)
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


////////////////////////////////////////////////////////////////////
//     Function: CullResult::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CullResult::
CullResult(GraphicsStateGuardianBase *gsg) :
  _gsg(gsg)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CullResult::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CullResult::
~CullResult() {
}

////////////////////////////////////////////////////////////////////
//     Function: CullResult::get_bin
//       Access: Public
//  Description: Returns the CullBin associated with the indicated
//               bin_index, or NULL if the bin_index is invalid.  If
//               there is the first time this bin_index has been
//               requested for this CullResult, creates a new CullBin
//               object on the fly.
////////////////////////////////////////////////////////////////////
INLINE CullBin *CullResult::
get_bin(int bin_index) {
  if (bin_index >= 0 && bin_index < (int)_bins.size() && 
      _bins[bin_index] != (CullBin *)NULL) {
    return _bins[bin_index];
  }
  return make_new_bin(bin_index);
}
