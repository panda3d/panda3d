/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file distortionDSP.I
 * @author lachbr
 * @date 2020-10-04
 */

/**
 *
 */
INLINE DistortionDSP::
DistortionDSP(float level) :
  DSP(DSP::DT_distortion),
  _level(level) {
}

/**
 * Sets the distortion level.
 */
INLINE void DistortionDSP::
set_level(float level) {
  _level = level;
}

/**
 * Returns the distortion level.
 */
INLINE float DistortionDSP::
get_level() const {
  return _level;
}
