// Filename: config_wgldisplay.h
// Created by:  drose (20Dec02)
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
