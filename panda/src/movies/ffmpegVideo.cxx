// Filename: ffmpegVideo.cxx
// Created by: jyelon (01Aug2007)
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

#ifdef HAVE_FFMPEG

#include "ffmpegVideo.h"
#include "config_movies.h"

TypeHandle FfmpegVideo::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVideo::Constructor
//       Access: Public
//  Description: xxx
////////////////////////////////////////////////////////////////////
FfmpegVideo::
FfmpegVideo(const Filename &name) :
  MovieVideo(name)
{
  _filename = name;
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVideo::Destructor
//       Access: Public
//  Description: xxx
////////////////////////////////////////////////////////////////////
FfmpegVideo::
~FfmpegVideo() {
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVideo::open
//       Access: Published, Virtual
//  Description: Open this video, returning a MovieVideoCursor.
////////////////////////////////////////////////////////////////////
PT(MovieVideoCursor) FfmpegVideo::
open() {
  PT(FfmpegVideoCursor) result = new FfmpegVideoCursor(this);
  if (result->_format_ctx == 0) {
    movies_cat.error() << "Could not open " << _filename << "\n";
    return NULL;
  } else {
    return (MovieVideoCursor*)(FfmpegVideoCursor*)result;
  }
}


////////////////////////////////////////////////////////////////////

#endif // HAVE_FFMPEG
