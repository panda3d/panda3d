// Filename: test_texmem.cxx
// Created by:  drose (03Sep02)
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
#include "geomQuad.h"
#include "textureAttrib.h"

NodePath bogus_scene;

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
    bogus_scene.remove_node();
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

  PTA_Colorf colors;
  colors.push_back(Colorf(1.0f, 1.0f, 1.0f, 1.0f));
  for (int yi = 0; yi < num_quads_side; yi++) {
    float y0 = (float)yi / (float)num_quads_side;
    float y1 = (float)(yi + 1) / (float)num_quads_side;
    for (int xi = 0; xi < num_quads_side; xi++) {
      float x0 = (float)xi / (float)num_quads_side;
      float x1 = (float)(xi + 1) / (float)num_quads_side;

      PNMImage bogus_image(tex_x_size, tex_y_size);
      bogus_image.fill(x0, (xi + yi) & 1, y0);
      
      PT(Texture) tex = new Texture;
      tex->set_minfilter(Texture::FT_linear_mipmap_linear);
      tex->load(bogus_image);

      PTA_Vertexf coords;
      PTA_TexCoordf uvs;
      coords.push_back(Vertexf(x0, 0.0f, y0));
      coords.push_back(Vertexf(x1, 0.0f, y0));
      coords.push_back(Vertexf(x1, 0.0f, y1));
      coords.push_back(Vertexf(x0, 0.0f, y1));
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
