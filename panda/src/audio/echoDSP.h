/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file echoDSP.h
 * @author lachbr
 * @date 2020-10-04
 */

#ifndef ECHODSP_H
#define ECHODSP_H

#include "dsp.h"

/**
 * DSP filter that produces an echo on the sound and fades out at the
 * desired rate.
 */
class EXPCL_PANDA_AUDIO EchoDSP : public DSP {
PUBLISHED:
  INLINE EchoDSP(float delay = 500, float feedback = 50, float drylevel = 0,
                 float wetlevel = 0);

  INLINE void set_delay(float delay);
  INLINE float get_delay() const;
  MAKE_PROPERTY(delay, get_delay, set_delay);

  INLINE void set_feedback(float feedback);
  INLINE float get_feedback() const;
  MAKE_PROPERTY(feedback, get_feedback, set_feedback);

  INLINE void set_drylevel(float drylevel);
  INLINE float get_drylevel() const;
  MAKE_PROPERTY(drylevel, get_drylevel, set_drylevel);

  INLINE void set_wetlevel(float wetlevel);
  INLINE float get_wetlevel() const;
  MAKE_PROPERTY(wetlevel, get_wetlevel, set_wetlevel);

private:
  float _delay;
  float _feedback;
  float _drylevel;
  float _wetlevel;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    DSP::init_type();
    register_type(_type_handle, "EchoDSP",
                  DSP::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "echoDSP.I"

#endif // ECHODSP_H
