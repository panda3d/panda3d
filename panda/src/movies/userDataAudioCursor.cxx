// Filename: userDataAudioCursor.cxx
// Created by: jyelon (02Jul07)
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
