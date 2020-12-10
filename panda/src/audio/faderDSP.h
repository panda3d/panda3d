/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file faderDSP.h
 * @author lachbr
 * @date 2020-10-04
 */

#ifndef FADERDSP_H
#define FADERDSP_H

#include "dsp.h"

/**
 * DSP filter that pans and scales the volume of a unit.
 */
class EXPCL_PANDA_AUDIO FaderDSP : public DSP {
PUBLISHED:
  INLINE FaderDSP(float gain = 0);

  INLINE void set_gain(float gain);
  INLINE float get_gain() const;
  MAKE_PROPERTY(gain, get_gain, set_gain);

private:
  float _gain;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    DSP::init_type();
    register_type(_type_handle, "FaderDSP",
                  DSP::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "faderDSP.I"

#endif // FADERDSP_H
