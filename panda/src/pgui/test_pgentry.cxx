/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_pgentry.cxx
 * @author drose
 * @date 2004-04-30
 */

#include "pandaFramework.h"
#include "pgEntry.h"

PandaFramework framework;

int
main(int argc, char *argv[]) {
  framework.open_framework(argc, argv);
  framework.set_window_title("Panda Viewer");

  WindowFramework *window = framework.open_window();
  if (window != nullptr) {
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
