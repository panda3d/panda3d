// Filename: transformBlendPalette.I
// Created by:  drose (24Mar05)
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
//     Function: TransformBlendPalette::get_num_blends
//       Access: Published
//  Description: Returns the total number of different blend
//               combinations in the palette.
////////////////////////////////////////////////////////////////////
INLINE int TransformBlendPalette::
get_num_blends() const {
  return _blends.size();
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlendPalette::get_blend
//       Access: Published
//  Description: Returns the nth blend in the palette.
////////////////////////////////////////////////////////////////////
INLINE const TransformBlend &TransformBlendPalette::
get_blend(int n) const {
  nassertr(n >= 0 && n < (int)_blends.size(), _blends[0]);
  return _blends[n];
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlendPalette::get_modified
//       Access: Published
//  Description: Returns a counter which is guaranteed to increment at
//               least when any TransformBlends within the palette
//               have changed.
////////////////////////////////////////////////////////////////////
INLINE UpdateSeq TransformBlendPalette::
get_modified() const {
  CDReader cdata(_cycler);
  if (cdata->_global_modified != VertexTransform::get_global_modified()) {
    CDWriter cdataw(((TransformBlendPalette *)this)->_cycler, cdata);
    ((TransformBlendPalette *)this)->recompute_modified(cdataw);
    return cdataw->_modified;
  } else {
    return cdata->_modified;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlendPalette::CData::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE TransformBlendPalette::CData::
CData() {
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlendPalette::CData::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE TransformBlendPalette::CData::
CData(const TransformBlendPalette::CData &copy) :
  _modified(copy._modified),
  _global_modified(copy._global_modified)
{
}
