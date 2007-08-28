// Filename: movieAudio.I
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

