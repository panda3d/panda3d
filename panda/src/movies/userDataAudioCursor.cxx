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
  _can_seek = !src->_remove_after_read;
  _can_seek_fast = !src->_remove_after_read;
  _aborted = false;
  if(!src->_remove_after_read) {
    assert(src->_aborted && "UserData was not closed before by a done() call");
    _length = static_cast<double>(src->_data.size() / _audio_channels) / _audio_rate;
  }
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
  
  if(source->_remove_after_read) {
    source->read_samples(n, data);
  }
  else {
    int offset = _samples_read * _audio_channels;
    int avail = source->_data.size() - offset;
    int desired = n * _audio_channels;
    if (avail > desired) avail = desired;

    for (int i=0; i<avail; i++) {
      data[i] = source->_data[i+offset];
    }
    for (int i=avail; i<desired; i++) {
      data[i] = 0;
    }
  }

  _samples_read += n;
}

////////////////////////////////////////////////////////////////////
//     Function: UserDataAudioCursor::ready
//       Access: Published
//  Description: Set the offset if possible.
////////////////////////////////////////////////////////////////////
void UserDataAudioCursor::
seek(double t) {
  if(_can_seek && 0 <= t && _length <= t) {
    _samples_read = static_cast<int>(t * _audio_rate * _audio_channels + 0.5f);
  }
  else {
    _samples_read = 0;
  }
  _last_seek = t;
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

  if(source->_remove_after_read) return source->_data.size() / _audio_channels;
  else                     return source->_data.size() / _audio_channels - _samples_read;
}
