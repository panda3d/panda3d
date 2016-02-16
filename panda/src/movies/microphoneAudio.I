/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file microphoneAudio.I
 * @author jyelon
 * @date 2007-07-02
 */

/**
 * Returns the number of channels.
 */
INLINE int MicrophoneAudio::
get_channels() const {
  return _channels;
}

/**
 * Returns the sample rate.
 */
INLINE int MicrophoneAudio::
get_rate() const {
  return _rate;
}
