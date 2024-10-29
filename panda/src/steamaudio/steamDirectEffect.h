/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file steamDirectEffect.h
 * @author Jackson Sutherland
 */

#ifndef STEAMDIRECTEFFECT_H
#define STEAMDIRECTEFFECT_H

#include "pandabase.h"
#include "typedObject.h"

#include "steamAudioSound.h"
#include "nodePath.h"
#include "movieAudioCursor.h"
#include "plist.h"//Don't know if I'll need these, but good idea to keep in hand
#include "pmap.h"
#include "pset.h"
#include "steamAudioEffect.h"

#include <phonon.h>


class EXPCL_STEAMAUDIO SteamDirectEffect : public TypedObject {

  friend class SteamAudioSound
    PUBLISHED :
    SteamDirectEffect();
    ~SteamDirectEffect();

private:
  virtual IPLAudioBuffer apply_effect(SteamAudioSound::SteamGlobalHolder* globals, IPLAudioBuffer inBuffer);


public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedObject::init_type();
    register_type(_type_handle, "SteamSpatialEffect", TypedObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {
    init_type();
    return get_class_type();
  }
}

#endif
