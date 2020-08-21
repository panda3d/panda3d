/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file movieAudioCursor.cxx
 * @author jyelon
 * @date 2007-07-02
 */

#include "movieAudioCursor.h"

TypeHandle MovieAudioCursor::_type_handle;

/**
 * This constructor returns a null audio stream --- a stream of total silence,
 * at 8000 samples per second.  To get more interesting audio, you need to
 * construct a subclass of this class.
 */
MovieAudioCursor::
MovieAudioCursor(MovieAudio *src) :
  _source(src),
  _audio_rate(8000),
  _audio_channels(1),
  _length(1.0E10),
  _can_seek(true),
  _can_seek_fast(true),
  _aborted(false),
  _samples_read(0)
{
}

/**
 *
 */
MovieAudioCursor::
~MovieAudioCursor() {
}

/**
 * Read audio samples from the stream.  N is the number of samples you wish to
 * read.  Your buffer must be equal in size to N * channels.  Multiple-channel
 * audio will be interleaved.
 */
int MovieAudioCursor::
read_samples(int n, int16_t *data) {

  // This is the null implementation, which generates pure silence.  Normally,
  // this method will be overridden by a subclass.

  if (n <= 0) {
    return 0;
  }

  int desired = n * _audio_channels;
  for (int i=0; i<desired; i++) {
    data[i] = 0;
  }
  _samples_read += n;
  return n;
}

/**
 * Read audio samples from the stream into a Datagram.  N is the number of
 * samples you wish to read.  Multiple-channel audio will be interleaved.
 *
 * This is not particularly efficient, but it may be a convenient way to
 * manipulate samples in python.
 */
void MovieAudioCursor::
read_samples(int n, Datagram *dg) {
  int16_t tmp[4096];
  while (n > 0) {
    int blocksize = (4096 / _audio_channels);
    if (blocksize > n) blocksize = n;
    int words = blocksize * _audio_channels;
    read_samples(blocksize, tmp);
    for (int i=0; i<words; i++) {
      dg->add_int16(tmp[i]);
    }
    n -= blocksize;
  }
}

/**
 * Read audio samples from the stream and returns them as a string.  The
 * samples are stored little-endian in the string.  N is the number of samples
 * you wish to read.  Multiple-channel audio will be interleaved.
 *
 * This is not particularly efficient, but it may be a convenient way to
 * manipulate samples in python.
 */
vector_uchar MovieAudioCursor::
read_samples(int n) {
  vector_uchar result;
  int16_t tmp[4096];
  while (n > 0) {
    int blocksize = (4096 / _audio_channels);
    if (blocksize > n) {
      blocksize = n;
    }
    int nread = read_samples(blocksize, tmp);
    if (nread == 0) {
      return result;
    }
    int words = nread * _audio_channels;
    for (int i = 0; i < words; ++i) {
      int16_t word = tmp[i];
      result.push_back((uint8_t)(word & 255u));
      result.push_back((uint8_t)((word >> 8) & 255u));
    }
    n -= nread;
  }
  return result;
}


/**
 * Skips to the specified offset within the file.
 *
 * If the movie reports that it cannot seek, then this method can still
 * advance by reading samples and discarding them.  However, to move backward,
 * can_seek must be true.
 *
 * If the movie reports that it can_seek, it doesn't mean that it can do so
 * quickly.  It may have to rewind the movie and then fast forward to the
 * desired location.  Only if can_seek_fast returns true can seek operations
 * be done in constant time.
 *
 * Seeking may not be precise, because AVI files often have inaccurate
 * indices.  After seeking, tell will indicate that the cursor is at the
 * target location.  However, in truth, the data you read may come from a
 * slightly offset location.
 */
void MovieAudioCursor::
seek(double offset) {
  _last_seek = offset;
  _samples_read = 0;
}

/**
 * Returns the number of audio samples that are ready to read.  This is
 * primarily relevant for sources like microphones which produce samples at a
 * fixed rate.  If you try to read more samples than are ready, the result
 * will be silent samples.
 *
 * Some audio streams do not have a limit on how fast they can produce
 * samples.  Such streams will always return 0x40000000 as the ready-count.
 * This may well exceed the length of the audio stream.  You therefore need to
 * check length separately.
 *
 * If the aborted flag is set, that means the ready count is no longer being
 * replenished.  For example, a MovieAudioCursor might be reading from an
 * internet radio station, and it might buffer data to avoid underruns.  If it
 * loses connection to the radio station, it will set the aborted flag to
 * indicate that the buffer is no longer being replenished.  But it is still
 * ok to read the samples that are in the buffer, at least until they run out.
 * Once those are gone, there will be no more.
 *
 * An audio consumer needs to check the length, the ready status, and the
 * aborted flag.
 */
int MovieAudioCursor::
ready() const {
  return 0x40000000;
}
