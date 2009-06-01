// Filename: config_physx.h
// Created by: pratt (Apr 18, 2006)
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

#ifndef CONFIG_PHYSX_H
#define CONFIG_PHYSX_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "dconfig.h"

ConfigureDecl(config_physx, EXPCL_PANDAPHYSX, EXPTP_PANDAPHYSX);
NotifyCategoryDecl(physx, EXPCL_PANDAPHYSX, EXPTP_PANDAPHYSX);

extern EXPCL_PANDA ConfigVariableBool physx_want_visual_debugger;
extern EXPCL_PANDA ConfigVariableString physx_visual_debugger_host;
extern EXPCL_PANDA ConfigVariableInt physx_visual_debugger_port;

extern EXPCL_PANDAPHYSX void init_libphysx();

// These macros get stripped out in a non-debug build (like asserts).
// Use them like cout but with paranthesis aroud the cout input.
// e.g. foo_debug("The value of bar is " << bar);
#ifndef NDEBUG //[
  // Non-release build:
  #define PHYSX_DEBUG

  #define physx_spam(msg) \
  if (physx_cat.is_spam()) { \
    physx_cat->spam() << msg << endl; \
  } else {}

  #define physx_debug(msg) \
  if (physx_cat.is_debug()) { \
    physx_cat->debug() << msg << endl; \
  } else {}

  #define physx_info(msg) \
    physx_cat->info() << msg << endl

  #define physx_warning(msg) \
    physx_cat->warning() << msg << endl

  // defining NX_USER_DEBUG_MODE causes the PhysX SDK to output extra
  // information to NxUserOutputStream.
  #define NX_USER_DEBUG_MODE
#else //][
  // Release build:
  #undef PHYSX_DEBUG

  #define physx_spam(msg) ((void)0)
  #define physx_debug(msg) ((void)0)
  #define physx_info(msg) ((void)0)
  #define physx_warning(msg) ((void)0)
#endif //]

#define physx_error(msg) \
  physx_cat->error() << msg << endl

#endif // CONFIG_PHYSX_H
