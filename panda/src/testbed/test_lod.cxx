// Filename: test_lod.cxx
// Created by:  drose (24Sep04)
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
#include "lodNode.h"
#include "fadeLodNode.h"
#include "loader.h"

PandaFramework framework;

char *filename_a = "smiley";
char *filename_b = "frowney";

void
create_lod_node(NodePath &models, const char *a, const char *b) {
  Loader loader;
  PT(PandaNode) smiley = loader.load_sync(a);
  PT(PandaNode) frowney = loader.load_sync(b);

  PT(LODNode) lod = new FadeLODNode("lod");
  //PT(LODNode) lod = new LODNode("lod");
  if (!smiley.is_null()) {
    lod->add_child(smiley);
    lod->add_switch(10, 0);
  }
  if (!frowney.is_null()) {
    lod->add_child(frowney);
    lod->add_switch(1000, 10);
  }

  NodePath instance1(models.attach_new_node("instance1"));
  instance1.set_pos(2, 0, 0);
  instance1.attach_new_node(lod);

  NodePath instance2(models.attach_new_node("instance2"));
  instance2.set_pos(-2, 0, 0);
  instance2.attach_new_node(lod);
}


int
main(int argc, char *argv[]) {
  framework.open_framework(argc, argv);
  framework.set_window_title("LOD Test");

  WindowFramework *window = framework.open_window();
  if (window != (WindowFramework *)NULL) {
    // We've successfully opened a window.

    window->enable_keyboard();
    window->setup_trackball();
    framework.get_models().instance_to(window->get_render());

    create_lod_node(framework.get_models(),
                    argc > 1 ? argv[1] : filename_a,
                    argc > 2 ? argv[2] : filename_b);

    window->center_trackball(framework.get_models());

    // Open another window too.
    WindowFramework *window2 = framework.open_window();
    if (window2 != (WindowFramework *)NULL) {
      window2->enable_keyboard();
      window2->setup_trackball();
      framework.get_models().instance_to(window2->get_render());
      
      window2->center_trackball(framework.get_models());
    }

    framework.enable_default_keys();
    framework.main_loop();
  }

  return (0);
}
