// Filename: perspectiveLens.I
// Created by:  drose (18Feb99)
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
//     Function: PerspectiveLens::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PerspectiveLens::
PerspectiveLens() {
}

////////////////////////////////////////////////////////////////////
//     Function: PerspectiveLens::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PerspectiveLens::
PerspectiveLens(PN_stdfloat hfov, PN_stdfloat vfov) {
  Lens::CDWriter lens_cdata(Lens::_cycler, true);
  lens_cdata->_fov.set(hfov, vfov);
}

////////////////////////////////////////////////////////////////////
//     Function: PerspectiveLens::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PerspectiveLens::
PerspectiveLens(const PerspectiveLens &copy) : Lens(copy) {
}

////////////////////////////////////////////////////////////////////
//     Function: PerspectiveLens::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void PerspectiveLens::
operator = (const PerspectiveLens &copy) {
  Lens::operator = (copy);
}

