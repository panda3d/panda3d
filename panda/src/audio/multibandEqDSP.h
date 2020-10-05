/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file limiterDSP.h
 * @author lachbr
 * @date 2020-10-05
 */

#ifndef MULTIBANDEQDSP_H
#define MULTIBANDEQDSP_H

#include "dsp.h"

/**
 * Five band parametric equalizer.
 */
class EXPCL_PANDA_AUDIO MultibandEqDSP : public DSP {
PUBLISHED:
  enum FilterType {
    FT_disabled,
    FT_lowpass_12db,
    FT_lowpass_24db,
    FT_lowpass_48db,
    FT_highpass_12db,
    FT_highpass_24db,
    FT_highpass_48db,
    FT_lowshelf,
    FT_highshelf,
    FT_peaking,
    FT_bandpass,
    FT_notch,
    FT_allpass,
  };

  //INLINE MultibandEqDSP();

  //void set_
};

#endif // MULTIBANDEQDSP_H
