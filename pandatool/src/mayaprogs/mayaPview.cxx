// Filename: mayaPview.cxx
// Created by:  drose (10Mar03)
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

#include "pandaFramework.h"

#include "pre_maya_include.h"
#include <maya/MSimple.h>
#include "post_maya_include.h"

static PandaFramework framework;
static bool opened_framework = false;

// This Maya macro sets up a class that meets the plug-in requirements
// for a simple command.
DeclareSimpleCommand(MayaPview, "VR Studio", "1.0");

MStatus MayaPview::
doIt(const MArgList &) {
  if (!opened_framework) {
    int argc = 0;
    char **argv = NULL;
    framework.open_framework(argc, argv);
    framework.set_window_title("Panda Viewer");
    framework.enable_default_keys();
  }

  WindowFramework *window = framework.open_window();
  if (window == (WindowFramework *)NULL) {
    // Couldn't open a window.
    nout << "Couldn't open a window!\n";
    return MS::kFailure;
  }

  // We've successfully opened a window.

  window->enable_keyboard();
  window->setup_trackball();

  window->load_default_model(framework.get_models());
  window->loop_animations();

  framework.clear_exit_flag();
  framework.main_loop();
  return MS::kSuccess;
}
