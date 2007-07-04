// Filename: movie.I
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
//     Function: Movie::ignores_offset
//       Access: Published
//  Description: The methods get_video and get_audio both accept
//               an offset parameter, which allow you to seek
//               to a particular location within the movie.  Not
//               all movies can do this, though.  For instance, an
//               internet TV station cannot be fast-forwarded.  In
//               these cases, the offset parameter is ignored.
////////////////////////////////////////////////////////////////////
INLINE bool Movie::
ignores_offset() const {
  return _ignores_offset;
}

////////////////////////////////////////////////////////////////////
//     Function: Movie::dummy_video
//       Access: Published
//  Description: If you open a sound file as a movie, a dummy video
//               stream will be synthesized.  Returns true in this case.
////////////////////////////////////////////////////////////////////
INLINE bool Movie::
dummy_video() const {
  return _dummy_video;
}

////////////////////////////////////////////////////////////////////
//     Function: Movie::dummy_audio
//       Access: Published
//  Description: If you open a movie with no sound, a dummy sound
//               stream will be synthesized.  Returns true in this case.
////////////////////////////////////////////////////////////////////
INLINE bool Movie::
dummy_audio() const {
  return _dummy_audio;
}

