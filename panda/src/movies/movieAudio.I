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
//     Function: MovieAudio::get_source
//       Access: Public
//  Description: Returns the source Movie.
////////////////////////////////////////////////////////////////////
INLINE CPT(Movie) MovieAudio::
get_source() const {
  return _source;
}

////////////////////////////////////////////////////////////////////
//     Function: MovieAudio::audio_rate
//       Access: Public
//  Description: See Movie::audio_rate.
////////////////////////////////////////////////////////////////////
INLINE int MovieAudio::
audio_rate() const {
  return _source->audio_rate();
}

////////////////////////////////////////////////////////////////////
//     Function: MovieAudio::audio_channels
//       Access: Public
//  Description: See Movie::audio_channels.
////////////////////////////////////////////////////////////////////
INLINE int MovieAudio::
audio_channels() const {
  return _source->audio_channels();
}

////////////////////////////////////////////////////////////////////
//     Function: MovieAudio::length
//       Access: Public
//  Description: See Movie::length.
////////////////////////////////////////////////////////////////////
INLINE double MovieAudio::
length() const {
  return _source->length();
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
//     Function: MovieAudio::skip_samples
//       Access: Published
//  Description: Skip audio samples from the stream.  This is mostly
//               for debugging purposes.
////////////////////////////////////////////////////////////////////
INLINE void MovieAudio::
skip_samples(int n) {
  read_samples(n, 0);
}
 
