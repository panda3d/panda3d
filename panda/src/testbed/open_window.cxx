// Filename: dxwindow.cxx
// Created by:  drose (09Jan02)
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

#include "graphicsWindow.h"
#include "graphicsChannel.h"
#include "graphicsLayer.h"
#include "displayRegion.h"
#include "camera.h"
#include "perspectiveLens.h"
#include "namedNode.h"
#include "renderRelation.h"
#include "transformTransition.h"
#include "loader.h"
#include "clockObject.h"

#ifdef USE_GLX
  #include "glxGraphicsPipe.h"
#else
  #include "wglGraphicsPipe.h"
  #include "wdxGraphicsPipe.h"
  #include "wcrGraphicsPipe.h"
#endif


// This program demonstrates creating a graphics window in Panda
// explicitly, without using ChanCfg as an interface wrapper.
// Normally we use ChanCfg to create the window and automatically set
// up many of its parameters.

void
get_models(Node *root, int argc, char *argv[]) {
  Loader loader;

  for (int i = 1; i < argc; i++) {
    Filename filename = argv[i];

    cerr << "Loading " << filename << "\n";
    PT_Node node = loader.load_sync(filename);
    if (node == (Node *)NULL) {
      cerr << "Unable to load " << filename << "\n";
    } else {
      new RenderRelation(root, node);
    }
  }
}


int
main(int argc, char *argv[]) {
  // First, create a GraphicsPipe.  For this we need a PipeSpecifier
  // to specify the parameters we want to pass to the pipe.  Yeah, I
  // know it's a weird way to pass parameters to an object.

  PipeSpecifier pipe_spec;
  PT(GraphicsPipe) pipe;

#ifdef USE_GLX
  pipe = new glxGraphicsPipe(pipe_spec);
#else
  // Yes, this is a stupid hard coded switch, but this is a test app, and
  // I'm just making it easier to see the choices (and switch between them).
  switch (2) {
    case 1:
      // ...OpenGL pipe
      pipe = new wglGraphicsPipe(pipe_spec);
      break;
    case 2:
      // ...Chromium OpenGL pipe (cr == chromium)
      pipe = new wcrGraphicsPipe(pipe_spec);
      break;
    default:
      // ...DirectX pipe
      pipe = new wdxGraphicsPipe(pipe_spec);
      break;
  }
#endif

  // Now create a window on that pipe.
  GraphicsWindow::Properties window_prop;
  window_prop._xorg = 0;
  window_prop._yorg = 0;
  window_prop._xsize = 640;
  window_prop._ysize = 480;
  window_prop._title = "Window";
  //  window_prop._fullscreen = true;

  PT(GraphicsWindow) window = pipe->make_window(window_prop);

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

  // Window setup is complete.  Now we just need to make a scene graph
  // for the camera to render.
  PT_Node render_top = new NamedNode("render_top");
  camera->set_scene(render_top);

  PT_Node render = new NamedNode("render");
  NodeRelation *render_arc = new RenderRelation(render_top, render);

  // Put something in the scene graph to look at.
  get_models(render, argc, argv);

  // Put the scene a distance in front of our face so we can see it.
  LMatrix4f mat = LMatrix4f::translate_mat(0, 20, 0);
  TransformTransition *tt = new TransformTransition(mat);
  render_arc->set_transition(tt);


  // This is our main update loop.  Run for 5 seconds, then shut down.
  ClockObject *clock = ClockObject::get_global_clock();
  clock->tick();
  double now = clock->get_frame_time();
  while (clock->get_frame_time() - now < 5.0) {
    clock->tick();
    window->get_gsg()->render_frame();
    window->update();
  } 

  cerr << "Exiting.\n";
  return (0);
}
