// Filename: glxGraphicsStateGuardian.I
// Created by:  drose (27Jan03)
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
//     Function: glxGraphicsStateGuardian::get_fb_properties
//       Access: Private
//  Description: Gets the FrameBufferProperties for all windows and
//               buffers that use this GSG.
////////////////////////////////////////////////////////////////////
INLINE const FrameBufferProperties &glxGraphicsStateGuardian::
get_fb_properties() const {
  return _fbprops;
}


