// Filename: config_framework.h
// Created by:  drose (06Sep00)
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

#ifndef CONFIG_FRAMEWORK_H
#define CONFIG_FRAMEWORK_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"

NotifyCategoryDecl(framework, EXPCL_FRAMEWORK, EXPTP_FRAMEWORK);

// Configure variables for framework package.
extern const int win_width;
extern const int win_height;
extern const bool fullscreen;
extern const bool undecorated;
extern const bool cursor_hidden;
extern const float aspect_ratio;

extern const float win_background_r;
extern const float win_background_g;
extern const float win_background_b;

#endif
