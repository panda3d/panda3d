// Filename: graphicsDevice.I
// Created by:  masad (21Jul03)
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
//     Function: GraphicsDevice::get_pipe
//       Access: Published
//  Description: Returns the GraphicsPipe that this device is
//               associated with.
////////////////////////////////////////////////////////////////////
INLINE GraphicsPipe *GraphicsDevice::
get_pipe() const {
  return _pipe;
}

