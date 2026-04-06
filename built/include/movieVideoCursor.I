/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file movieVideoCursor.I
 * @author jyelon
 * @date 2007-07-02
 */

/**
 * Get the MovieVideo which this cursor references.
 */
INLINE PT(MovieVideo) MovieVideoCursor::
get_source() const {
  return _source;
}

/**
 * Get the horizontal size of the movie.
 */
INLINE int MovieVideoCursor::
size_x() const {
  return _size_x;
}

/**
 * Get the vertical size of the movie.
 */
INLINE int MovieVideoCursor::
size_y() const {
  return _size_y;
}

/**
 * Returns 4 if the movie has an alpha channel, 3 otherwise.
 */
INLINE int MovieVideoCursor::
get_num_components() const {
  return _num_components;
}

/**
 * Returns the length of the movie.
 *
 * Some kinds of Movie, such as internet TV station, might not have a
 * predictable length.  In that case, the length will be set to a very large
 * number: 1.0E10. If the internet TV station goes offline, the video or audio
 * stream will set its abort flag.  Reaching the end of the movie (ie, the
 * specified length) normally does not cause the abort flag to be set.
 *
 * The video and audio streams produced by get_video and get_audio are always
 * of unlimited duration - you can always read another video frame or another
 * audio sample.  This is true even if the specified length is reached, or an
 * abort is flagged.  If either stream runs out of data, it will synthesize
 * blank video frames and silent audio samples as necessary to satisfy read
 * requests.
 *
 * Some AVI files have incorrect length values encoded into them - usually,
 * they're a second or two long or short.  When playing such an AVI using the
 * Movie class, you may see a slightly truncated video, or a slightly
 * elongated video (padded with black frames).  There are utilities out there
 * to fix the length values in AVI files.
 *
 */
INLINE double MovieVideoCursor::
length() const {
  return _length;
}

/**
 * Returns true if the movie can seek.  If this is true, seeking is still not
 * guaranteed to be fast: for some movies, seeking is implemented by rewinding
 * to the beginning and then fast-forwarding to the desired location.  Even if
 * the movie cannot seek, the fetch methods can still advance to an arbitrary
 * location by reading frames and discarding them.  However, to move backward,
 * can_seek must return true.
 */
INLINE bool MovieVideoCursor::
can_seek() const {
  return _can_seek;
}

/**
 * Returns true if seek operations are constant time.
 */
INLINE bool MovieVideoCursor::
can_seek_fast() const {
  return _can_seek_fast;
}

/**
 * Returns true if the video has aborted prematurely.  For example, this could
 * occur if the Movie was actually an internet TV station, and the connection
 * was lost.  Reaching the normal end of the video does not constitute an
 * 'abort' condition.
 */
INLINE bool MovieVideoCursor::
aborted() const {
  return _aborted;
}

/**
 * Returns true if the video frames are being "pushed" at us by something that
 * operates at its own speed - for example, a webcam.  In this case, the
 * frames come when they're ready to come.  Attempting to read too soon will
 * produce nothing, reading too late will cause frames to be dropped.  In this
 * case, the ready flag can be used to determine whether or not a frame is
 * ready for reading.
 *
 * When streaming, you should still pay attention to last_start, but the value
 * of next_start is only a guess.
 */
INLINE bool MovieVideoCursor::
streaming() const {
  return _streaming;
}

/**
 * Returns true if the cursor is a streaming source, and if a video frame is
 * ready to be read.  For non- streaming sources, this is always false.
 */
INLINE bool MovieVideoCursor::
ready() const {
  return _ready;
}
