// Filename: vorbisAudio.cxx
// Created by: rdb (23Aug13)
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

#include "vorbisAudio.h"
#include "vorbisAudioCursor.h"
#include "virtualFileSystem.h"
#include "dcast.h"

#ifdef HAVE_VORBIS

TypeHandle VorbisAudio::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: VorbisAudio::Constructor
//       Access: Protected
//  Description: xxx
////////////////////////////////////////////////////////////////////
VorbisAudio::
VorbisAudio(const Filename &name) :
  MovieAudio(name)
{
  _filename = name;
}

////////////////////////////////////////////////////////////////////
//     Function: VorbisAudio::Destructor
//       Access: Protected, Virtual
//  Description: xxx
////////////////////////////////////////////////////////////////////
VorbisAudio::
~VorbisAudio() {
}

////////////////////////////////////////////////////////////////////
//     Function: VorbisAudio::open
//       Access: Published, Virtual
//  Description: Open this audio, returning a MovieAudioCursor
////////////////////////////////////////////////////////////////////
PT(MovieAudioCursor) VorbisAudio::
open() {
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  istream *stream = vfs->open_read_file(_filename, true);

  if (stream == NULL) {
    return NULL;
  } else {
    PT(VorbisAudioCursor) cursor = new VorbisAudioCursor(this, stream);
    if (cursor == NULL || !cursor->_is_valid) {
      return NULL;
    } else {
      return DCAST(MovieAudioCursor, cursor);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: VorbisAudio::make
//       Access: Published, Static
//  Description: Obtains a MovieAudio that references a file.
////////////////////////////////////////////////////////////////////
PT(MovieAudio) VorbisAudio::
make(const Filename &name) {
  return DCAST(MovieAudio, new VorbisAudio(name));
}

#endif // HAVE_VORBIS
