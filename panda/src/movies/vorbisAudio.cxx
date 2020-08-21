/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vorbisAudio.cxx
 * @author rdb
 * @date 2013-08-23
 */

#include "vorbisAudio.h"
#include "vorbisAudioCursor.h"
#include "virtualFileSystem.h"
#include "dcast.h"

#ifdef HAVE_VORBIS

TypeHandle VorbisAudio::_type_handle;

/**
 * xxx
 */
VorbisAudio::
VorbisAudio(const Filename &name) :
  MovieAudio(name)
{
  _filename = name;
}

/**
 * xxx
 */
VorbisAudio::
~VorbisAudio() {
}

/**
 * Open this audio, returning a MovieAudioCursor
 */
PT(MovieAudioCursor) VorbisAudio::
open() {
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  std::istream *stream = vfs->open_read_file(_filename, true);

  if (stream == nullptr) {
    return nullptr;
  } else {
    PT(VorbisAudioCursor) cursor = new VorbisAudioCursor(this, stream);
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
PT(MovieAudio) VorbisAudio::
make(const Filename &name) {
  return DCAST(MovieAudio, new VorbisAudio(name));
}

#endif // HAVE_VORBIS
