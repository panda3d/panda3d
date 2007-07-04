// Filename: movieVideo.I
// Created by: jyelon (02Jul07)
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
//     Function: MovieVideo::size_x
//       Access: Published
//  Description: Get the horizontal size of the movie.
////////////////////////////////////////////////////////////////////
INLINE int MovieVideo::
size_x() const {
  return _size_x;
}

////////////////////////////////////////////////////////////////////
//     Function: MovieVideo::size_y
//       Access: Published
//  Description: Get the vertical size of the movie.
////////////////////////////////////////////////////////////////////
INLINE int MovieVideo::
size_y() const {
  return _size_y;
}

////////////////////////////////////////////////////////////////////
//     Function: MovieVideo::frame_start
//       Access: Published
//  Description: MovieVideo streams have variable frame rates.  Each
//               frame will specify how long it is to be displayed.
//               These lengths may not be equal from frame to frame.
//
//               The frame_start function returns the time at which
//               the current frame should start being displayed.  This
//               time is relative to the start of the stream.
////////////////////////////////////////////////////////////////////
INLINE double MovieVideo::
frame_start() const {
  return _frame_start;
}

////////////////////////////////////////////////////////////////////
//     Function: MovieVideo::frame_end
//       Access: Published
//  Description: Return the time at which this frame should stop
//               being displayed.  This time is relative to the start
//               of the stream.
//
//               The frame_end function returns the time at which
//               the current frame should stop being displayed.  This
//               time is relative to the start of the stream.
////////////////////////////////////////////////////////////////////
INLINE double MovieVideo::
frame_end() const {
  return _frame_end;
}

////////////////////////////////////////////////////////////////////
//     Function: MovieVideo::approx_len
//       Access: Published
//  Description: Get the vertical size of the movie.
////////////////////////////////////////////////////////////////////
INLINE double MovieVideo::
approx_len() const {
  return _approx_len;
}

