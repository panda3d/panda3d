// Filename: config_framework.cxx
// Created by:  drose (06Sep00)
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

#include "config_framework.h"

#include "dconfig.h"
#include "windowFramework.h"

Configure(config_framework);
NotifyCategoryDef(framework, "");

const int win_width = config_framework.GetInt("win-width", 640);
const int win_height = config_framework.GetInt("win-height", 480);
const int win_origin_x = config_framework.GetInt("win-origin-x", -1);
const int win_origin_y = config_framework.GetInt("win-origin-y", -1);
const bool fullscreen = config_framework.GetBool("fullscreen", false);
const bool undecorated = config_framework.GetBool("undecorated", false);
const bool cursor_hidden = config_framework.GetBool("cursor-hidden", false);
WindowProperties::ZOrder z_order = WindowProperties::Z_normal;
const string window_title = config_framework.GetString("window-title", "Panda");

const float aspect_ratio = config_framework.GetFloat("aspect-ratio", 0.0f);
const bool show_frame_rate_meter = config_framework.GetBool("show-frame-rate-meter", false);

// The default window background color.
const float win_background_r = config_framework.GetFloat("win-background-r", 0.41);
const float win_background_g = config_framework.GetFloat("win-background-g", 0.41);
const float win_background_b = config_framework.GetFloat("win-background-b", 0.41);

const string record_session = config_framework.GetString("record-session", "");
const string playback_session = config_framework.GetString("playback-session", "");


ConfigureFn(config_framework) {
  string string_z_order = config_framework.GetString("z-order", "normal");
  if (string_z_order == "bottom") {
    z_order = WindowProperties::Z_bottom;
  } else if (string_z_order == "top") {
    z_order = WindowProperties::Z_top;
  } else if (!(string_z_order == "normal")) {
    framework_cat.warning()
      << "Unknown z-order: " << string_z_order << "\n";
  }

  WindowFramework::init_type();
}
