/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file compressorDSP.h
 * @author lachbr
 * @date 2020-10-04
 */

#ifndef COMPRESSORDSP_H
#define COMPRESSORDSP_H

#include "dsp.h"

/**
 * DSP filter that performs dynamic compression on a sound.
 */
class EXPCL_PANDA_AUDIO CompressorDSP : public DSP {
PUBLISHED:
  INLINE CompressorDSP(float threshold = 0, float ratio = 2.5,
                       float attack = 20, float release = 100,
                       float gainmakeup = 0, bool linked = false);

  INLINE void set_threshold(float threshold);
  INLINE float get_threshold() const;
  MAKE_PROPERTY(threshold, get_threshold, set_threshold);

  INLINE void set_ratio(float ratio);
  INLINE float get_ratio() const;
  MAKE_PROPERTY(ratio, get_ratio, set_ratio);

  INLINE void set_attack(float attack);
  INLINE float get_attack() const;
  MAKE_PROPERTY(attack, get_attack, set_attack);

  INLINE void set_release(float release);
  INLINE float get_release() const;
  MAKE_PROPERTY(release, get_release, set_release);

  INLINE void set_gainmakeup(float gainmakeup);
  INLINE float get_gainmakeup() const;
  MAKE_PROPERTY(gainmakeup, get_gainmakeup, set_gainmakeup);

  INLINE void set_linked(bool linked);
  INLINE bool get_linked() const;
  MAKE_PROPERTY(linked, get_linked, set_linked);

private:
  float _threshold;
  float _ratio;
  float _attack;
  float _release;
  float _gainmakeup;
  bool _linked;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    DSP::init_type();
    register_type(_type_handle, "CompressorDSP",
                  DSP::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "compressorDSP.I"

#endif // COMPRESSORDSP_H
