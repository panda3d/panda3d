// Filename: config_express.cxx
// Created by:  cary (04Jan00)
// 
////////////////////////////////////////////////////////////////////

#include "config_express.h"
#include "clockObject.h"
#include "typeHandle.h"
#include "referenceCount.h"
#include "typedReferenceCount.h"
#include "datagram.h"

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
