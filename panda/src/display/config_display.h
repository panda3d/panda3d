// Filename: config_display.h
// Created by:  drose (06Oct99)
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

#ifndef CONFIG_DISPLAY_H
#define CONFIG_DISPLAY_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "dconfig.h"

#include <string>
#include "pvector.h"

ConfigureDecl(config_display, EXPCL_PANDA, EXPTP_PANDA);
NotifyCategoryDecl(display, EXPCL_PANDA, EXPTP_PANDA);
NotifyCategoryDecl(gsg, EXPCL_PANDA, EXPTP_PANDA);

extern const bool view_frustum_cull;
extern const bool pstats_unused_states;

extern const string threading_model;
extern const bool auto_flip;
extern const bool yield_timeslice;

extern const string screenshot_filename;
extern const string screenshot_extension;

extern EXPCL_PANDA const bool multiple_windows;

extern EXPCL_PANDA void init_libdisplay();

#endif /* CONFIG_DISPLAY_H */
