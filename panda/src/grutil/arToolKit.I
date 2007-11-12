// Filename: arToolKit.I
// Created by: jyelon (01Nov2007)
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
//     Function: ARToolKit::set_threshold
//       Access: private
//  Description: As part of its analysis, the ARToolKit occasionally
//               converts images to black and white by thresholding
//               them.  The threshold is set to 0.5 by default, but
//               you can tweak it here.
////////////////////////////////////////////////////////////////////
INLINE void ARToolKit::
set_threshold(double thresh) {
  _threshold = thresh;
}

