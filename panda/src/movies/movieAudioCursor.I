/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file movieAudioCursor.I
 * @author jyelon
 * @date 2007-07-02
 */

/**
 * Returns the MovieAudio which this cursor references.
 */
INLINE PT(MovieAudio) MovieAudioCursor::
get_source() const {
  return _source;
}

/**
 * Returns the audio sample rate.
 */
INLINE int MovieAudioCursor::
audio_rate() const {
  return _audio_rate;
}

/**
 * Returns the number of audio channels (ie, two for stereo, one for mono).
 */
INLINE int MovieAudioCursor::
audio_channels() const {
  return _audio_channels;
}

/**
 * Returns the length of the movie.  Attempting to read audio samples beyond
 * the specified length will produce silent samples.
 *
 * Some kinds of Movie, such as internet TV station, might not have a
 * predictable length.  In that case, the length will be set to a very large
 * number: 1.0E10.
 *
 * Some AVI files have incorrect length values encoded into them - they may be
 * a second or two long or short.  When playing such an AVI using the Movie
 * class, you may see a slightly truncated video, or a slightly elongated
 * video (padded with black frames).  There are utilities out there to fix the
 * length values in AVI files.
 *
 * An audio consumer needs to check the length, the ready status, and the
 * aborted flag.
 */
INLINE double MovieAudioCursor::
length() const {
  return _length;
}

/**
 * Returns true if the movie can seek.  If this is true, seeking is still not
 * guaranteed to be fast: for some movies, seeking is implemented by rewinding
 * to the beginning and then fast-forwarding to the desired location.  Even if
 * the movie cannot seek, the seek method can still advance to an arbitrary
 * location by reading samples and discarding them.  However, to move
 * backward, can_seek must return true.
 */
INLINE bool MovieAudioCursor::
can_seek() const {
  return _can_seek;
}

/**
 * Returns true if seek operations are constant time.
 */
INLINE bool MovieAudioCursor::
can_seek_fast() const {
  return _can_seek_fast;
}

/**
 * If aborted is true, it means that the "ready" samples are not being
 * replenished.  See the method "ready" for an explanation.
 */
INLINE bool MovieAudioCursor::
aborted() const {
  return _aborted;
}

/**
 * Returns the current offset within the file.
 */
INLINE double MovieAudioCursor::
tell() const {
  return _last_seek + ((_samples_read * 1.0) / _audio_rate);
}

/**
 * Skip audio samples from the stream.  This is mostly for debugging purposes.
 */
INLINE void MovieAudioCursor::
skip_samples(int n) {
  read_samples(n, (int16_t*)0);
}
