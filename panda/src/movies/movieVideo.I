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
//     Function: MovieVideo::get_source
//       Access: Published
//  Description: Returns a pointer to the source Movie.
////////////////////////////////////////////////////////////////////
INLINE CPT(Movie) MovieVideo::
get_source() const {
  return _source;
}

////////////////////////////////////////////////////////////////////
//     Function: MovieVideo::size_x
//       Access: Published
//  Description: See Movie::size_x
////////////////////////////////////////////////////////////////////
INLINE int MovieVideo::
size_x() const {
  return _source->size_x();
}

////////////////////////////////////////////////////////////////////
//     Function: MovieVideo::size_y
//       Access: Published
//  Description: See Movie::size_y
////////////////////////////////////////////////////////////////////
INLINE int MovieVideo::
size_y() const {
  return _source->size_y();
}

////////////////////////////////////////////////////////////////////
//     Function: MovieVideo::get_num_components
//       Access: Published
//  Description: See Movie::get_num_components
////////////////////////////////////////////////////////////////////
INLINE int MovieVideo::
get_num_components() const {
  return _source->get_num_components();
}

////////////////////////////////////////////////////////////////////
//     Function: MovieVideo::length
//       Access: Published
//  Description: See Movie::length
////////////////////////////////////////////////////////////////////
INLINE int MovieVideo::
length() const {
  return _source->length();
}

////////////////////////////////////////////////////////////////////
//     Function: MovieVideo::aborted
//       Access: Published
//  Description: Returns true if the video has aborted prematurely.
//               For example, this could occur if the Movie was actually
//               an internet TV station, and the connection was lost.
//               Reaching the normal end of the video does not
//               constitute an 'abort' condition.
////////////////////////////////////////////////////////////////////
INLINE bool MovieVideo::
aborted() const {
  return _aborted;
}

////////////////////////////////////////////////////////////////////
//     Function: MovieVideo::last_start
//       Access: Published
//  Description: Returns the start time of the last frame you read.
//
//               MovieVideo streams have variable frame rates.  Each
//               frame will specify how long it is to be displayed.
//               These lengths may not be equal from frame to frame.
////////////////////////////////////////////////////////////////////
INLINE double MovieVideo::
last_start() const {
  return _last_start;
}

////////////////////////////////////////////////////////////////////
//     Function: MovieVideo::next_start
//       Access: Published
//  Description: Returns the start time of the next frame you can read.
//
//               MovieVideo streams have variable frame rates.  Each
//               frame will specify how long it is to be displayed.
//               These lengths may not be equal from frame to frame.
////////////////////////////////////////////////////////////////////
INLINE double MovieVideo::
next_start() const {
  return _next_start;
}

