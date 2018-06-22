/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file wavAudio.cxx
 * @author rdb
 * @date 2013-08-23
 */

#include "wavAudio.h"
#include "wavAudioCursor.h"
#include "virtualFileSystem.h"
#include "dcast.h"

TypeHandle WavAudio::_type_handle;

/**
 * xxx
 */
WavAudio::
WavAudio(const Filename &name) :
  MovieAudio(name)
{
  _filename = name;
}

/**
 * xxx
 */
WavAudio::
~WavAudio() {
}

/**
 * Open this audio, returning a MovieAudioCursor
 */
PT(MovieAudioCursor) WavAudio::
open() {
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  std::istream *stream = vfs->open_read_file(_filename, true);

  if (stream == nullptr) {
    return nullptr;
  } else {
    PT(WavAudioCursor) cursor = new WavAudioCursor(this, stream);
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
PT(MovieAudio) WavAudio::
make(const Filename &name) {
  return DCAST(MovieAudio, new WavAudio(name));
}
