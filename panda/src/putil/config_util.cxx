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
#include "animInterface.h"
#include "bamReaderParam.h"
#include "cachedTypedWritableReferenceCount.h"
#include "clockObject.h"
#include "configurable.h"
#include "datagram.h"
#include "factoryParam.h"
#include "namable.h"
#include "nodeCachedReferenceCount.h"
#include "referenceCount.h"
#include "typedObject.h"
#include "typedReferenceCount.h"
#include "typedWritable.h"
#include "typedWritableReferenceCount.h"
#include "writableConfigurable.h"
#include "writableParam.h"
#include "keyboardButton.h"
#include "mouseButton.h"

#include "dconfig.h"

ConfigureDef(config_util);
NotifyCategoryDef(util, "");
NotifyCategoryDef(bam, util_cat);

ConfigVariableEnum<BamEndian> bam_endian
("bam-endian", BE_native,
 PRC_DESC("The default endianness to use for writing major numeric data "
          "tables to bam files.  This does not affect all numbers written "
          "to bam files, only those for which the individual object was "
          "designed to support this flag.  The default is \"native\"; you "
          "may set it to \"littleendian\" or \"bigendian\" to target a "
          "particular platform."));

ConfigVariableSearchPath model_path
("model-path", 
 PRC_DESC("The default directories to search for all models and general "
          "files loaded into Panda."));

ConfigVariableSearchPath texture_path
("texture-path", 
 PRC_DESC("A special directory path to search for textures only.  "
          "Textures are also searched for along the model-path, so the "
          "use of texture-path is only useful if you have special "
          "directories that only contain textures."));

ConfigVariableSearchPath sound_path
("sound-path", 
 PRC_DESC("The directories to search for sound and music files to be loaded."));

ConfigureFn(config_util) {
  AnimInterface::init_type();
  BamReaderParam::init_type();
  CachedTypedWritableReferenceCount::init_type();
  Configurable::init_type();
  Datagram::init_type();
  FactoryParam::init_type();
  Namable::init_type();
  NodeCachedReferenceCount::init_type();
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
// ConfigVariableBool track_memory_usage("track-memory-usage", false);

// There is no longer any need for C++ code to call these functions to
// access the various path variables; instead, new C++ code should
// just access the path variables directly.
ConfigVariableSearchPath &
get_model_path() {
  return model_path;
}

ConfigVariableSearchPath &
get_texture_path() {
  return texture_path;
}

ConfigVariableSearchPath &
get_sound_path() {
  return sound_path;
}
