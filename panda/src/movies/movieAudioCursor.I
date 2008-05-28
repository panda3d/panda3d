// Filename: movieAudioCursor.I
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
//  Description: Returns the length of the movie.  Attempting to read
//               audio samples beyond the specified length will produce 
//               silent samples.
//
//               Some kinds of Movie, such as internet TV station, 
//               might not have a predictable length.  In that case,
//               the length will be set to a very large number: 1.0E10.
//
//               Some AVI files have incorrect length values encoded
//               into them - they may be a second or two long or
//               short.  When playing such an AVI using the Movie class,
//               you may see a slightly truncated video, or a slightly
//               elongated video (padded with black frames).  There are
//               utilities out there to fix the length values in AVI
//               files.
//
//               An audio consumer needs to check the length, the
//               ready status, and the aborted flag.
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
//  Description: If aborted is true, it means that the "ready" samples
//               are not being replenished.  See the method "ready"
//               for an explanation.
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
  read_samples(n, (PN_int16*)0);
}
 
