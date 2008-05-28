// Filename: microphoneAudio.I
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

////////////////////////////////////////////////////////////////////
//     Function: MicrophoneAudio::get_channels
//       Access: Published
//  Description: Returns the number of channels.
////////////////////////////////////////////////////////////////////
INLINE int MicrophoneAudio::
get_channels() const {
  return _channels;
}

////////////////////////////////////////////////////////////////////
//     Function: MicrophoneAudio::get_rate
//       Access: Published
//  Description: Returns the sample rate.
////////////////////////////////////////////////////////////////////
INLINE int MicrophoneAudio::
get_rate() const {
  return _rate;
}

