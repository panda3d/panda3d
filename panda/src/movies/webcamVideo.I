// Filename: webcamVideo.I
// Created by: jyelon (01Nov2007)
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
//     Function: WebcamVideo::get_size_x
//       Access: Published
//  Description: Returns the camera's size_x.
////////////////////////////////////////////////////////////////////
INLINE int WebcamVideo::
get_size_x() const {
  return _size_x;
}

////////////////////////////////////////////////////////////////////
//     Function: WebcamVideo::get_size_y
//       Access: Published
//  Description: Returns the camera's size_y.
////////////////////////////////////////////////////////////////////
INLINE int WebcamVideo::
get_size_y() const {
  return _size_y;
}

////////////////////////////////////////////////////////////////////
//     Function: WebcamVideo::get_fps
//       Access: Published
//  Description: Returns the camera's framerate.  This
//               is a maximum theoretical: the actual performance 
//               will depend on the speed of the hardware.
////////////////////////////////////////////////////////////////////
INLINE int WebcamVideo::
get_fps() const {
  return _fps;
}
