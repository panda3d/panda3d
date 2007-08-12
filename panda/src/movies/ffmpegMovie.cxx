// Filename: ffmpegMovie.cxx
// Created by: jyelon (01Aug2007)
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
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "ffmpegVideo.h"
#include "ffmpegAudio.h"
#include "ffmpegMovie.h"
#include "config_movies.h"

TypeHandle FfmpegMovie::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: FfmpegMovie::Constructor
//       Access: Published
//  Description: xxx
////////////////////////////////////////////////////////////////////
FfmpegMovie::
FfmpegMovie(const string &filename) :
  Movie(filename,1.0)
{
}

////////////////////////////////////////////////////////////////////
//     Function: Movie::get_video
//       Access: Published, Virtual
//  Description: Fetch a video stream.  Always constructs a new
//               MovieVideo or subclass of MovieVideo.
////////////////////////////////////////////////////////////////////
PT(MovieVideo) FfmpegMovie::
get_video(double offset) const {
  if (_dummy_video) {
    return new MovieVideo(this);
  } else {
    return new FfmpegVideo(this, offset);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Movie::get_audio
//       Access: Published, Virtual
//  Description: Fetch an audio stream.  Always constructs a new
//               MovieAudio or subclass of MovieAudio.
////////////////////////////////////////////////////////////////////
PT(MovieAudio) FfmpegMovie::
get_audio(double offset) const {
  if (_dummy_audio) {
    return new MovieAudio(this);
  } else {
    return new FfmpegAudio(this, offset);
  }
}

