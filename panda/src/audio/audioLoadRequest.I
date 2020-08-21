/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file audioLoadRequest.I
 * @author drose
 * @date 2006-08-29
 */

/**
 * Create a new AudioLoadRequest, and add it to the loader via load_async(),
 * to begin an asynchronous load.
 */
INLINE AudioLoadRequest::
AudioLoadRequest(AudioManager *audio_manager, const std::string &filename,
                 bool positional) :
  _audio_manager(audio_manager),
  _filename(filename),
  _positional(positional)
{
}

/**
 * Returns the AudioManager that will serve this asynchronous
 * AudioLoadRequest.
 */
INLINE AudioManager *AudioLoadRequest::
get_audio_manager() const {
  return _audio_manager;
}

/**
 * Returns the filename associated with this asynchronous AudioLoadRequest.
 */
INLINE const std::string &AudioLoadRequest::
get_filename() const {
  return _filename;
}

/**
 * Returns the positional flag associated with this asynchronous
 * AudioLoadRequest.
 */
INLINE bool AudioLoadRequest::
get_positional() const {
  return _positional;
}

/**
 * Returns true if this request has completed, false if it is still pending.
 * When this returns true, you may retrieve the sound loaded by calling
 * get_sound().
 * Equivalent to `req.done() and not req.cancelled()`.
 * @see done()
 */
INLINE bool AudioLoadRequest::
is_ready() const {
  return (FutureState)AtomicAdjust::get(_future_state) == FS_finished;
}

/**
 * Returns the sound that was loaded asynchronously, if any, or nullptr if
 * there was an error.  It is an error to call this unless done() returns
 * true.
 * @deprecated Use result() instead.
 */
INLINE AudioSound *AudioLoadRequest::
get_sound() const {
  nassertr_always(done(), nullptr);
  return (AudioSound *)_result;
}
