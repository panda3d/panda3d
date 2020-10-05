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

INLINE DSP::
DSP(DSP::DSPType type) {
  _dsp_type = type;
}

INLINE DSP::DSPType DSP::
get_dsp_type() const {
  return _dsp_type;
}
