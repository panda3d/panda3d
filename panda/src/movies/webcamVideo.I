// Filename: webcamVideo.I
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
//     Function: WebcamVideo::get_name
//       Access: Published
//  Description: Returns the camera's name / description.
////////////////////////////////////////////////////////////////////
INLINE const string &WebcamVideo::
get_name() const {
  return _name;
}

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
