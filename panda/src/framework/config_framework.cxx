// Filename: config_framework.cxx
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

#include "config_framework.h"

#include "dconfig.h"
#include "windowFramework.h"

Configure(config_framework);
NotifyCategoryDef(framework, "");

ConfigureFn(config_framework) {
  WindowFramework::init_type();
}

const int win_width = config_framework.GetInt("win-width", 640);
const int win_height = config_framework.GetInt("win-height", 480);
const bool fullscreen = config_framework.GetBool("fullscreen", false);
const bool undecorated = config_framework.GetBool("undecorated", false);
const bool cursor_hidden = config_framework.GetBool("cursor-hidden", false);
const float aspect_ratio = config_framework.GetFloat("aspect-ratio", 0.0f);

// The default window background color.
const float win_background_r = config_framework.GetFloat("win-background-r", 0.41);
const float win_background_g = config_framework.GetFloat("win-background-g", 0.41);
const float win_background_b = config_framework.GetFloat("win-background-b", 0.41);
