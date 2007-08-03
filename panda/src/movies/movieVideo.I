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
//     Function: MovieVideo::at_end
//       Access: Published
//  Description: Returns true if there is no video left to fetch.
//               
//               It is not an error to call 'fetch' when at_end, 
//               doing so will yield a black frame of one second
//               duration.
////////////////////////////////////////////////////////////////////
INLINE bool MovieVideo::
at_end() const {
  return _at_end;
}

////////////////////////////////////////////////////////////////////
//     Function: MovieVideo::next_start
//       Access: Published
//  Description: Indicates the time at which the current frame should
//               stop being displayed and when the next frame should
//               start being displayed.  This time is relative to the
//               start of the stream.
//
//               MovieVideo streams have variable frame rates.  Each
//               frame will specify how long it is to be displayed.
//               These lengths may not be equal from frame to frame.
////////////////////////////////////////////////////////////////////
INLINE double MovieVideo::
next_start() const {
  return _next_start;
}

////////////////////////////////////////////////////////////////////
//     Function: MovieVideo::approx_len
//       Access: Published
//  Description: Get the approximate length of the movie.
//               This is not guaranteed to be accurate. The only
//               accurate way to determine when the movie is over
//               is to check the at_end flag.
////////////////////////////////////////////////////////////////////
INLINE double MovieVideo::
approx_len() const {
  return _approx_len;
}

