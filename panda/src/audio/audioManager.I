/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file audioManager.I
 * @author jyelon
 * @date 2007-08-01
 */

/**
 * Adds the specified DSP filter to the beginning of the DSP chain.  Returns
 * true if the DSP filter is supported by the audio implementation, false
 * otherwise.
 */
bool AudioManager::
add_dsp_to_head(DSP *dsp) {
  return insert_dsp(0, dsp);
}

/**
 * Adds the specified DSP filter to the end of the DSP chain.  Returns
 * true if the DSP filter is supported by the audio implementation, false
 * otherwise.
 */
bool AudioManager::
add_dsp_to_tail(DSP *dsp) {
  return insert_dsp(get_num_dsps() - 1, dsp);
}
