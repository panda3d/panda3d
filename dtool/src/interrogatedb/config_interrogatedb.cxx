// Filename: config_interrogatedb.cxx
// Created by:  drose (01Aug00)
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

#include "config_interrogatedb.h"
#include "interrogate_request.h"

#include <dconfig.h>

#if defined(WIN32_VC) && defined(_DEBUG)
// _DEBUG assumes you are linking to msvcrt70d.dll, not msvcrt70.dll
#define USE_WIN32_DBGHEAP
#include <crtdbg.h>
#endif

Configure(config_interrogatedb);
NotifyCategoryDef(interrogatedb, "");

ConfigureFn(config_interrogatedb) {
  //  interrogate_request_library("types");

#ifdef USE_WIN32_DBGHEAP
  if(!config_interrogatedb.GetBool("use-win32-dbgheap", false)) {
      // deflt disable complete heap verify every 1024 allocations (VC7 deflt).
      // With vc7 stl small-string-optimization causing more allocs, 
      // this can cause order-of-magnitude slowdowns in dbg builds
      ::_CrtSetDbgFlag(0x0);
  }
#endif
}

const DSearchPath &
get_interrogatedb_path() {
  static DSearchPath *interrogatedb_path = NULL;
  if (interrogatedb_path == (DSearchPath *)NULL) {
    interrogatedb_path = new DSearchPath(".");
    interrogatedb_path->append_path
      (config_interrogatedb.GetString("ETC_PATH", "."));
  }
  return *interrogatedb_path;
}

