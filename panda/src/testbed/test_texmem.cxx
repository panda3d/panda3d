/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_texmem.cxx
 * @author drose
 * @date 2002-09-03
 */

#include "pandaFramework.h"
#include "cardMaker.h"
#include "texture.h"
#include "pnmImage.h"
#include "cmath.h"
#include "mathNumbers.h"

NodePath bogus_scene;
NodePath old_bogus_scene;

void
event_T(const Event *, void *data) {
  PandaFramework *framework = (PandaFramework *)data;
  WindowFramework *wf = framework->get_window(0);

  NodePath models = framework->get_models();
  NodePath render = wf->get_render();

  if (!bogus_scene.is_empty()) {
    // We are undoing a previous shift-t.
    old_bogus_scene = bogus_scene;
    old_bogus_scene.detach_node();
    bogus_scene = NodePath();
    models.show();
    return;
  }

  // We are doing a new shift-t.  Hide the normal models, and create a new
  // bogus node to show the texture grid object.
  models.hide();
  bogus_scene = render.attach_new_node("bogus");

  // Try to force a flush of the texture memory by making a scene with lots of
  // bogus textures.
  static const int num_quads_side = 20;
  static const int tex_x_size = 256;
  static const int tex_y_size = 256;

  std::cerr << "Loading " << num_quads_side * num_quads_side << " textures at "
       << tex_x_size << ", " << tex_y_size << "\n";

  PNMImage white_center(tex_x_size / 4, tex_y_size / 4);
  white_center.fill(1.0f, 1.0f, 1.0f);

  for (int yi = 0; yi < num_quads_side; yi++) {
    PN_stdfloat y0 = (PN_stdfloat)yi / (PN_stdfloat)num_quads_side;
    PN_stdfloat y1 = (PN_stdfloat)(yi + 1) / (PN_stdfloat)num_quads_side;

    // Map the x, y vertices onto a sphere just for fun.
    PN_stdfloat px0 = ccos((y0 - 0.5f) * MathNumbers::pi_f);
    PN_stdfloat px1 = ccos((y1 - 0.5f) * MathNumbers::pi_f);
    PN_stdfloat py0 = csin((y0 - 0.5f) * MathNumbers::pi_f);
    PN_stdfloat py1 = csin((y1 - 0.5f) * MathNumbers::pi_f);
    for (int xi = 0; xi < num_quads_side; xi++) {
      PN_stdfloat x0 = (PN_stdfloat)xi / (PN_stdfloat)num_quads_side;
      PN_stdfloat x1 = (PN_stdfloat)(xi + 1) / (PN_stdfloat)num_quads_side;

      PN_stdfloat hx0 = ccos(x0 * MathNumbers::pi_f * 2.0f);
      PN_stdfloat hx1 = ccos(x1 * MathNumbers::pi_f * 2.0f);
      PN_stdfloat hy0 = csin(x0 * MathNumbers::pi_f * 2.0f);
      PN_stdfloat hy1 = csin(x1 * MathNumbers::pi_f * 2.0f);

      PNMImage bogus_image(tex_x_size, tex_y_size);
      bogus_image.fill(x0, (xi + yi) & 1, y0);
      bogus_image.copy_sub_image(white_center,
                                 (tex_x_size - white_center.get_x_size()) / 2,
                                 (tex_y_size - white_center.get_y_size()) / 2);

      PT(Texture) tex = new Texture;
      tex->set_minfilter(SamplerState::FT_linear_mipmap_linear);
      tex->load(bogus_image);

      CardMaker cm("card");
      cm.set_frame(Vertexf(hx0 * px0, hy0 * px0, py0),
                   Vertexf(hx1 * px0, hy1 * px0, py0),
                   Vertexf(hx1 * px1, hy1 * px1, py1),
                   Vertexf(hx0 * px1, hy0 * px1, py1));
      NodePath card = bogus_scene.attach_new_node(cm.generate());
      card.set_texture(tex);
    }
  }
  std::cerr << "Done.\n";
}

int
main(int argc, char *argv[]) {
  PandaFramework framework;
  framework.open_framework(argc, argv);
  framework.set_window_title("Panda Viewer");

  WindowFramework *window = framework.open_window();
  if (window != nullptr) {
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
    framework.define_key("shift-t", "test texture memory", event_T, &framework);
    framework.main_loop();
  }

  framework.report_frame_rate(nout);
  return (0);
}
