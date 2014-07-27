// Filename: ffmpegAudio.cxx
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

#include "config_ffmpeg.h"
#include "ffmpegAudio.h"
#include "ffmpegAudioCursor.h"
#include "dcast.h"

TypeHandle FfmpegAudio::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: FfmpegAudio::Constructor
//       Access: Protected
//  Description: xxx
////////////////////////////////////////////////////////////////////
FfmpegAudio::
FfmpegAudio(const Filename &name) :
  MovieAudio(name)
{
  _filename = name;
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegAudio::Destructor
//       Access: Protected, Virtual
//  Description: xxx
////////////////////////////////////////////////////////////////////
FfmpegAudio::
~FfmpegAudio() {
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegAudio::open
//       Access: Published, Virtual
//  Description: Open this audio, returning a MovieAudioCursor
////////////////////////////////////////////////////////////////////
PT(MovieAudioCursor) FfmpegAudio::
open() {
  PT(FfmpegAudioCursor) result = new FfmpegAudioCursor(this);
  if (result->_format_ctx == 0) {
    ffmpeg_cat.error() << "Could not open " << _filename << "\n";
    return NULL;
  } else {
    return (MovieAudioCursor*)(FfmpegAudioCursor*)result;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegAudio::make
//       Access: Published, Static
//  Description: Obtains a MovieAudio that references a file.
////////////////////////////////////////////////////////////////////
PT(MovieAudio) FfmpegAudio::
make(const Filename &name) {
  return DCAST(MovieAudio, new FfmpegAudio(name));
}
