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
 * @date 2020-10-04
 */

#ifndef LIMITERDSP_H
#define LIMITERDSP_H

#include "dsp.h"

/**
 * DSP filter that limits the sound to a certain level.
 */
class EXPCL_PANDA_AUDIO LimiterDSP : public DSP {
PUBLISHED:
  INLINE LimiterDSP(float release_time = 10, float ceiling = 0,
                    float maximizer_gain = 0);

  INLINE void set_release_time(float time);
  INLINE float get_release_time() const;
  MAKE_PROPERTY(release_time, get_release_time, set_release_time);

  INLINE void set_ceiling(float ceiling);
  INLINE float get_ceiling() const;
  MAKE_PROPERTY(ceiling, get_ceiling, set_ceiling);

  INLINE void set_maximizer_gain(float gain);
  INLINE float get_maximizer_gain() const;
  MAKE_PROPERTY(maximizer_gain, get_maximizer_gain, set_maximizer_gain);

private:
  float _release_time;
  float _ceiling;
  float _maximizer_gain;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    DSP::init_type();
    register_type(_type_handle, "LimiterDSP",
                  DSP::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "limiterDSP.I"

#endif // LIMITERDSP_H
