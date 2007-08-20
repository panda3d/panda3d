// Filename: movieAudioCursor.I
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
//     Function: MovieAudioCursor::get_source
//       Access: Public
//  Description: Returns the MovieAudio which this cursor references.
////////////////////////////////////////////////////////////////////
INLINE PT(MovieAudio) MovieAudioCursor::
get_source() const {
  return _source;
}

////////////////////////////////////////////////////////////////////
//     Function: MovieAudioCursor::audio_rate
//       Access: Public
//  Description: Returns the audio sample rate.
////////////////////////////////////////////////////////////////////
INLINE int MovieAudioCursor::
audio_rate() const {
  return _audio_rate;
}

////////////////////////////////////////////////////////////////////
//     Function: MovieAudioCursor::audio_channels
//       Access: Public
//  Description: Returns the number of audio channels (ie, two for
//               stereo, one for mono).
////////////////////////////////////////////////////////////////////
INLINE int MovieAudioCursor::
audio_channels() const {
  return _audio_channels;
}

////////////////////////////////////////////////////////////////////
//     Function: MovieAudioCursor::length
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
INLINE double MovieAudioCursor::
length() const {
  return _length;
}

////////////////////////////////////////////////////////////////////
//     Function: MovieAudioCursor::can_seek
//       Access: Public
//  Description: Returns true if the movie can seek.  If this is
//               true, seeking is still not guaranteed to be fast:
//               for some movies, seeking is implemented by rewinding
//               to the beginning and then fast-forwarding to the
//               desired location.  Even if the movie cannot seek,
//               the seek method can still advance to an arbitrary
//               location by reading samples and discarding them.
//               However, to move backward, can_seek must return true.
////////////////////////////////////////////////////////////////////
INLINE bool MovieAudioCursor::
can_seek() const {
  return _can_seek;
}

////////////////////////////////////////////////////////////////////
//     Function: MovieAudioCursor::can_seek_fast
//       Access: Public
//  Description: Returns true if seek operations are constant time.
////////////////////////////////////////////////////////////////////
INLINE bool MovieAudioCursor::
can_seek_fast() const {
  return _can_seek_fast;
}

////////////////////////////////////////////////////////////////////
//     Function: MovieAudioCursor::aborted
//       Access: Public
//  Description: Returns true if the audio has aborted prematurely.
//               For example, this could occur if the Movie was actually
//               an internet TV station, and the connection was lost.
//               Reaching the normal end of the audio does not
//               constitute an 'abort' condition.
////////////////////////////////////////////////////////////////////
INLINE bool MovieAudioCursor::
aborted() const {
  return _aborted;
}

////////////////////////////////////////////////////////////////////
//     Function: MovieAudioCursor::tell
//       Access: Public
//  Description: Returns the current offset within the file.
////////////////////////////////////////////////////////////////////
INLINE double MovieAudioCursor::
tell() const {
  return _last_seek + ((_samples_read * 1.0) / _audio_rate);
}

////////////////////////////////////////////////////////////////////
//     Function: MovieAudioCursor::skip_samples
//       Access: Published
//  Description: Skip audio samples from the stream.  This is mostly
//               for debugging purposes.
////////////////////////////////////////////////////////////////////
INLINE void MovieAudioCursor::
skip_samples(int n) {
  read_samples(n, 0);
}
 
