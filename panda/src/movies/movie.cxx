// Filename: movie.cxx
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
#include "movieAudio.h"
#include "movie.h"
#include "config_movies.h"

TypeHandle Movie::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: Movie::Constructor
//       Access: Published
//  Description: This constructor returns a null/dummy movie --- 
//               the video is flashing blue/white, and the audio is
//               silent.  To get more interesting movie, you need to
//               construct a subclass of this class.
////////////////////////////////////////////////////////////////////
Movie::
Movie(const string &name, double len) :
  Namable(name),
  _size_x(1),
  _size_y(1),
  _num_components(3),
  _length(len),
  _audio_rate(8000),
  _audio_channels(1),
  _ignores_offset(true),
  _dummy_video(true),
  _dummy_audio(true)
{
}

////////////////////////////////////////////////////////////////////
//     Function: Movie::Destructor
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
Movie::
~Movie() {
}
 
////////////////////////////////////////////////////////////////////
//     Function: Movie::get_video
//       Access: Published, Virtual
//  Description: Fetch a video stream.  Always constructs a new
//               MovieVideo or subclass of MovieVideo.
////////////////////////////////////////////////////////////////////
PT(MovieVideo) Movie::
get_video(double offset) const {
  return new MovieVideo(get_name(), this);
}

////////////////////////////////////////////////////////////////////
//     Function: Movie::get_audio
//       Access: Published, Virtual
//  Description: Fetch an audio stream.  Always constructs a new
//               MovieAudio or subclass of MovieAudio.
////////////////////////////////////////////////////////////////////
PT(MovieAudio) Movie::
get_audio(double offset) const {
  return new MovieAudio(get_name(), this);
}

////////////////////////////////////////////////////////////////////
//     Function: Movie::load
//       Access: Published, Static
//  Description: Loads a movie from a file.
////////////////////////////////////////////////////////////////////
PT(Movie) Movie::
load(const Filename &path) {
  // For now, just return a dummy movie.
  return new Movie("dummy",30.0);
}
