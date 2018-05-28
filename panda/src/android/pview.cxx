/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pview.cxx
 * @author rdb
 * @date 2013-01-12
 */

#include "pandaFramework.h"
#include "pandaSystem.h"
#include "texturePool.h"
#include "multitexReducer.h"
#include "sceneGraphReducer.h"
#include "partGroup.h"
#include "cardMaker.h"
#include "bamCache.h"
#include "virtualFileSystem.h"

int main(int argc, char **argv) {
  PandaFramework framework;
  framework.open_framework(argc, argv);
  framework.set_window_title("Panda Viewer");

  int hierarchy_match_flags = PartGroup::HMF_ok_part_extra |
                              PartGroup::HMF_ok_anim_extra;

  WindowFramework *window = framework.open_window();
  if (window != nullptr) {
    // We've successfully opened a window.

    NodePath loading_np;

    if (true) {
      // Put up a "loading" message for the user's benefit.
      NodePath aspect_2d = window->get_aspect_2d();
      PT(TextNode) loading = new TextNode("loading");
      loading_np = aspect_2d.attach_new_node(loading);
      loading_np.set_scale(0.125f);
      loading->set_text_color(1.0f, 1.0f, 1.0f, 1.0f);
      loading->set_shadow_color(0.0f, 0.0f, 0.0f, 1.0f);
      loading->set_shadow(0.04, 0.04);
      loading->set_align(TextNode::A_center);
      loading->set_text("Loading...");

      // Allow a couple of frames to go by so the window will be fully created
      // and the text will be visible.
      Thread *current_thread = Thread::get_current_thread();
      framework.do_frame(current_thread);
      framework.do_frame(current_thread);
    }

    window->enable_keyboard();
    window->setup_trackball();
    framework.get_models().instance_to(window->get_render());
    if (argc < 2) {
      // If we have no arguments, get that trusty old triangle
      // out.
      window->load_default_model(framework.get_models());
    } else {
      if (!window->load_models(framework.get_models(), argc, argv)) {
        framework.close_framework();
        return 1;
      }
    }

    window->loop_animations(hierarchy_match_flags);

    // Make sure the textures are preloaded.
    framework.get_models().prepare_scene(window->get_graphics_output()->get_gsg());

    loading_np.remove_node();

    window->center_trackball(framework.get_models());
    window->set_anim_controls(true);

    framework.enable_default_keys();
    framework.main_loop();
    framework.report_frame_rate(nout);
  } else {
    assert(false);
  }

  framework.close_framework();
  return (0);
}
