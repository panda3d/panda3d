// Filename: pview.cxx
// Created by:  drose (25Feb02)
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

int
main(int argc, char *argv[]) {
  PandaFramework framework;
  framework.open_framework(argc, argv);
  framework.set_window_title("Panda Viewer");

  WindowFramework *window = framework.open_window();
  if (window != (WindowFramework *)NULL) {
    // We've successfully opened a window.

    window->enable_keyboard();
    window->setup_trackball();
    framework.get_models().instance_to(window->get_render());
    if (argc < 2) {
      // If we have no arguments, get that trusty old triangle out.
      window->load_default_model(framework.get_models());
    } else {
      window->load_models(framework.get_models(), argc, argv);
    }
    window->loop_animations();

    framework.enable_default_keys();
    framework.main_loop();
  }

  framework.report_frame_rate(nout);
  return (0);
}
