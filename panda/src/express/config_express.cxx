// Filename: config_express.cxx
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


#include "config_express.h"
#include "datagram.h"
#include "referenceCount.h"
#include "textEncoder.h"
#include "thread.h"
#include "typedObject.h"
#include "typedReferenceCount.h"
#include "virtualFile.h"
#include "virtualFileComposite.h"
#include "virtualFileMount.h"
#include "virtualFileMountMultifile.h"
#include "virtualFileMountSystem.h"
#include "virtualFileSimple.h"
#include "pandaSystem.h"

#include "dconfig.h"

ConfigureDef(config_express);
NotifyCategoryDef(express, "");
NotifyCategoryDef(thread, "");

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

ConfigVariableDouble average_frame_rate_interval
("average-frame-rate-interval", 1.0);

ConfigVariableDouble clock_frame_rate
("clock-frame-rate", 1.0);
ConfigVariableDouble clock_degrade_factor
("clock-degrade-factor", 1.0);
ConfigVariableDouble max_dt
("max-dt", -1.0);

ConfigVariableDouble sleep_precision
("sleep-precision", 0.01,
 PRC_DESC("This is the accuracy within which we can expect select() to "
          "return precisely.  That is, if we use select() to request a "
          "timeout of 1.0 seconds, we can expect to actually sleep for "
          "somewhere between 1.0 and 1.0 + sleep-precision seconds."));

ConfigVariableString encryption_algorithm
("encryption-algorithm", "bf-cbc",
 PRC_DESC("This defines the OpenSSL encryption algorithm which is used to "
          "encrypt any streams created by the current runtime.  The default is "
          "Blowfish; the complete set of available algorithms is defined by "
          "the current version of OpenSSL.  This value is used only to control "
          "encryption; the correct algorithm will automatically be selected on "
          "decryption."));

ConfigVariableInt encryption_key_length
("encryption-key-length", 0,
 PRC_DESC("This defines the key length, in bits, for the selected encryption "
          "algorithm.  Some algorithms have a variable key length.  Specifying "
          "a value of 0 here means to use the default key length for the "
          "algorithm as defined by OpenSSL.  This value is used only to "
          "control encryption; the correct key length will automatically be "
          "selected on decryption."));

ConfigVariableInt encryption_iteration_count
("encryption-iteration-count", 100000,
 PRC_DESC("This defines the number of times a password is hashed to generate a "
          "key when encrypting.  Its purpose is to make it computationally "
          "more expensive for an attacker to search the key space "
          "exhaustively.  This should be a multiple of 1,000 and should not "
          "exceed about 65 million; the value 0 indicates just one application "
          "of the hashing algorithm.  This value is used only to control "
          "encryption; the correct count will automatically be selected on "
          "decryption."));

ConfigVariableInt multifile_encryption_iteration_count
("multifile-encryption-iteration-count", 0,
 PRC_DESC("This is a special value of encryption-iteration-count used to encrypt "
          "subfiles within a multifile.  It has a default value of 0 (just one "
          "application), on the assumption that the files from a multifile must "
          "be loaded quickly, without paying the cost of an expensive hash on "
          "each subfile in order to decrypt it."));

ConfigVariableBool vfs_case_sensitive
("vfs-case-sensitive", true,
 PRC_DESC("Set this true to make the VirtualFileSystem present the native "
          "OS-provided filesystem as if it were a case-sensitive file "
          "system, even if it is not (e.g. on Windows).  This variable "
          "has no effect if the native filesystem is already case-sensitive, "
          "and it has no effect on mounted multifile systems, which are "
          "always case-sensitive."));

ConfigVariableBool use_vfs
("use-vfs", true,
 PRC_DESC("Set this true to use the VirtualFileSystem mechanism for loading "
          "models, etc.  Since the VirtualFileSystem maps to the same as the "
          "actual file system by default, there is probably no reason to set "
          "this false, except for testing or if you mistrust the new code."));

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
  ReferenceCount::init_type();
  TextEncoder::init_type();
  Thread::init_type();
  TypedObject::init_type();
  TypedReferenceCount::init_type();
  VirtualFile::init_type();
  VirtualFileComposite::init_type();
  VirtualFileMount::init_type();
  VirtualFileMountMultifile::init_type();
  VirtualFileMountSystem::init_type();
  VirtualFileSimple::init_type();

  init_system_type_handles();

#ifdef HAVE_ZLIB
  PandaSystem *ps = PandaSystem::get_global_ptr();
  ps->add_system("zlib");
#endif
}


bool
get_leak_memory() {
  static ConfigVariableBool *leak_memory = NULL;

  if (leak_memory == (ConfigVariableBool *)NULL) {
    leak_memory = new ConfigVariableBool
      ("leak-memory", false,
       PRC_DESC("Set leak-memory true to disable the actual deletion of "
                "ReferenceCount-derived objects.  This is sometimes useful to track "
                "a reference counting bug, since the formerly deleted objects will "
                "still remain (with a reference count of -100) without being "
                "overwritten with a newly-allocated object, and the assertion tests "
                "in ReferenceCount may more accurately detect the first instance of "
                "an error."));
  }

  return *leak_memory;
}

bool
get_never_destruct() {
  static ConfigVariableBool *never_destruct = NULL;

  if (never_destruct == (ConfigVariableBool *)NULL) {
    never_destruct = new ConfigVariableBool
      ("never-destruct", false,
       PRC_DESC("never-destruct is similar to leak-memory, except that not "
                "only will memory not be freed, but the destructor will not even be "
                "called (on ReferenceCount objects, at least).  This will leak gobs "
                "of memory, but ensures that every pointer to a ReferenceCount "
                "object will always be valid, and may be useful for tracking down "
                "certain kinds of errors.  "
                "never-destruct is only respected if leak-memory is true."));
  }

  return *never_destruct;
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
                "clock against the system clock.  This has no effect if NDEBUG is "
                "defined."));
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
ConfigExpress &
get_config_express() {
  return config_express;
}
