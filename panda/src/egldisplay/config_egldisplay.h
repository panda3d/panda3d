/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_egldisplay.h
 * @author cary
 * @date 2009-05-21
 */

#ifndef CONFIG_EGLDISPLAY_H
#define CONFIG_EGLDISPLAY_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "configVariableString.h"
#include "configVariableBool.h"
#include "configVariableInt.h"

#if defined(OPENGLES_1) && defined(OPENGLES_2)
  #error OPENGLES_1 and OPENGLES_2 cannot be defined at the same time!
#endif
#if !defined(OPENGLES_1) && !defined(OPENGLES_2)
  #error Either OPENGLES_1 or OPENGLES_2 must be defined when compiling egldisplay!
#endif

#ifdef OPENGLES_2
  NotifyCategoryDecl(egldisplay, EXPCL_PANDAGLES2, EXPTP_PANDAGLES2);

  extern EXPCL_PANDAGLES2 void init_libegldisplay();
  extern EXPCL_PANDAGLES2 const std::string get_egl_error_string(int error);
#else
  NotifyCategoryDecl(egldisplay, EXPCL_PANDAGLES, EXPTP_PANDAGLES);

  extern EXPCL_PANDAGLES void init_libegldisplay();
  extern EXPCL_PANDAGLES const std::string get_egl_error_string(int error);
#endif

#endif
