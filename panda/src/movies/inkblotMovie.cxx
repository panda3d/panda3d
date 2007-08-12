// Filename: inkblotMovie.cxx
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

#include "inkblotVideo.h"
#include "inkblotMovie.h"
#include "config_movies.h"

TypeHandle InkblotMovie::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: InkblotMovie::Constructor
//       Access: Published
//  Description: xxx
////////////////////////////////////////////////////////////////////
InkblotMovie::
InkblotMovie(const string &name, double len, int sizex, int sizey, int fps) :
  Movie(name,len)
{
  _ignores_offset = true;
  if (sizex < 1) sizex=1;
  if (sizey < 1) sizey=1;
  if (fps < 1) fps=1;
  _size_x = sizex;
  _size_y = sizey;
  _fps = fps;
  _dummy_video = false;
}

////////////////////////////////////////////////////////////////////
//     Function: Movie::get_video
//       Access: Published, Virtual
//  Description: Fetch a video stream.  Always constructs a new
//               MovieVideo or subclass of MovieVideo.
////////////////////////////////////////////////////////////////////
PT(MovieVideo) InkblotMovie::
get_video(double offset) const {
  return new InkblotVideo(this);
}

////////////////////////////////////////////////////////////////////
//     Function: Movie::get_audio
//       Access: Published, Virtual
//  Description: Fetch an audio stream.  Always constructs a new
//               MovieAudio or subclass of MovieAudio.
////////////////////////////////////////////////////////////////////
PT(MovieAudio) InkblotMovie::
get_audio(double offset) const {
  return new MovieAudio(this);
}

