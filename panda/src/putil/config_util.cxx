// Filename: config_util.cxx
// Created by:  cary (04Jan00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "config_util.h"
#include "bamReaderParam.h"
#include "clockObject.h"
#include "configurable.h"
#include "datagram.h"
#include "factoryParam.h"
#include "namable.h"
#include "referenceCount.h"
#include "typedObject.h"
#include "typedReferenceCount.h"
#include "typedWritable.h"
#include "typedWritableReferenceCount.h"
#include "writableConfigurable.h"
#include "writableParam.h"
#include "keyboardButton.h"
#include "mouseButton.h"
#include "get_config_path.h"

#include "dconfig.h"

ConfigureDef(config_util);
NotifyCategoryDef(util, "");
NotifyCategoryDef(bam, util_cat);

ConfigureFn(config_util) {
  BamReaderParam::init_type();
  Configurable::init_type();
  Datagram::init_type();
  FactoryParam::init_type();
  Namable::init_type();
  ReferenceCount::init_type();
  TypedObject::init_type();
  TypedReferenceCount::init_type();
  TypedWritable::init_type();
  TypedWritableReferenceCount::init_type();
  WritableConfigurable::init_type();
  WritableParam::init_type();

  KeyboardButton::init_keyboard_buttons();
  MouseButton::init_mouse_buttons();

  register_type(BamReader::_remove_flag, "remove");
}


// Set this true to enable tracking of ReferenceCount pointer
// allocation/deallcation via the MemoryUsage object.  This is
// primarily useful for detecting memory leaks.  It has no effect when
// compiling in NDEBUG mode.
//
// This variable is no longer defined here; instead, it's a member of
// MemoryUsage.
//
//const bool track_memory_usage = config_util.GetBool("track-memory-usage", false);

DSearchPath &
get_model_path() {
  static DSearchPath *model_path = NULL;
  return get_config_path("model-path", model_path);
}

DSearchPath &
get_texture_path() {
  static DSearchPath *texture_path = NULL;
  return get_config_path("texture-path", texture_path);
}

DSearchPath &
get_sound_path() {
  static DSearchPath *sound_path = NULL;
  return get_config_path("sound-path", sound_path);
}
