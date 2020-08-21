/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_express.cxx
 * @author drose
 * @date 2006-03-28
 */

#include "config_express.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "nodeReferenceCount.h"
#include "referenceCount.h"
#include "textEncoder.h"
#include "typedObject.h"
#include "typedReferenceCount.h"
#include "virtualFile.h"
#include "virtualFileComposite.h"
#include "virtualFileMount.h"
#include "virtualFileMountAndroidAsset.h"
#include "virtualFileMountMultifile.h"
#include "virtualFileMountRamdisk.h"
#include "virtualFileMountSystem.h"
#include "virtualFileSimple.h"
#include "fileReference.h"
#include "temporaryFile.h"
#include "pandaSystem.h"
#include "numeric_types.h"
#include "namable.h"
#include "export_dtool.h"
#include "dconfig.h"
#include "streamWrapper.h"

#if !defined(CPPPARSER) && !defined(LINK_ALL_STATIC) && !defined(BUILDING_PANDA_EXPRESS)
  #error Buildsystem error: BUILDING_PANDA_EXPRESS not defined
#endif

ConfigureDef(config_express);
NotifyCategoryDef(express, "");
NotifyCategoryDef(clock, ":express");

ConfigureFn(config_express) {
  init_libexpress();
}

ConfigVariableInt patchfile_window_size
("patchfile-window-size", 16);

ConfigVariableInt patchfile_increment_size
("patchfile-increment-size", 8);

ConfigVariableInt patchfile_buffer_size
("patchfile-buffer-size", 4096);

ConfigVariableInt patchfile_zone_size
("patchfile-zone-size", 10000);

ConfigVariableBool keep_temporary_files
("keep-temporary-files", false,
 PRC_DESC("Set this true to keep around the temporary files from "
          "downloading, decompressing, and patching, or false (the "
          "default) to delete these.  Mainly useful for debugging "
          "when the process goes wrong."));

ConfigVariableBool multifile_always_binary
("multifile-always-binary", false,
 PRC_DESC("This is a temporary transition variable.  Set this true "
          "to enable the old behavior for multifiles: all subfiles are "
          "always added to and extracted from the multifile in binary mode.  "
          "Set it false to enable the new behavior: subfiles may be added "
          "or extracted in either binary or text mode, according to the "
          "set_binary() or set_text() flag on the Filename."));

ConfigVariableBool collect_tcp
("collect-tcp", false,
 PRC_DESC("Set this true to enable accumulation of several small consecutive "
          "TCP datagrams into one large datagram before sending it, to reduce "
          "overhead from the TCP/IP protocol.  See "
          "Connection::set_collect_tcp() or SocketStream::set_collect_tcp()."));

ConfigVariableDouble collect_tcp_interval
("collect-tcp-interval", 0.2);

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libexpress() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  Datagram::init_type();
  DatagramIterator::init_type();
  Namable::init_type();
  NodeReferenceCount::init_type();
  ReferenceCount::init_type();
  TypedObject::init_type();
  TypedReferenceCount::init_type();
  VirtualFile::init_type();
  VirtualFileComposite::init_type();
  VirtualFileMount::init_type();
#ifdef ANDROID
  VirtualFileMountAndroidAsset::init_type();
#endif
  VirtualFileMountMultifile::init_type();
  VirtualFileMountRamdisk::init_type();
  VirtualFileMountSystem::init_type();
  VirtualFileSimple::init_type();
  FileReference::init_type();
  TemporaryFile::init_type();

  init_system_type_handles();

#ifdef HAVE_ZLIB
  {
    PandaSystem *ps = PandaSystem::get_global_ptr();
    ps->add_system("zlib");
  }
#endif

  // This is a fine place to ensure that the numeric types have been chosen
  // correctly.
  nassertv(sizeof(int8_t) == 1 && sizeof(uint8_t) == 1);
  nassertv(sizeof(int16_t) == 2 && sizeof(uint16_t) == 2);
  nassertv(sizeof(int32_t) == 4 && sizeof(uint32_t) == 4);
  nassertv(sizeof(int64_t) == 8 && sizeof(uint64_t) == 8);
  nassertv(sizeof(PN_float32) == 4);
  nassertv(sizeof(PN_float64) == 8);

  // Also, ensure that we have the right endianness.
  uint32_t word;
  memcpy(&word, "\1\2\3\4", 4);
#ifdef WORDS_BIGENDIAN
  nassertv(word == 0x01020304);
#else
  nassertv(word == 0x04030201);
#endif
}

bool
get_use_high_res_clock() {
  static ConfigVariableBool *use_high_res_clock = nullptr;

  if (use_high_res_clock == nullptr) {
    use_high_res_clock = new ConfigVariableBool
      ("use-high-res-clock", true,
       PRC_DESC("Set this to false to avoid using the high-precision clock, even if "
                "it is available."));
  }

  return *use_high_res_clock;
}

bool
get_paranoid_clock() {
  static ConfigVariableBool *paranoid_clock = nullptr;

  if (paranoid_clock == nullptr) {
    paranoid_clock = new ConfigVariableBool
      ("paranoid-clock", false,
       PRC_DESC("Set this to true to double-check the results of the high-resolution "
                "clock against the system clock."));
  }

  return *paranoid_clock;
}

bool
get_verify_dcast() {
  static ConfigVariableBool *verify_dcast = nullptr;

  if (verify_dcast == nullptr) {
    verify_dcast = new ConfigVariableBool
      ("verify-dcast", true,
       PRC_DESC("Set this to true to verify that every attempted DCAST operation in "
                "fact references the correct type, or false otherwise.  This has no "
                "effect if NDEBUG is defined, in which case it is never tested."));
  }

  return *verify_dcast;
}
