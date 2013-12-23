// Filename: config_express.cxx
// Created by:  drose (28Mar06)
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

////////////////////////////////////////////////////////////////////
//     Function: init_libexpress
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
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

  // This is a fine place to ensure that the numeric types have been
  // chosen correctly.
  nassertv(sizeof(PN_int8) == 1 && sizeof(PN_uint8) == 1);
  nassertv(sizeof(PN_int16) == 2 && sizeof(PN_uint16) == 2);
  nassertv(sizeof(PN_int32) == 4 && sizeof(PN_uint32) == 4);
  nassertv(sizeof(PN_int64) == 8 && sizeof(PN_uint64) == 8);
  nassertv(sizeof(PN_float32) == 4);
  nassertv(sizeof(PN_float64) == 8);

  // Also, ensure that we have the right endianness.
  PN_uint32 word;
  memcpy(&word, "\1\2\3\4", 4);
#ifdef WORDS_BIGENDIAN
  nassertv(word == 0x01020304);
#else
  nassertv(word == 0x04030201);
#endif
}

bool
get_use_high_res_clock() {
  static ConfigVariableBool *use_high_res_clock = NULL;

  if (use_high_res_clock == (ConfigVariableBool *)NULL) {
    use_high_res_clock = new ConfigVariableBool
      ("use-high-res-clock", true,
       PRC_DESC("Set this to false to avoid using the high-precision clock, even if "
                "it is available."));
  }

  return *use_high_res_clock;
}

bool
get_paranoid_clock() {
  static ConfigVariableBool *paranoid_clock = NULL;

  if (paranoid_clock == (ConfigVariableBool *)NULL) {
    paranoid_clock = new ConfigVariableBool
      ("paranoid-clock", false,
       PRC_DESC("Set this to true to double-check the results of the high-resolution "
                "clock against the system clock."));
  }

  return *paranoid_clock;
}

bool
get_verify_dcast() {
  static ConfigVariableBool *verify_dcast = NULL;

  if (verify_dcast == (ConfigVariableBool *)NULL) {
    verify_dcast = new ConfigVariableBool
      ("verify-dcast", true,
       PRC_DESC("Set this to true to verify that every attempted DCAST operation in "
                "fact references the correct type, or false otherwise.  This has no "
                "effect if NDEBUG is defined, in which case it is never tested."));
  }

  return *verify_dcast;
}

// Returns the configure object for accessing config variables from a
// scripting language.
DConfig &
get_config_express() {
  static DConfig config_express;
  return config_express;
}

#ifdef ANDROID
static JavaVM *panda_jvm = NULL;

////////////////////////////////////////////////////////////////////
//     Function: JNI_OnLoad
//  Description: Called by Java when loading this library.
////////////////////////////////////////////////////////////////////
jint JNI_OnLoad(JavaVM *jvm, void *reserved) {
  panda_jvm = jvm;
  return JNI_VERSION_1_4;
}

////////////////////////////////////////////////////////////////////
//     Function: get_java_vm
//  Description: Returns a pointer to the JavaVM object.
////////////////////////////////////////////////////////////////////
JavaVM *get_java_vm() {
  nassertr(panda_jvm != NULL, NULL);
  return panda_jvm;
}

////////////////////////////////////////////////////////////////////
//     Function: get_jni_env
//  Description: Returns a JNIEnv object for the current thread.
//               If it doesn't already exist, attaches the JVM
//               to this thread.
////////////////////////////////////////////////////////////////////
JNIEnv *get_jni_env() {
  nassertr(panda_jvm != NULL, NULL);
  JNIEnv *env = NULL;
  int status = panda_jvm->GetEnv((void**) &env, JNI_VERSION_1_4);

  if (status < 0 || env == NULL) {
    express_cat.error() << "JVM is not available in this thread!\n";
    return NULL;
  }

  return env;
}
#endif
