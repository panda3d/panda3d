// Filename: config_glxdisplay.h
// Created by:  cary (07Oct99)
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

#ifndef __CONFIG_GLXDISPLAY_H__
#define __CONFIG_GLXDISPLAY_H__

#include "pandabase.h"
#include <notifyCategoryProxy.h>

NotifyCategoryDecl(glxdisplay, EXPCL_PANDAGL, EXPTP_PANDAGL);

extern EXPCL_PANDAGL void init_libglxdisplay();

extern const string display_cfg;

#endif /* __CONFIG_GLXDISPLAY_H__ */
