// Filename: config_egldisplay.h
// Created by:  cary (21May09)
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
  extern EXPCL_PANDAGLES2 const string get_egl_error_string(int error);
#else
  NotifyCategoryDecl(egldisplay, EXPCL_PANDAGLES, EXPTP_PANDAGLES);
  
  extern EXPCL_PANDAGLES void init_libegldisplay();
  extern EXPCL_PANDAGLES const string get_egl_error_string(int error);
#endif

extern ConfigVariableString display_cfg;
extern ConfigVariableBool x_error_abort;

extern ConfigVariableInt x_wheel_up_button;
extern ConfigVariableInt x_wheel_down_button;
extern ConfigVariableInt x_wheel_left_button;
extern ConfigVariableInt x_wheel_right_button;

#endif
