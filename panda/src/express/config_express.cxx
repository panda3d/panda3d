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


// Set this true to enable tracking of ReferenceCount pointer
// allocation/deallcation via the MemoryUsage object.  This is
// primarily useful for detecting memory leaks.  It has no effect when
// compiling in NDEBUG mode.

// This variable is no longer defined here; instead, it's a member of
// MemoryUsage.

//const bool track_memory_usage = config_express.GetBool("track-memory-usage", false);

const int patchfile_window_size =
	config_express.GetInt("patchfile-window-size", 16);

const int patchfile_increment_size =
	config_express.GetInt("patchfile-increment-size", 8);

const int patchfile_buffer_size =
	config_express.GetInt("patchfile-buffer-size", 4096);

const int patchfile_zone_size =
	config_express.GetInt("patchfile-zone-size", 10000);
