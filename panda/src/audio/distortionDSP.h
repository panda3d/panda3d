/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file distortionDSP.h
 * @author lachbr
 * @date 2020-10-04
 */

#ifndef DISTORTIONDSP_H
#define DISTORTIONDSP_H

#include "dsp.h"

/**
 * DSP filter that distorts the sound.
 */
class EXPCL_PANDA_AUDIO DistortionDSP : public DSP {
PUBLISHED:
  INLINE DistortionDSP(float level = 0.5);

  INLINE void set_level(float level);
  INLINE float get_level() const;

private:
  float _level;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    DSP::init_type();
    register_type(_type_handle, "DistortionDSP",
                  DSP::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "distortionDSP.I"

#endif // DISTORTIONDSP_H
