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

#ifndef HAVE_GETOPT
  #include "gnu_getopt.h"
#else
  #ifdef HAVE_GETOPT_H
    #include <getopt.h>
  #endif
#endif

PandaFramework framework;

void
event_W(CPT_Event, void *) {
  // shift-W: open a new window on the same scene.

  // If we already have a window, use the same GSG.
  GraphicsPipe *pipe = (GraphicsPipe *)NULL;
  GraphicsStateGuardian *gsg = (GraphicsStateGuardian *)NULL;

  if (framework.get_num_windows() > 0) {
    WindowFramework *old_window = framework.get_window(0);
    GraphicsWindow *win = old_window->get_graphics_window();
    pipe = win->get_pipe();
    gsg = win->get_gsg();
  }

  WindowFramework *window = framework.open_window(pipe, gsg);
  if (window != (WindowFramework *)NULL) {
    window->enable_keyboard();
    window->setup_trackball();
    framework.get_models().instance_to(window->get_render());
  }
}

void 
usage() {
  cerr <<
    "Usage: pview [opts] model [model ...]\n";
    "       pview -h\n";
}

void 
help() {
  usage();
  cerr << "\n"
    "pview opens a quick Panda window for viewing one or more models and/or\n"
    "animations.\n\n"

    "Options:\n\n"
    
    "  -c\n"
    "      Automatically center models within the viewing window on startup.\n"
    "      This can also be achieved with the 'c' hotkey at runtime.\n\n"

    "  -h\n"
    "      Display this help text.\n\n";
}

int
main(int argc, char *argv[]) {
  framework.open_framework(argc, argv);
  framework.set_window_title("Panda Viewer");

  bool auto_center = false;

  extern char *optarg;
  extern int optind;
  static const char *optflags = "ch";
  int flag = getopt(argc, argv, optflags);

  while (flag != EOF) {
    switch (flag) {
    case 'c':
      auto_center = true;
      break;

    case 'h':
      help();
      return 1;
    case '?':
      usage();
      return 1;
    default:
      cerr << "Unhandled switch: " << flag << endl;
      break;
    }
    flag = getopt(argc, argv, optflags);
  }
  argc -= (optind - 1);
  argv += (optind - 1);

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

    if (auto_center) {
      window->center_trackball(framework.get_models());
    }

    framework.enable_default_keys();
    framework.define_key("shift-w", "open a new window", event_W, NULL);
    framework.main_loop();
    framework.report_frame_rate(nout);
  }

  return (0);
}
