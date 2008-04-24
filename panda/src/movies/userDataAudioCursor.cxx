// Filename: userDataAudioCursor.cxx
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
// panda3d-general@lists.sourceforge.net 
//
////////////////////////////////////////////////////////////////////

#include "userDataAudioCursor.h"

TypeHandle UserDataAudioCursor::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: UserDataAudioCursor::Constructor
//       Access: 
//  Description: 
////////////////////////////////////////////////////////////////////
UserDataAudioCursor::
UserDataAudioCursor(UserDataAudio *src) :
  MovieAudioCursor(src)
{
  _audio_rate = src->_desired_rate;
  _audio_channels = src->_desired_channels;
  _can_seek = false;
  _can_seek_fast = false;
  _aborted = false;
}

////////////////////////////////////////////////////////////////////
//     Function: UserDataAudioCursor::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
UserDataAudioCursor::
~UserDataAudioCursor() {
  UserDataAudio *source = (UserDataAudio*)(MovieAudio*)_source;
  source->_cursor = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: UserDataAudioCursor::read_samples
//       Access: Private
//  Description: Read audio samples from the stream.  N is the
//               number of samples you wish to read.  Your buffer
//               must be equal in size to N * channels.  
//               Multiple-channel audio will be interleaved. 
////////////////////////////////////////////////////////////////////
void UserDataAudioCursor::
read_samples(int n, PN_int16 *data) {
  UserDataAudio *source = (UserDataAudio*)(MovieAudio*)_source;
  source->read_samples(n, data);
  _samples_read += n;
}

////////////////////////////////////////////////////////////////////
//     Function: UserDataAudioCursor::ready
//       Access: Private
//  Description: Returns the number of audio samples ready to be
//               read.
////////////////////////////////////////////////////////////////////
int UserDataAudioCursor::
ready() const {
  UserDataAudio *source = (UserDataAudio*)(MovieAudio*)_source;
  ((UserDataAudioCursor*)this)->_aborted = source->_aborted;
  return (source->_data.size()) / _audio_channels;
}
