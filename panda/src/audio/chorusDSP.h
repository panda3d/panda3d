/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file chorusDSP.h
 * @author lachbr
 * @date 2020-10-04
 */

#ifndef CHORUSDSP_H
#define CHORUSDSP_H

#include "dsp.h"

/**
 * DSP filter that produces a chorus effect on the sound.
 */
class EXPCL_PANDA_AUDIO ChorusDSP : public DSP {
PUBLISHED:
  INLINE ChorusDSP(float mix = 50, float rate = 0.8, float depth = 3);

  INLINE void set_mix(float mix);
  INLINE float get_mix() const;
  MAKE_PROPERTY(mix, get_mix, set_mix);

  INLINE void set_rate(float rate);
  INLINE float get_rate() const;
  MAKE_PROPERTY(rate, get_rate, set_rate);

  INLINE void set_depth(float depth);
  INLINE float get_depth() const;
  MAKE_PROPERTY(depth, get_depth, set_depth);

private:
  float _mix;
  float _rate;
  float _depth;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    DSP::init_type();
    register_type(_type_handle, "ChorusDSP",
                  DSP::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "chorusDSP.I"

#endif // CHORUSDSP_H
