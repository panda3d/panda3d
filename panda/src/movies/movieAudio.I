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
//     Function: MovieAudio::audio_rate
//       Access: Public
//  Description: Returns the audio sample rate.
////////////////////////////////////////////////////////////////////
INLINE int MovieAudio::
audio_rate() const {
  return _audio_rate;
}

////////////////////////////////////////////////////////////////////
//     Function: MovieAudio::audio_channels
//       Access: Public
//  Description: Returns the number of audio channels (ie, two for
//               stereo, one for mono).
////////////////////////////////////////////////////////////////////
INLINE int MovieAudio::
audio_channels() const {
  return _audio_channels;
}

////////////////////////////////////////////////////////////////////
//     Function: MovieAudio::length
//       Access: Public
//  Description: Returns the length of the movie.
//
//               Some kinds of Movie, such as internet TV station, 
//               might not have a predictable length.  In that case,
//               the length will be set to a very large number: 1.0E10.
//               If the internet TV station goes offline, the video
//               or audio stream will set its abort flag.  Reaching the
//               end of the movie (ie, the specified length) normally
//               does not cause the abort flag to be set.
//
//               The video and audio streams produced by get_video and
//               get_audio are always of unlimited duration - you can
//               always read another video frame or another audio
//               sample.  This is true even if the specified length
//               is reached, or an abort is flagged. If either stream
//               runs out of data, it will synthesize blank video
//               frames and silent audio samples as necessary to
//               satisfy read requests.
//
//               Some AVI files have incorrect length values encoded
//               into them - usually, they're a second or two long or
//               short.  When playing such an AVI using the Movie class,
//               you may see a slightly truncated video, or a slightly
//               elongated video (padded with black frames).  There are
//               utilities out there to fix the length values in AVI
//               files.
//
////////////////////////////////////////////////////////////////////
INLINE double MovieAudio::
length() const {
  return _length;
}

////////////////////////////////////////////////////////////////////
//     Function: MovieAudio::can_seek
//       Access: Public
//  Description: See method 'seek' for an explanation.
////////////////////////////////////////////////////////////////////
INLINE bool MovieAudio::
can_seek() const {
  return _can_seek;
}

////////////////////////////////////////////////////////////////////
//     Function: MovieAudio::can_seek_zero
//       Access: Public
//  Description: See method 'seek' for an explanation.
////////////////////////////////////////////////////////////////////
INLINE bool MovieAudio::
can_seek_zero() const {
  return _can_seek_zero;
}

////////////////////////////////////////////////////////////////////
//     Function: MovieAudio::aborted
//       Access: Public
//  Description: Returns true if the audio has aborted prematurely.
//               For example, this could occur if the Movie was actually
//               an internet TV station, and the connection was lost.
//               Reaching the normal end of the audio does not
//               constitute an 'abort' condition.
////////////////////////////////////////////////////////////////////
INLINE bool MovieAudio::
aborted() const {
  return _aborted;
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
 
