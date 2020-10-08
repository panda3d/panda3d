/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pitchShiftDSP.h
 * @author lachbr
 * @date 2020-10-08
 */

#ifndef PITCHSHIFTDSP_H
#define PITCHSHIFTDSP_H

#include "dsp.h"

/**
 * DSP filter that bends the pitch of a sound without changing the playback
 * speed.
 */
class EXPCL_PANDA_AUDIO PitchShiftDSP : public DSP {
PUBLISHED:
  INLINE PitchShiftDSP(float pitch = 1, int fft_size = 1024);

  INLINE void set_pitch(float pitch);
  INLINE float get_pitch() const;
  MAKE_PROPERTY(pitch, get_pitch, set_pitch);

  INLINE void set_fft_size(int size);
  INLINE int get_fft_size() const;
  MAKE_PROPERTY(fft_size, get_fft_size, set_fft_size);

private:
  float _pitch;
  int _fft_size;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    DSP::init_type();
    register_type(_type_handle, "PitchShiftDSP",
                  DSP::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "pitchShiftDSP.I"

#endif // PITCHSHIFTDSP_H
