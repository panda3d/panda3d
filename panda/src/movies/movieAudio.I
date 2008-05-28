// Filename: movieAudio.I
// Created by: jyelon (02Jul07)
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
//     Function: MovieAudio::get_filename
//       Access: Published
//  Description: Returns the movie's filename.  A movie is not
//               guaranteed to have a filename, if not, then this
//               function returns a null filename.
////////////////////////////////////////////////////////////////////
INLINE const Filename &MovieAudio::
get_filename() const {
  return _filename;
}

