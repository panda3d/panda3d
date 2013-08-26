// Filename: wavAudio.cxx
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

#include "wavAudio.h"
#include "wavAudioCursor.h"
#include "virtualFileSystem.h"
#include "dcast.h"

TypeHandle WavAudio::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: WavAudio::Constructor
//       Access: Protected
//  Description: xxx
////////////////////////////////////////////////////////////////////
WavAudio::
WavAudio(const Filename &name) :
  MovieAudio(name)
{
  _filename = name;
}

////////////////////////////////////////////////////////////////////
//     Function: WavAudio::Destructor
//       Access: Protected, Virtual
//  Description: xxx
////////////////////////////////////////////////////////////////////
WavAudio::
~WavAudio() {
}

////////////////////////////////////////////////////////////////////
//     Function: WavAudio::open
//       Access: Published, Virtual
//  Description: Open this audio, returning a MovieAudioCursor
////////////////////////////////////////////////////////////////////
PT(MovieAudioCursor) WavAudio::
open() {
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  istream *stream = vfs->open_read_file(_filename, true);

  if (stream == NULL) {
    return NULL;
  } else {
    PT(WavAudioCursor) cursor = new WavAudioCursor(this, stream);
    if (cursor == NULL || !cursor->_is_valid) {
      return NULL;
    } else {
      return DCAST(MovieAudioCursor, cursor);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: WavAudio::make
//       Access: Published, Static
//  Description: Obtains a MovieAudio that references a file.
////////////////////////////////////////////////////////////////////
PT(MovieAudio) WavAudio::
make(const Filename &name) {
  return DCAST(MovieAudio, new WavAudio(name));
}
