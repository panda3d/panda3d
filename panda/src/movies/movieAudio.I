// Filename: movieAudio.I
// Created by: jyelon (02Jul07)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: MovieAudio::rate
//       Access: Public
//  Description: Returns the audio sample rate, in samples per sec.
////////////////////////////////////////////////////////////////////
INLINE int MovieAudio::
rate() const {
  return _rate;
}

////////////////////////////////////////////////////////////////////
//     Function: MovieAudio::channels
//       Access: Public
//  Description: Returns the number of audio channels.  Ie, 1 for
//               mono, 2 for stereo.
////////////////////////////////////////////////////////////////////
INLINE int MovieAudio::
channels() const {
  return _channels;
}

////////////////////////////////////////////////////////////////////
//     Function: MovieAudio::samples_read
//       Access: Public
//  Description: Returns the number of audio samples you have read
//               from the audio stream so far.
////////////////////////////////////////////////////////////////////
INLINE int MovieAudio::
samples_read() const {
  return _samples_read;
}

////////////////////////////////////////////////////////////////////
//     Function: MovieAudio::approx_time_remaining
//       Access: Public
//  Description: Returns a "best guess" amount of time left on the
//               stream. For ffmpeg streams, this is usually a
//               pretty close approximation --- ie, off by a few
//               hundred samples.  For infinite streams, always
//               returns the constant value 1.0E10.
////////////////////////////////////////////////////////////////////
INLINE double MovieAudio::
approx_time_remaining() const {
  return _approx_time_remaining;
}

