// Filename: config_interrogatedb.cxx
// Created by:  drose (01Aug00)
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

#include "config_interrogatedb.h"
#include "interrogate_request.h"

#include "dconfig.h"

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
  string use_win32_dbgheap_str = config_interrogatedb.GetString("use-win32-dbgheap", "");
  bool win32_report_leaks = config_interrogatedb.GetBool("win32-report-leaks", false);

  int dbg_flags = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );

  if (use_win32_dbgheap_str == "full") {
    // "full" means check the heap after *every* alloc/dealloc.
    // Expensive.
    dbg_flags |= (_CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF |
                  _CRTDBG_CHECK_CRT_DF);

  } else {
    // Otherwise, it's a bool flag.  true means check the heap
    // normally, false means don't do any debug checking.
    bool use_win32_dbgheap_bool = config_interrogatedb.GetBool("use-win32-dbgheap", false);

    if (!use_win32_dbgheap_bool) {
      // deflt disable complete heap verify every 1024 allocations (VC7 deflt).
      // With vc7 stl small-string-optimization causing more allocs, 
      // this can cause order-of-magnitude slowdowns in dbg builds
      dbg_flags = 0;
    }
  }

  if (win32_report_leaks) {
    // Report memory still allocated at program termination.  Not sure
    // how useful this is, as many things get allocated once and never
    // freed, but they aren't really leaks.
    dbg_flags |= _CRTDBG_LEAK_CHECK_DF;
  }

  _CrtSetDbgFlag(dbg_flags);
#endif
}

DSearchPath &
get_interrogatedb_path() {
  static DSearchPath *interrogatedb_path = NULL;
  if (interrogatedb_path == (DSearchPath *)NULL) {
    interrogatedb_path = new DSearchPath(".");
    interrogatedb_path->append_path
      (config_interrogatedb.GetString("ETC_PATH", "."));
  }
  return *interrogatedb_path;
}

