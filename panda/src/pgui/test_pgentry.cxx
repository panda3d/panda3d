// Filename: test_pgentry.cxx
// Created by:  drose (30Apr04)
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

#include "pandaFramework.h"
#include "pgEntry.h"

PandaFramework framework;

int
main(int argc, char *argv[]) {
  framework.open_framework(argc, argv);
  framework.set_window_title("Panda Viewer");

  WindowFramework *window = framework.open_window();
  if (window != (WindowFramework *)NULL) {
    window->enable_keyboard();

    NodePath aspect2d = window->get_aspect_2d();
    PGEntry *entry = new PGEntry("entry");
    NodePath entry_np = aspect2d.attach_new_node(entry);
    entry_np.set_scale(0.1);
    entry_np.set_pos(-0.5, 0, 0.2);

    entry->setup(10, 4);

    framework.define_key("escape", "close window", 
                         PandaFramework::event_esc, &framework);

    framework.main_loop();
  }

  return (0);
}
