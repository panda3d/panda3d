// Filename: graphicsDevice.I
// Created by:  masad (21Jul03)
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
//     Function: GraphicsDevice::get_pipe
//       Access: Published
//  Description: Returns the GraphicsPipe that this device is
//               associated with.
////////////////////////////////////////////////////////////////////
INLINE GraphicsPipe *GraphicsDevice::
get_pipe() const {
  return _pipe;
}

