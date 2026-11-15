/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file steamAudioEffect.h
 * @author Jackson Sutherland
 */

#ifndef STEAMAUDIOEFFECT_H
#define STEAMAUDIOEFFECT_H

#include "pandabase.h"
#include "typedObject.h"

#include <phonon.h>

#include "steamAudioProcessor.h"

class EXPCL_STEAM_AUDIO SteamAudioEffect : public TypedReferenceCount {

  friend class SteamAudioProcessor;
PUBLISHED:
  SteamAudioEffect();
  ~SteamAudioEffect();

  void set_active(bool val);
  bool get_active();

protected:
  virtual void get_render_flags(SteamAudioProcessor::source_properties* source);
  virtual IPLAudioBuffer* apply_effect(SteamAudioProcessor::SteamGlobalHolder* globals, IPLAudioBuffer& inBuffer);
  bool _isActive;


public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedObject::init_type();
    register_type(_type_handle, "SteamAudioEffect", TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {
    init_type();
    return get_class_type();
  }
private:
  static TypeHandle _type_handle;
};

#endif
