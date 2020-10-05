/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lowpassDSP.h
 * @author lachbr
 * @date 2020-10-04
 */

#ifndef LOWPASSDSP_H
#define LOWPASSDSP_H

#include "dsp.h"

/**
 * DSP filter that filters sound using a high quality, resonant lowpass filter.
 */
class EXPCL_PANDA_AUDIO LowpassDSP : public DSP {
PUBLISHED:
  INLINE LowpassDSP(float cutoff = 5000, float resonance = 1);

  INLINE void set_cutoff(float cutoff);
  INLINE float get_cutoff() const;
  MAKE_PROPERTY(cutoff, get_cutoff, set_cutoff);

  INLINE void set_resonance(float resonance);
  INLINE float get_resonance() const;
  MAKE_PROPERTY(resonance, get_resonance, set_resonance);

private:
  float _cutoff;
  float _resonance;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    DSP::init_type();
    register_type(_type_handle, "LowpassDSP",
                  DSP::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "lowpassDSP.I"

#endif // LOWPASSDSP_H
