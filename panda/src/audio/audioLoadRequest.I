// Filename: audioLoadRequest.I
// Created by:  drose (29Aug06)
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


////////////////////////////////////////////////////////////////////
//     Function: AudioLoadRequest::Constructor
//       Access: Published
//  Description: Create a new AudioLoadRequest, and add it to the loader
//               via load_async(), to begin an asynchronous load.
////////////////////////////////////////////////////////////////////
INLINE AudioLoadRequest::
AudioLoadRequest(AudioManager *audio_manager, const string &filename, 
                 bool positional) :
  _audio_manager(audio_manager),
  _filename(filename),
  _positional(positional),
  _is_ready(false)
{
}

////////////////////////////////////////////////////////////////////
//     Function: AudioLoadRequest::get_audio_manager
//       Access: Published
//  Description: Returns the AudioManager that will serve this
//               asynchronous AudioLoadRequest.
////////////////////////////////////////////////////////////////////
INLINE AudioManager *AudioLoadRequest::
get_audio_manager() const {
  return _audio_manager;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioLoadRequest::get_filename
//       Access: Published
//  Description: Returns the filename associated with this
//               asynchronous AudioLoadRequest.
////////////////////////////////////////////////////////////////////
INLINE const string &AudioLoadRequest::
get_filename() const {
  return _filename;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioLoadRequest::get_positional
//       Access: Published
//  Description: Returns the positional flag associated with this
//               asynchronous AudioLoadRequest.
////////////////////////////////////////////////////////////////////
INLINE bool AudioLoadRequest::
get_positional() const {
  return _positional;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioLoadRequest::is_ready
//       Access: Published
//  Description: Returns true if this request has completed, false if
//               it is still pending.  When this returns true, you may
//               retrieve the sound loaded by calling get_sound().
////////////////////////////////////////////////////////////////////
INLINE bool AudioLoadRequest::
is_ready() const {
  return _is_ready;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioLoadRequest::get_sound
//       Access: Published
//  Description: Returns the sound that was loaded asynchronously, if
//               any, or NULL if there was an error.  It is an error
//               to call this unless is_ready() returns true.
////////////////////////////////////////////////////////////////////
INLINE AudioSound *AudioLoadRequest::
get_sound() const {
  nassertr(_is_ready, NULL);
  return _sound;
}
