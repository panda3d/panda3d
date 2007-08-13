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
//     Function: MovieVideo::get_num_components
//       Access: Published
//  Description: Returns 4 if the movie has an alpha
//               channel, 3 otherwise.
////////////////////////////////////////////////////////////////////
INLINE int MovieVideo::
get_num_components() const {
  return _num_components;
}

////////////////////////////////////////////////////////////////////
//     Function: MovieVideo::length
//       Access: Published
//  Description: Returns the length of the movie.
//
//               Some kinds of Movie, such as internet TV station, 
//               might not have a predictable length.  In that case,
//               the length will be set to a very large number: 1.0E10.
//               If the internet TV station goes offline, the video
//               or audio stream will set its abort flag.  Reaching the
//               end of the movie (ie, the specified length) normally
//               does not cause the abort flag to be set.
//
//               The video and audio streams produced by get_video and
//               get_audio are always of unlimited duration - you can
//               always read another video frame or another audio
//               sample.  This is true even if the specified length
//               is reached, or an abort is flagged. If either stream
//               runs out of data, it will synthesize blank video
//               frames and silent audio samples as necessary to
//               satisfy read requests.
//
//               Some AVI files have incorrect length values encoded
//               into them - usually, they're a second or two long or
//               short.  When playing such an AVI using the Movie class,
//               you may see a slightly truncated video, or a slightly
//               elongated video (padded with black frames).  There are
//               utilities out there to fix the length values in AVI
//               files.
//
////////////////////////////////////////////////////////////////////
INLINE int MovieVideo::
length() const {
  return _length;
}

////////////////////////////////////////////////////////////////////
//     Function: MovieVideo::can_seek
//       Access: Published
//  Description: See explanation in fetch_to_buffer.
////////////////////////////////////////////////////////////////////
INLINE bool MovieVideo::
can_seek() const {
  return _can_seek;
}

////////////////////////////////////////////////////////////////////
//     Function: MovieVideo::can_seek_zero
//       Access: Published
//  Description: See explanation in fetch_to_buffer.
////////////////////////////////////////////////////////////////////
INLINE bool MovieVideo::
can_seek_zero() const {
  return _can_seek_zero;
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

