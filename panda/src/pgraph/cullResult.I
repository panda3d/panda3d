// Filename: cullResult.I
// Created by:  drose (28Feb02)
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

////////////////////////////////////////////////////////////////////
//     Function: CullResult::add_object
//       Access: Public
//  Description: Adds the indicated CullableObject to the appropriate
//               bin.  The bin becomes the owner of the object
//               pointer, and will eventually delete it.
////////////////////////////////////////////////////////////////////
INLINE void CullResult::
add_object(CullableObject *object) {
  nassertv(object->_state != (const RenderState *)NULL);
  CullBin *bin = get_bin(object->_state->get_bin_index());
  nassertv(bin != (CullBin *)NULL);
  bin->add_object(object);
}
