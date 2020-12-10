/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file normalizeDSP.h
 * @author lachbr
 * @date 2020-10-08
 */

#ifndef NORMALIZEDSP_H
#define NORMALIZEDSP_H

#include "dsp.h"

/**
 * DSP filter that normalizes or amplifies the sound to a certain level.
 */
class EXPCL_PANDA_AUDIO NormalizeDSP : public DSP {
PUBLISHED:
  INLINE NormalizeDSP(float fade_time = 5000, float threshold = 0.1,
                      float max_amp = 20);

  INLINE void set_fade_time(float fade_time);
  INLINE float get_fade_time() const;
  MAKE_PROPERTY(fade_time, get_fade_time, set_fade_time);

  INLINE void set_threshold(float threshold);
  INLINE float get_threshold() const;
  MAKE_PROPERTY(threshold, get_threshold, set_threshold);

  INLINE void set_max_amp(float max_amp);
  INLINE float get_max_amp() const;
  MAKE_PROPERTY(max_amp, get_max_amp, set_max_amp);

private:
  float _fade_time;
  float _threshold;
  float _max_amp;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    DSP::init_type();
    register_type(_type_handle, "NormalizeDSP",
                  DSP::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "normalizeDSP.I"

#endif // NORMALIZEDSP_H
