// Filename: config_physics.h
// Created by:  charles (17Jul00)
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

#ifndef CONFIG_PHYSICS_H
#define CONFIG_PHYSICS_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "dconfig.h"

ConfigureDecl(config_physics, EXPCL_PANDAPHYSICS, EXPTP_PANDAPHYSICS);
NotifyCategoryDecl(physics, EXPCL_PANDAPHYSICS, EXPTP_PANDAPHYSICS);

extern EXPCL_PANDAPHYSICS void init_libphysics();

// These macros get stripped out in a non-debug build (like asserts).
// Use them like cout but with paranthesis aroud the cout input.
// e.g. foo_debug("The value of bar is " << bar);
#ifndef NDEBUG //[
  // Non-release build:
  #define PHYSICS_DEBUG

  #define physics_spam(msg) \
  if (physics_cat.is_spam()) { \
    physics_cat->spam() << msg << endl; \
  } else {}

  #define physics_debug(msg) \
  if (physics_cat.is_debug()) { \
    physics_cat->debug() << msg << endl; \
  } else {}

  #define physics_info(msg) \
    physics_cat->info() << msg << endl

  #define physics_warning(msg) \
    physics_cat->warning() << msg << endl

  #define physics_error(msg) \
    physics_cat->error() << msg << endl
#else //][
  // Release build:
  #undef PHYSICS_DEBUG

  #define physics_spam(msg) ((void)0)
  #define physics_debug(msg) ((void)0)
  #define physics_info(msg) ((void)0)
  #define physics_warning(msg) ((void)0)
  #define physics_error(msg) ((void)0)
#endif //]

#define audio_error(msg) \
  audio_cat->error() << msg << endl

#endif // CONFIG_PHYSICS_H
