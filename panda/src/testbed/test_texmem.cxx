// Filename: test_texmem.cxx
// Created by:  drose (03Sep02)
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
#include "geomQuad.h"
#include "textureAttrib.h"
#include "cmath.h"
#include "mathNumbers.h"

NodePath bogus_scene;
NodePath old_bogus_scene;

void
event_T(CPT_Event, void *data) {
  PandaFramework *framework = (PandaFramework *)data;
  WindowFramework *wf = framework->get_window(0);

  GraphicsStateGuardian *gsg = wf->get_graphics_window()->get_gsg();
  Camera *camera = wf->get_camera(0);
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

  // We are doing a new shift-t.  Hide the normal models, and create a
  // new bogus node to show the texture grid object.
  models.hide();
  bogus_scene = render.attach_new_node("bogus");

  // Try to force a flush of the texture memory by making a scene with
  // lots of bogus textures.
  static const int num_quads_side = 20;
  static const int tex_x_size = 256;
  static const int tex_y_size = 256;

  cerr << "Loading " << num_quads_side * num_quads_side << " textures at " 
       << tex_x_size << ", " << tex_y_size << "\n";

  GeomNode *gnode = new GeomNode("quads");
  bogus_scene.attach_new_node(gnode);

  PNMImage white_center(tex_x_size / 4, tex_y_size / 4);
  white_center.fill(1.0f, 1.0f, 1.0f);

  PTA_Colorf colors;
  colors.push_back(Colorf(1.0f, 1.0f, 1.0f, 1.0f));
  for (int yi = 0; yi < num_quads_side; yi++) {
    float y0 = (float)yi / (float)num_quads_side;
    float y1 = (float)(yi + 1) / (float)num_quads_side;

    // Map the x, y vertices onto a sphere just for fun.
    float px0 = ccos((y0 - 0.5f) * MathNumbers::pi_f);
    float px1 = ccos((y1 - 0.5f) * MathNumbers::pi_f);
    float py0 = csin((y0 - 0.5f) * MathNumbers::pi_f);
    float py1 = csin((y1 - 0.5f) * MathNumbers::pi_f);
    for (int xi = 0; xi < num_quads_side; xi++) {
      float x0 = (float)xi / (float)num_quads_side;
      float x1 = (float)(xi + 1) / (float)num_quads_side;

      float hx0 = ccos(x0 * MathNumbers::pi_f * 2.0f);
      float hx1 = ccos(x1 * MathNumbers::pi_f * 2.0f);
      float hy0 = csin(x0 * MathNumbers::pi_f * 2.0f);
      float hy1 = csin(x1 * MathNumbers::pi_f * 2.0f);

      PNMImage bogus_image(tex_x_size, tex_y_size);
      bogus_image.fill(x0, (xi + yi) & 1, y0);
      bogus_image.copy_sub_image(white_center,
                                 (tex_x_size - white_center.get_x_size()) / 2,
                                 (tex_y_size - white_center.get_y_size()) / 2);
      
      PT(Texture) tex = new Texture;
      tex->set_minfilter(Texture::FT_linear_mipmap_linear);
      tex->load(bogus_image);

      PTA_Vertexf coords;
      PTA_TexCoordf uvs;
      coords.push_back(Vertexf(hx0 * px0, hy0 * px0, py0));
      coords.push_back(Vertexf(hx1 * px0, hy1 * px0, py0));
      coords.push_back(Vertexf(hx1 * px1, hy1 * px1, py1));
      coords.push_back(Vertexf(hx0 * px1, hy0 * px1, py1));
      uvs.push_back(TexCoordf(0.0f, 0.0f));
      uvs.push_back(TexCoordf(1.0f, 0.0f));
      uvs.push_back(TexCoordf(1.0f, 1.0f));
      uvs.push_back(TexCoordf(0.0f, 1.0f));
      
      PT(GeomQuad) quad = new GeomQuad;
      quad->set_coords(coords);
      quad->set_colors(colors, G_OVERALL);
      quad->set_num_prims(1);
      quad->set_texcoords(uvs, G_PER_VERTEX);
      gnode->add_geom(quad, RenderState::make(TextureAttrib::make(tex)));
    }
  }
  cerr << "Done.\n";
}

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
    framework.get_event_handler().add_hook("shift-t", event_T, &framework);
    framework.main_loop();
  }

  framework.report_frame_rate(nout);
  return (0);
}
