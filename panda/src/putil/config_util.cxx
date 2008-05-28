// Filename: config_util.cxx
// Created by:  cary (04Jan00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "config_util.h"
#include "animInterface.h"
#include "bamCacheIndex.h"
#include "bamCacheRecord.h"
#include "bamReader.h"
#include "bamReaderParam.h"
#include "bitArray.h"
#include "bitMask.h"
#include "buttonHandle.h"
#include "cachedTypedWritableReferenceCount.h"
#include "clockObject.h"
#include "configurable.h"
#include "copyOnWriteObject.h"
#include "datagram.h"
#include "doubleBitMask.h"
#include "factoryParam.h"
#include "namable.h"
#include "nodeCachedReferenceCount.h"
#include "referenceCount.h"
#include "sparseArray.h"
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

ConfigVariableEnum<BamTextureMode> bam_texture_mode
("bam-texture-mode", BTM_relative,
 PRC_DESC("Set this to specify how textures should be written into Bam files."
          "See the panda source or documentation for available options."));

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
ConfigVariableSearchPath plugin_path
("plugin-path", "<auto>",
 PRC_DESC("The directories to search for plugin shared libraries."));



ConfigureFn(config_util) {
  init_libputil();
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

ConfigVariableSearchPath &
get_plugin_path() {
  return plugin_path;
}

ConfigVariableDouble sleep_precision
("sleep-precision", 0.01,
 PRC_DESC("This is the accuracy within which we can expect select() to "
          "return precisely.  That is, if we use select() to request a "
          "timeout of 1.0 seconds, we can expect to actually sleep for "
          "somewhere between 1.0 and 1.0 + sleep-precision seconds."));

////////////////////////////////////////////////////////////////////
//     Function: init_libputil
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libputil() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  AnimInterface::init_type();
  BamCacheIndex::init_type();
  BamCacheRecord::init_type();
  BamReaderParam::init_type();
  BitArray::init_type();
  BitMask32::init_type();
  BitMask64::init_type();
  ButtonHandle::init_type();
  CachedTypedWritableReferenceCount::init_type();
  ClockObject::init_type();
  Configurable::init_type();
  CopyOnWriteObject::init_type();
  Datagram::init_type();
  DoubleBitMaskNative::init_type();
  QuadBitMaskNative::init_type();
  FactoryParam::init_type();
  Namable::init_type();
  NodeCachedReferenceCount::init_type();
  ReferenceCount::init_type();
  SparseArray::init_type();
  TypedObject::init_type();
  TypedReferenceCount::init_type();
  TypedWritable::init_type();
  TypedWritableReferenceCount::init_type();
  WritableConfigurable::init_type();
  WritableParam::init_type();

  KeyboardButton::init_keyboard_buttons();
  MouseButton::init_mouse_buttons();

  register_type(BamReader::_remove_flag, "remove");

  BamCacheIndex::register_with_read_factory();
  BamCacheRecord::register_with_read_factory();

  // Initialize the num_bits_on table, for BitMask::get_num_on_bits().
  for (int bit = 0; bit < 16; ++bit) {
    int w = (1 << bit);
    for (int i = 0; i < w; ++i) {
      num_bits_on[i + w] = num_bits_on[i] + 1;
    }
  }
}

