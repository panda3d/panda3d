// Filename: config_wgldisplay.h
// Created by:  drose (20Dec02)
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

#ifndef CONFIG_WGLDISPLAY_H
#define CONFIG_WGLDISPLAY_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"

NotifyCategoryDecl(wgldisplay, EXPCL_PANDAGL, EXPTP_PANDAGL);

extern EXPCL_PANDAGL void init_libwgldisplay();

extern int gl_force_pixfmt;
extern bool gl_force_invalid;
extern bool gl_do_vidmemsize_check;
extern bool show_pbuffers;

#endif
