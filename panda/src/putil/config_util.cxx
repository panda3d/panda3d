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
#include "buttonMap.h"
#include "cachedTypedWritableReferenceCount.h"
#include "callbackData.h"
#include "callbackObject.h"
#include "clockObject.h"
#include "configurable.h"
#include "copyOnWriteObject.h"
#include "cPointerCallbackObject.h"
#include "datagram.h"
#include "doubleBitMask.h"
#include "factoryParam.h"
#include "namable.h"
#include "nodeCachedReferenceCount.h"
#include "pythonCallbackObject.h"
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

ConfigVariableEnum<BamEnums::BamEndian> bam_endian
("bam-endian", BamEnums::BE_native,
 PRC_DESC("The default endianness to use for writing major numeric data "
          "tables to bam files.  This does not affect all numbers written "
          "to bam files, only those for which the individual object was "
          "designed to support this flag.  The default is \"native\"; you "
          "may set it to \"littleendian\" or \"bigendian\" to target a "
          "particular platform."));

ConfigVariableBool bam_stdfloat_double
("bam-stdfloat-double", 
#ifdef STDFLOAT_DOUBLE
 true,
#else
 false,
#endif
 PRC_DESC("The default width of floating-point numbers to write to a bam "
          "file.  Set this true to force doubles (64-bit floats), or false "
          "to force sinles (32-bit floats).  The default is whichever width "
          "Panda has been compiled to use natively.  Normally, this setting "
          "should not be changed from the default."));

ConfigVariableEnum<BamEnums::BamTextureMode> bam_texture_mode
("bam-texture-mode", BamEnums::BTM_relative,
 PRC_DESC("Set this to specify how textures should be written into Bam files."
          "See the panda source or documentation for available options."));



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

ConfigVariableSearchPath &
get_model_path() {
  static ConfigVariableSearchPath *model_path = NULL;
  if (model_path == NULL) {
    model_path = new ConfigVariableSearchPath
      ("model-path", 
       PRC_DESC("The default directories to search for all models and general "
                "files loaded into Panda."));
  }

  return *model_path;
}

ConfigVariableSearchPath &
get_plugin_path() {
  static ConfigVariableSearchPath *plugin_path = NULL;
  if (plugin_path == NULL) {
    plugin_path = new ConfigVariableSearchPath
      ("plugin-path", "<auto>",
       PRC_DESC("The directories to search for plugin shared libraries."));
  }

  return *plugin_path;
}

ConfigVariableDouble sleep_precision
("sleep-precision", 0.01,
 PRC_DESC("This is the accuracy within which we can expect select() to "
          "return precisely.  That is, if we use select() to request a "
          "timeout of 1.0 seconds, we can expect to actually sleep for "
          "somewhere between 1.0 and 1.0 + sleep-precision seconds."));

ConfigVariableBool preload_textures
("preload-textures", true,
 PRC_DESC("When this is true, texture images are loaded from disk as soon "
          "as the Texture is created from the TexturePool.  When this is "
          "false, the Texture is created immediately, but the image data "
          "is not loaded from disk until the Texture is actually rendered "
          "(or otherwise prepared) on the GSG.  This can help reduce "
          "wasted memory from Textures that are created but never used "
          "to render."));

ConfigVariableBool preload_simple_textures
("preload-simple-textures", false,
 PRC_DESC("When this is true, every texture image will have a simple "
          "image generated for it at load time.  (Normally, textures "
          "get a simple image at egg2bam time.)  This slows the initial "
          "loading time of textures, but allows you to take advantage "
          "of gsg::set_incomplete_render() to load textures on-the-fly "
          "in a sub-thread.  It's not generally necessary if you are "
          "loading bam files that were generated via egg2bam."));

ConfigVariableBool cache_check_timestamps
("cache-check-timestamps", true,
 PRC_DESC("Set this true to check the timestamps on disk (when possible) "
          "before reloading a file from the in-memory cache, e.g. via ModelPool, "
          "TexturePool, etc.  When this is false, a model or texture "
          "that was previously loaded and is still found in the ModelPool is "
          "immediately returned without consulting the disk, even if the "
          "file on disk has recently changed.  When this is true, the file "
          "on disk is always checked to ensure its timestamp has not "
          "recently changed; and if it has, the in-memory cache is automatically "
          "invalidated and the file is reloaded from disk.  This is not related "
          "to on-disk caching via model-cache-dir, which always checks the "
          "timestamps."));

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
  BamReaderAuxData::init_type();
  BamReaderParam::init_type();
  BitArray::init_type();
  BitMask16::init_type();
  BitMask32::init_type();
  BitMask64::init_type();
  ButtonHandle::init_type();
  ButtonMap::init_type();
  CPointerCallbackObject::init_type();
  CachedTypedWritableReferenceCount::init_type();
  CallbackData::init_type();
  CallbackObject::init_type();
  ClockObject::init_type();
  Configurable::init_type();
  CopyOnWriteObject::init_type();
  Datagram::init_type();
  DoubleBitMaskNative::init_type();
  FactoryParam::init_type();
  Namable::init_type();
  NodeCachedReferenceCount::init_type();
#ifdef HAVE_PYTHON
  PythonCallbackObject::init_type();
#endif
  QuadBitMaskNative::init_type();
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

