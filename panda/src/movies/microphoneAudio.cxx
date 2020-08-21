/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file microphoneAudio.cxx
 * @author jyelon
 * @date 2007-07-02
 */

#include "microphoneAudio.h"
#include "movieAudioCursor.h"

pvector<PT(MicrophoneAudio)> MicrophoneAudio::_all_microphones;
TypeHandle MicrophoneAudio::_type_handle;

/**
 *
 */
MicrophoneAudio::
~MicrophoneAudio() {
}

/**
 * Scans the hardware for microphones, and pushes them onto the global list of
 * all microphones.
 *
 * There are several implementations of MicrophoneAudio, including one based
 * on DirectShow, one based on Linux ALSA, and so forth.  These
 * implementations are contained in one C++ file each, and they export nothing
 * at all except a single "find_all" function.  Otherwise, they can only be
 * accessed through the virtual methods of the MicrophoneAudio objects they
 * create.
 */
void MicrophoneAudio::
find_all_microphones() {
  static bool initialized = false;
  if (initialized) return;
  initialized = true;

#ifdef HAVE_DIRECTCAM
  extern void find_all_microphones_ds();
  find_all_microphones_ds();
#endif

#ifdef HAVE_ALSA
  extern void find_all_microphones_alsa();
  find_all_microphones_alsa();
#endif
}

/**
 * Returns the number of microphone options.  An "option" consists of a device
 * plus a set of configuration parameters.  For example, "Soundblaster Audigy
 * Line in at 44,100 samples/sec" would be an option.
 */
int MicrophoneAudio::
get_num_options() {
  find_all_microphones();
  return _all_microphones.size();
}

/**
 * Returns the nth microphone option.
 */
PT(MicrophoneAudio) MicrophoneAudio::
get_option(int n) {
  find_all_microphones();
  nassertr((n >= 0) && (n < (int)_all_microphones.size()), nullptr);
  return _all_microphones[n];
}
