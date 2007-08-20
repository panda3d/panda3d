// Filename: ffmpegAudio.cxx
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
// panda3d-general@lists.sourceforge.net 
//
////////////////////////////////////////////////////////////////////

#ifdef HAVE_FFMPEG

#include "ffmpegAudio.h"
#include "ffmpegAudioCursor.h"

TypeHandle FfmpegAudio::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: FfmpegAudio::Constructor
//       Access: Protected
//  Description: xxx
////////////////////////////////////////////////////////////////////
FfmpegAudio::
FfmpegAudio(const Filename &name) :
  MovieAudio(name),
  _specified_filename(name)
{
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
  return new FfmpegAudioCursor(this);
}

////////////////////////////////////////////////////////////////////

#endif // HAVE_FFMPEG
