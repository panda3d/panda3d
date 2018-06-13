/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file opusAudio.cxx
 * @author rdb
 * @date 2017-05-24
 */

#include "opusAudio.h"
#include "opusAudioCursor.h"
#include "virtualFileSystem.h"
#include "dcast.h"

#ifdef HAVE_OPUS

TypeHandle OpusAudio::_type_handle;

/**
 * xxx
 */
OpusAudio::
OpusAudio(const Filename &name) :
  MovieAudio(name)
{
  _filename = name;
}

/**
 * xxx
 */
OpusAudio::
~OpusAudio() {
}

/**
 * Open this audio, returning a MovieAudioCursor
 */
PT(MovieAudioCursor) OpusAudio::
open() {
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  std::istream *stream = vfs->open_read_file(_filename, true);

  if (stream == nullptr) {
    return nullptr;
  } else {
    PT(OpusAudioCursor) cursor = new OpusAudioCursor(this, stream);
    if (cursor == nullptr || !cursor->_is_valid) {
      return nullptr;
    } else {
      return DCAST(MovieAudioCursor, cursor);
    }
  }
}

/**
 * Obtains a MovieAudio that references a file.
 */
PT(MovieAudio) OpusAudio::
make(const Filename &name) {
  return DCAST(MovieAudio, new OpusAudio(name));
}

#endif // HAVE_OPUS
