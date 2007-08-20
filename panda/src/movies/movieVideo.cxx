// Filename: movieVideo.cxx
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
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "movieVideo.h"
#include "config_movies.h"

TypeHandle MovieVideo::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: MovieVideo::Constructor
//       Access: Public
//  Description: This constructor returns a null video stream --- a
//               stream of plain blue and white frames that last one
//               second each. To get more interesting video, you need
//               to construct a subclass of this class.
////////////////////////////////////////////////////////////////////
MovieVideo::
MovieVideo(const string &name) :
  Namable(name)
{
}

////////////////////////////////////////////////////////////////////
//     Function: MovieVideo::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
MovieVideo::
~MovieVideo() {
}

////////////////////////////////////////////////////////////////////
//     Function: MovieVideo::open
//       Access: Published, Virtual
//  Description: Open this video, returning a MovieVideoCursor.
////////////////////////////////////////////////////////////////////
PT(MovieVideoCursor) MovieVideo::
open() {
  return new MovieVideoCursor(this);
}

////////////////////////////////////////////////////////////////////
//     Function: MovieVideo::get
//       Access: Published, Static
//  Description: Obtains a MovieVideo that references a file.
////////////////////////////////////////////////////////////////////
PT(MovieVideo) MovieVideo::
get(const Filename &name) {
#ifdef HAVE_FFMPEG
  // Someday, I'll probably put a dispatcher here.
  // But for now, just hardwire it to go to FFMPEG.
  return new FfmpegVideo(name);
#else
  return new MovieVideo("Load-Failure Stub");
#endif
}

