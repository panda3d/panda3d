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

#include "graphicsWindow.h"
#include "graphicsChannel.h"
#include "graphicsLayer.h"
#include "graphicsEngine.h"
#include "graphicsPipe.h"
#include "interactiveGraphicsPipe.h"
#include "displayRegion.h"
#include "camera.h"
#include "perspectiveLens.h"
#include "pandaNode.h"
#include "textureAttrib.h"
#include "clockObject.h"
#include "qpgeomNode.h"
#include "geomTri.h"
#include "texture.h"
#include "texturePool.h"

PT(GraphicsPipe)
make_pipe() {
  // We use the GraphicsPipe factory to make us a renderable pipe
  // without knowing exactly what kind of pipes we have available.  We
  // don't care, so long as we can render to it interactively.

  // This depends on the shared library or libraries (DLL's to you
  // Windows folks) that have been loaded in at runtime from the
  // load-display Configrc variable.
  GraphicsPipe::resolve_modules();

  cerr << "Known pipe types:" << endl;
  GraphicsPipe::get_factory().write_types(cerr, 2);

  PT(GraphicsPipe) pipe;
  pipe = GraphicsPipe::get_factory().
    make_instance(InteractiveGraphicsPipe::get_class_type());

  if (pipe == (GraphicsPipe*)0L) {
    cerr << "No interactive pipe is available!  Check your Configrc!\n";
    exit(1);
  }

  return pipe;
}

PT(Camera)
setup_window(GraphicsPipe *pipe, GraphicsEngine *engine) {
  // Now create a window on that pipe.
  GraphicsWindow::Properties window_prop;
  window_prop._xorg = 0;
  window_prop._yorg = 0;
  window_prop._xsize = 640;
  window_prop._ysize = 480;
  window_prop._title = "Panda Viewer";
  //  window_prop._fullscreen = true;

  PT(GraphicsWindow) window = pipe->make_window(window_prop);
  engine->add_window(window);

  // Get the first channel on the window.  This will be the only
  // channel on non-SGI hardware.
  PT(GraphicsChannel) channel = window->get_channel(0);

  // Make a layer on the channel to hold our display region.
  PT(GraphicsLayer) layer = channel->make_layer();

  // And create a display region that covers the entire window.
  PT(DisplayRegion) dr = layer->make_display_region();

  // Finally, we need a camera to associate with the display region.
  PT(Camera) camera = new Camera;
  PT(Lens) lens = new PerspectiveLens;
  lens->set_film_size(640, 480);
  camera->set_lens(lens);
  dr->set_camera(camera);

  return camera;
}


void
make_default_geometry(PandaNode *parent) {
  PTA_Vertexf coords;
  PTA_TexCoordf uvs;
  PTA_Normalf norms;
  PTA_Colorf colors;
  PTA_ushort cindex;
  
  coords.push_back(Vertexf::rfu(0.0, 20.0, 0.0));
  coords.push_back(Vertexf::rfu(1.0, 20.0, 0.0));
  coords.push_back(Vertexf::rfu(0.0, 20.0, 1.0));
  uvs.push_back(TexCoordf(0.0, 0.0));
  uvs.push_back(TexCoordf(1.0, 0.0));
  uvs.push_back(TexCoordf(0.0, 1.0));
  norms.push_back(Normalf::back());
  colors.push_back(Colorf(0.5, 0.5, 1.0, 1.0));
  cindex.push_back(0);
  cindex.push_back(0);
  cindex.push_back(0);
  
  PT(GeomTri) geom = new GeomTri;
  geom->set_num_prims(1);
  geom->set_coords(coords);
  geom->set_texcoords(uvs, G_PER_VERTEX);
  geom->set_normals(norms, G_PER_PRIM);
  geom->set_colors(colors, G_PER_VERTEX, cindex);
  
  qpGeomNode *geomnode = new qpGeomNode("tri");
  parent->add_child(geomnode);
  geomnode->add_geom(geom, RenderState::make_empty());
  
  Texture *tex = TexturePool::load_texture("rock-floor.rgb");
  if (tex != (Texture *)NULL) {
    tex->set_minfilter(Texture::FT_linear);
    tex->set_magfilter(Texture::FT_linear);
    geomnode->set_attrib(TextureAttrib::make(tex));
  }
}

void
get_models(PandaNode *parent, int argc, char *argv[]) {
  make_default_geometry(parent);
  /*
  Loader loader;

  for (int i = 1; i < argc; i++) {
    Filename filename = argv[i];

    cerr << "Loading " << filename << "\n";
    PT_Node node = loader.load_sync(filename);
    if (node == (Node *)NULL) {
      cerr << "Unable to load " << filename << "\n";
    } else {
      new RenderRelation(parent, node);
    }
  }
  */
}


int
main(int argc, char *argv[]) {
  // First, we need a GraphicsPipe, before we can open a window.
  PT(GraphicsPipe) pipe = make_pipe();

  // We also need a GraphicsEngine to manage the rendering process.
  GraphicsEngine *engine = new GraphicsEngine;

  // Now open a window and get a camera.
  PT(Camera) camera = setup_window(pipe, engine);

  // Now we just need to make a scene graph for the camera to render.
  PT(PandaNode) render = new PandaNode("render");
  camera->set_qpscene(render);

  // Put something in the scene graph to look at.
  get_models(render, argc, argv);

  /*
  // Put the scene a distance in front of our face so we can see it.
  LMatrix4f mat = LMatrix4f::translate_mat(0, 20, 0);
  TransformTransition *tt = new TransformTransition(mat);
  render_arc->set_transition(tt);
  */


  // This is our main update loop.  Run for 5 seconds, then shut down.
  ClockObject *clock = ClockObject::get_global_clock();
  clock->tick();
  double now = clock->get_frame_time();
  while (clock->get_frame_time() - now < 5.0) {
    clock->tick();
    engine->render_frame();
  } 

  cerr << "Exiting.\n";
  delete engine;
  return (0);
}
