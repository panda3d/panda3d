/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dsp.I
 * @author lachbr
 * @date 2020-10-04
 */

/**
 *
 */
INLINE DSP::
DSP(DSP::DSPType type) {
  _dsp_type = type;
  _dirty = true;
}

/**
 * Returns the DSP filter type.
 */
INLINE DSP::DSPType DSP::
get_dsp_type() const {
  return _dsp_type;
}

/**
 * Marks the DSP as dirty, meaning the parameter values have changed and need
 * to be updated by the AudioManager.
 */
INLINE void DSP::
on_values_changed() {
  _dirty = true;
}

/**
 * Returns true if the DSP parameter values have changed and need to be updated
 * by the AudioManager.
 */
INLINE bool DSP::
is_dirty() const {
  return _dirty;
}

/**
 * Clears the dirty flag.  This should be called by the AudioManager after they
 * have updated the parameter values to the audio API.
 */
INLINE void DSP::
clear_dirty() {
  _dirty = false;
}
