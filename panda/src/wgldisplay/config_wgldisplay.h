// Filename: config_wgldisplay.h
// Created by:  drose (20Dec02)
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

#ifndef CONFIG_WGLDISPLAY_H
#define CONFIG_WGLDISPLAY_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "configVariableInt.h"
#include "configVariableBool.h"

NotifyCategoryDecl(wgldisplay, EXPCL_PANDAGL, EXPTP_PANDAGL);

extern ConfigVariableInt gl_force_pixfmt;
extern ConfigVariableBool gl_force_invalid;
extern ConfigVariableBool gl_do_vidmemsize_check;

extern EXPCL_PANDAGL void init_libwgldisplay();

#endif
