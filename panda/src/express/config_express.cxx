// Filename: config_express.cxx
// Created by:  cary (04Jan00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#if defined(WIN32_VC) && !defined(NO_PCH)
#include "express_headers.h"
#endif

#pragma hdrstop

#if !defined(WIN32_VC) || defined(NO_PCH)
#include "config_express.h"
#include "clockObject.h"
#include "typedObject.h"
#include "referenceCount.h"
#include "typedReferenceCount.h"
#include "datagram.h"
#endif

#include <dconfig.h>

ConfigureDef(config_express);
NotifyCategoryDef(express, "");

extern void init_system_type_handles();

ConfigureFn(config_express) {
//  ClockObject::init_ptr();
  TypedObject::init_type();
  ReferenceCount::init_type();
  TypedReferenceCount::init_type();
  init_system_type_handles();
  Datagram::init_type();
}


// Set leak-memory true to disable the actual deletion of
// ReferenceCount-derived objects.  This is sometimes useful to track
// a reference counting bug, since the formerly deleted objects will
// still remain (with a reference count of -100) without being
// overwritten with a newly-allocated object, and the assertion tests
// in ReferenceCount may more accurately detect the first instance of
// an error.
bool
get_leak_memory() {
  static bool got_leak_memory = false;
  static bool leak_memory;

  if (!got_leak_memory) {
    leak_memory = config_express.GetBool("leak-memory", false);
    got_leak_memory = true;
  }

  return leak_memory;
}

// never-destruct is similar to leak-memory, above, except that not
// only will memory not be freed, but the destructor will not even be
// called (on ReferenceCount objects, at least).  This will leak gobs
// of memory, but ensures that every pointer to a ReferenceCount
// object will always be valid, and may be useful for tracking down
// certain kinds of errors.

// never-destruct is only respected if leak-memory, above, is true.
bool
get_never_destruct() {
  static bool got_never_destruct = false;
  static bool never_destruct;

  if (!got_never_destruct) {
    never_destruct = config_express.GetBool("never-destruct", false);
    got_never_destruct = true;
  }

  return never_destruct;
}

//const bool track_memory_usage = config_express.GetBool("track-memory-usage", false);

// Set this to false to avoid using the high-precision clock, even if
// it is available.
bool
get_use_high_res_clock() {
  static bool got_use_high_res_clock = false;
  static bool use_high_res_clock;

  if (!got_use_high_res_clock) {
    use_high_res_clock = config_express.GetBool("use-high-res-clock", true);
    got_use_high_res_clock = true;
  }

  return use_high_res_clock;
}

const int patchfile_window_size =
        config_express.GetInt("patchfile-window-size", 16);

const int patchfile_increment_size =
        config_express.GetInt("patchfile-increment-size", 8);

const int patchfile_buffer_size =
        config_express.GetInt("patchfile-buffer-size", 4096);

const int patchfile_zone_size =
        config_express.GetInt("patchfile-zone-size", 10000);
