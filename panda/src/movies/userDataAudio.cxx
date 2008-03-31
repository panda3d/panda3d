// Filename: userDataAudio.cxx
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

#include "userDataAudio.h"
#include "userDataAudioCursor.h"

TypeHandle UserDataAudio::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: UserDataAudio::Constructor
//       Access: Public
//  Description: This constructor returns a UserDataAudio --- 
//               a means to supply raw audio samples manually.
////////////////////////////////////////////////////////////////////
UserDataAudio::
UserDataAudio(int rate, int channels) :
  MovieAudio("User Data Audio"),
  _desired_rate(rate),
  _desired_channels(channels),
  _aborted(false),
  _cursor(NULL)
{
}

////////////////////////////////////////////////////////////////////
//     Function: UserDataAudio::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
UserDataAudio::
~UserDataAudio() {
}

////////////////////////////////////////////////////////////////////
//     Function: UserDataAudio::update_cursor
//       Access: Private
//  Description: Make sure that the UserDataAudioCursor's ready
//               and aborted status flags are correct.
////////////////////////////////////////////////////////////////////
void UserDataAudio::
update_cursor() {
  if (_cursor == 0) return;
  _cursor->_ready = _data.size();
  _cursor->_aborted = _aborted;
}

////////////////////////////////////////////////////////////////////
//     Function: UserDataAudio::open
//       Access: Published, Virtual
//  Description: Open this audio, returning a UserDataAudioCursor.  A
//               UserDataAudio can only be opened by one consumer
//               at a time.
////////////////////////////////////////////////////////////////////
PT(MovieAudioCursor) UserDataAudio::
open() {
  if (_cursor) {
    nassert_raise("A UserDataAudio can only be opened by one consumer at a time.");
    return NULL;
  }
  _cursor = new UserDataAudioCursor(this);
  update_cursor();
  return _cursor;
}

////////////////////////////////////////////////////////////////////
//     Function: UserDataAudio::read_samples
//       Access: Private
//  Description: Read audio samples from the stream.  N is the
//               number of samples you wish to read.  Your buffer
//               must be equal in size to N * channels.  
//               Multiple-channel audio will be interleaved. 
////////////////////////////////////////////////////////////////////
void UserDataAudio::
read_samples(int n, PN_int16 *data) {
  int nread = n;
  if (nread > (int)_data.size()) {
    nread = _data.size();
  }
  for (int i=0; i<nread; i++) {
    data[i] = _data[i];
  }
  for (int i=nread; i<n; i++) {
    data[i] = 0;
  }
  for (int i=0; i<nread; i++) {
    _data.pop_front();
  }
  update_cursor();
}

////////////////////////////////////////////////////////////////////
//     Function: UserDataAudio::append
//       Access: Published
//  Description: Appends audio samples to the buffer.
////////////////////////////////////////////////////////////////////
void UserDataAudio::
append(PN_int16 *data, int len) {
  nassertv(!_aborted);
  for (int i=0; i<len; i++) {
    _data.push_back(data[i]);
  }
  update_cursor();
}

////////////////////////////////////////////////////////////////////
//     Function: UserDataAudio::append
//       Access: Published
//  Description: Appends audio samples to the buffer.
////////////////////////////////////////////////////////////////////
void UserDataAudio::
append(int val) {
  nassertv(!_aborted);
  PN_int16 truncated = (PN_int16)val;
  nassertv(truncated == val);
  _data.push_back(truncated);
  update_cursor();
}

////////////////////////////////////////////////////////////////////
//     Function: UserDataAudio::done
//       Access: Published
//  Description: Promises not to append any more samples, ie, this
//               marks the end of the audio stream.
////////////////////////////////////////////////////////////////////
void UserDataAudio::
done() {
  _aborted = true;
  update_cursor();
}
