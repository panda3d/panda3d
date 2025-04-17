/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file flacAudio.cxx
 * @author rdb
 * @date 2016-04-27
 * @author Stavros
 * @date 2025-03-05
 * 
 * @brief Updated FLAC functionality to use dr_flac v0.12.44 API.
 * 
 * This change updates the API calls to be compatible with the newer version
 * of dr_flac library, converting from sample-based to PCM frame-based operations
 * and adding support for the new memory allocation system.
 */

#include "flacAudio.h"
#include "flacAudioCursor.h"
#include "virtualFileSystem.h"
#include "dcast.h"

TypeHandle FlacAudio::_type_handle;

/**
 * xxx
 */
FlacAudio::
FlacAudio(const Filename &name) :
  MovieAudio(name)
{
  _filename = name;
}

/**
 * xxx
 */
FlacAudio::
~FlacAudio() {
}

/**
 * Open this audio, returning a MovieAudioCursor
 */
PT(MovieAudioCursor) FlacAudio::
open() {
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  std::istream *stream = vfs->open_read_file(_filename, true);

  if (stream == nullptr) {
    return nullptr;
  } else {
    PT(FlacAudioCursor) cursor = new FlacAudioCursor(this, stream);
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
PT(MovieAudio) FlacAudio::
make(const Filename &name) {
  return DCAST(MovieAudio, new FlacAudio(name));
}
