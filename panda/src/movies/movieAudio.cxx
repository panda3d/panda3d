// Filename: movieAudio.cxx
// Created by: jyelon (02Jul07)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2007, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net 
//
////////////////////////////////////////////////////////////////////

#include "movieAudio.h"
#include "movieAudioCursor.h"

TypeHandle MovieAudio::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: MovieAudio::Constructor
//       Access: Public
//  Description: This constructor returns a null audio stream --- a
//               stream of total silence, at 8000 samples per second.
//               To get more interesting audio, you need to construct
//               a subclass of this class.
////////////////////////////////////////////////////////////////////
MovieAudio::
MovieAudio(const string &name) :
  Namable(name)
{
}

////////////////////////////////////////////////////////////////////
//     Function: MovieAudio::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
MovieAudio::
~MovieAudio() {
}

////////////////////////////////////////////////////////////////////
//     Function: MovieAudio::open
//       Access: Published, Virtual
//  Description: Open this audio, returning a MovieAudioCursor
////////////////////////////////////////////////////////////////////
PT(MovieAudioCursor) MovieAudio::
open() {
  return new MovieAudioCursor(this);
}

////////////////////////////////////////////////////////////////////
//     Function: MovieAudio::get
//       Access: Published, Static
//  Description: Obtains a MovieAudio that references a file.
////////////////////////////////////////////////////////////////////
PT(MovieAudio) MovieAudio::
get(const Filename &name) {
#ifdef HAVE_FFMPEG
  // Someday, I'll probably put a dispatcher here.
  // But for now, just hardwire it to go to FFMPEG.
  return new FfmpegAudio(name);
#else
  return new MovieAudio("Load-Failure Stub");
#endif
}
