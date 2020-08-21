/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file ffmpegAudio.cxx
 * @author jyelon
 * @date 2007-08-01
 */

#include "config_ffmpeg.h"
#include "ffmpegAudio.h"
#include "ffmpegAudioCursor.h"
#include "dcast.h"

TypeHandle FfmpegAudio::_type_handle;

/**
 * xxx
 */
FfmpegAudio::
FfmpegAudio(const Filename &name) :
  MovieAudio(name)
{
  _filename = name;
}

/**
 * xxx
 */
FfmpegAudio::
~FfmpegAudio() {
}

/**
 * Open this audio, returning a MovieAudioCursor
 */
PT(MovieAudioCursor) FfmpegAudio::
open() {
  PT(FfmpegAudioCursor) result = new FfmpegAudioCursor(this);
  if (result->_format_ctx == nullptr) {
    ffmpeg_cat.error() << "Could not open " << _filename << "\n";
    return nullptr;
  } else {
    return (MovieAudioCursor*)(FfmpegAudioCursor*)result;
  }
}

/**
 * Obtains a MovieAudio that references a file.
 */
PT(MovieAudio) FfmpegAudio::
make(const Filename &name) {
  return DCAST(MovieAudio, new FfmpegAudio(name));
}
