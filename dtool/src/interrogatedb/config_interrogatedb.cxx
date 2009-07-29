// Filename: config_interrogatedb.cxx
// Created by:  drose (01Aug00)
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

#include "config_interrogatedb.h"
#include "interrogate_request.h"
#include "configVariableBool.h"
#include "configVariableSearchPath.h"
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
  ConfigVariableBool use_win32_dbgheap("use-win32-dbgheap", false);
  ConfigVariableBool win32_report_leaks("win32-report-leaks", false);

  int dbg_flags = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );

  if (use_win32_dbgheap.get_string_value() == "full") {
    // "full" means check the heap after *every* alloc/dealloc.
    // Expensive.
    dbg_flags |= (_CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF |
                  _CRTDBG_CHECK_CRT_DF);

  } else {
    // Otherwise, it's a bool flag.  true means check the heap
    // normally, false means don't do any debug checking.
    if (!use_win32_dbgheap) {
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

ConfigVariableSearchPath interrogatedb_path
("interrogatedb-path", "The search path for interrogate's *.in files.");

