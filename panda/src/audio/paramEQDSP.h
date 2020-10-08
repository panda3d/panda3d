/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file paramEQDSP.h
 * @author lachbr
 * @date 2020-10-08
 */

#ifndef PARAMEQDSP_H
#define PARAMEQDSP_H

#include "dsp.h"

/**
 * DSP filter that attenuates or amplifies a selected frequency range.
 */
class EXPCL_PANDA_AUDIO ParamEQDSP : public DSP {
PUBLISHED:
  INLINE ParamEQDSP(float center = 8000, float bandwith = 1, float gain = 0);

  INLINE void set_center(float center);
  INLINE float get_center() const;
  MAKE_PROPERTY(center, get_center, set_center);

  INLINE void set_bandwith(float bandwith);
  INLINE float get_bandwith() const;
  MAKE_PROPERTY(bandwith, get_bandwith, set_bandwith);

  INLINE void set_gain(float gain);
  INLINE float get_gain() const;
  MAKE_PROPERTY(gain, get_gain, set_gain);

private:
  float _center;
  float _bandwith;
  float _gain;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    DSP::init_type();
    register_type(_type_handle, "ParamEQDSP",
                  DSP::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "paramEQDSP.I"

#endif // PARAMEQDSP_H
