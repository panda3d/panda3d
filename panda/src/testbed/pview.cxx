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
#include "qpcamera.h"
#include "perspectiveLens.h"
#include "pandaNode.h"
#include "textureAttrib.h"
#include "clockObject.h"
#include "qpgeomNode.h"
#include "geomTri.h"
#include "texture.h"
#include "texturePool.h"
#include "dSearchPath.h"
#include "loader.h"
#include "auto_bind.h"
#include "pStatClient.h"
#include "notify.h"
#include "qpnodePath.h"
#include "cullBinManager.h"

// These are in support of legacy data graph operations.
#include "namedNode.h"
#include "mouse.h"
#include "mouseWatcher.h"
#include "trackball.h"
#include "transform2sg.h"
#include "dataRelation.h"
#include "dataGraphTraversal.h"
#include "buttonThrower.h"
#include "modifierButtons.h"
#include "keyboardButton.h"
#include "event.h"
#include "eventQueue.h"
#include "eventHandler.h"

// Use dconfig to read a few Configrc variables.
Configure(config_pview);
ConfigureFn(config_pview) {
}

static const int win_width = config_pview.GetInt("win-width", 640);
static const int win_height = config_pview.GetInt("win-height", 480);

// As long as this is true, the main loop will continue running.
bool run_flag = true;

// These are used by report_frame_rate().
static double start_time = 0.0;
static int start_frame_count = 0;

// A priority number high enough to override any model file settings.
static const int override_priority = 100;

// This is the main scene graph.
qpNodePath render;

void 
report_frame_rate() {
  double now = ClockObject::get_global_clock()->get_frame_time();
  double delta = now - start_time;
  
  int frame_count = ClockObject::get_global_clock()->get_frame_count();
  int num_frames = frame_count - start_frame_count;
  if (num_frames > 0) {
    nout << endl << num_frames << " frames in " << delta << " seconds" << endl;
    double x = ((double)num_frames) / delta;
    nout << x << " fps average (" << 1000.0 / x << "ms)" << endl;

    // Reset the frame rate counter for the next press of 'f'.
    start_time = now;
    start_frame_count = frame_count;
  }
}

PT(GraphicsPipe)
make_pipe() {
  // We use the GraphicsPipe factory to make us a renderable pipe
  // without knowing exactly what kind of pipes we have available.  We
  // don't care, so long as we can render to it interactively.

  // This depends on the shared library or libraries (DLL's to you
  // Windows folks) that have been loaded in at runtime from the
  // load-display Configrc variable.
  GraphicsPipe::resolve_modules();

  nout << "Known pipe types:" << endl;
  GraphicsPipe::get_factory().write_types(nout, 2);

  PT(GraphicsPipe) pipe;
  pipe = GraphicsPipe::get_factory().
    make_instance(InteractiveGraphicsPipe::get_class_type());

  if (pipe == (GraphicsPipe*)0L) {
    nout << "No interactive pipe is available!  Check your Configrc!\n";
    exit(1);
  }

  return pipe;
}

PT(GraphicsWindow)
make_window(GraphicsPipe *pipe, GraphicsEngine *engine) {
  // Now create a window on that pipe.
  GraphicsWindow::Properties window_prop;
  window_prop._xorg = 0;
  window_prop._yorg = 0;
  window_prop._xsize = win_width;
  window_prop._ysize = win_height;
  window_prop._title = "Panda Viewer";
  //  window_prop._fullscreen = true;

  PT(GraphicsWindow) window = pipe->make_window(window_prop);
  engine->add_window(window);
  
  return window;
}

PT(qpCamera)
make_camera(GraphicsWindow *window) {
  // Get the first channel on the window.  This will be the only
  // channel on non-SGI hardware.
  PT(GraphicsChannel) channel = window->get_channel(0);

  // Make a layer on the channel to hold our display region.
  PT(GraphicsLayer) layer = channel->make_layer();

  // And create a display region that covers the entire window.
  PT(DisplayRegion) dr = layer->make_display_region();

  // Finally, we need a camera to associate with the display region.
  PT(qpCamera) camera = new qpCamera("camera");
  PT(Lens) lens = new PerspectiveLens;
  lens->set_film_size(win_width, win_height);
  camera->set_lens(lens);
  dr->set_qpcamera(qpNodePath(camera));

  return camera;
}


void
make_default_geometry(PandaNode *parent) {
  PTA_Vertexf coords;
  PTA_TexCoordf uvs;
  PTA_Normalf norms;
  PTA_Colorf colors;
  PTA_ushort cindex;
  
  coords.push_back(Vertexf::rfu(0.0, 0.0, 0.0));
  coords.push_back(Vertexf::rfu(1.0, 0.0, 0.0));
  coords.push_back(Vertexf::rfu(0.0, 0.0, 1.0));
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

  CPT(RenderState) state = RenderState::make_empty();
  Texture *tex = TexturePool::load_texture("rock-floor.rgb");
  if (tex != (Texture *)NULL) {
    tex->set_minfilter(Texture::FT_linear);
    tex->set_magfilter(Texture::FT_linear);
    state = state->add_attrib(TextureAttrib::make(tex));
  }
  
  qpGeomNode *geomnode = new qpGeomNode("tri");
  parent->add_child(geomnode);
  geomnode->add_geom(geom, state);
}

void
get_models(PandaNode *parent, int argc, char *argv[]) {
  if (argc < 2) {
    // In the absence of any models on the command line, load up a
    // default triangle so we at least have something to look at.
    make_default_geometry(parent);

  } else {
    Loader loader;
    DSearchPath local_path(".");

    for (int i = 1; i < argc; i++) {
      Filename filename = argv[i];
      
      nout << "Loading " << filename << "\n";

      // First, we always try to resolve a filename from the current
      // directory.  This means a local filename will always be found
      // before the model path is searched.
      filename.resolve_filename(local_path);

      PT(PandaNode) node = loader.qpload_sync(filename);
      if (node == (PandaNode *)NULL) {
        nout << "Unable to load " << filename << "\n";

      } else {
        node->ls(nout, 0);
        parent->add_child(node);
      }
    }
  }
}

NamedNode * 
setup_mouse(NamedNode *data_root, GraphicsWindow *window) {
  MouseAndKeyboard *mouse = new MouseAndKeyboard(window, 0);
  new DataRelation(data_root, mouse);

  // Create a ButtonThrower to throw events from the keyboard.
  PT(ButtonThrower) bt = new ButtonThrower("kb-events");
  ModifierButtons mods;
  mods.add_button(KeyboardButton::shift());
  mods.add_button(KeyboardButton::control());
  mods.add_button(KeyboardButton::alt());
  bt->set_modifier_buttons(mods);
  new DataRelation(mouse, bt);

  return mouse;
}

void 
setup_trackball(NamedNode *mouse, qpCamera *camera) {
  PT(Trackball) trackball = new Trackball("trackball");
  trackball->set_pos(LVector3f::forward() * 50.0);
  new DataRelation(mouse, trackball);

  PT(Transform2SG) tball2cam = new Transform2SG("tball2cam");
  tball2cam->set_node(camera);
  new DataRelation(trackball, tball2cam);
}

void
event_esc(CPT_Event) {
  // The Escape or q key was pressed.  Exit the application.
  run_flag = false;
}

void
event_f(CPT_Event) {
  // 'f' : report frame rate.
  report_frame_rate();
}

void
event_t(CPT_Event) {
  // 't' : toggle texture.
  static bool texture_off = false;

  texture_off = !texture_off;
  if (texture_off) {
    nout << "Disabling texturing.\n";
    render.set_texture_off(override_priority);
  } else {
    nout << "Enabling texturing.\n";
    render.clear_texture();
  }
}

void
event_w(CPT_Event) {
  // 'w' : toggle wireframe.
  static bool wireframe = false;

  wireframe = !wireframe;
  if (wireframe) {
    nout << "Setting wireframe mode.\n";
    render.set_render_mode_wireframe(override_priority);
  } else {
    nout << "Clearing wireframe mode.\n";
    render.clear_render_mode();
  }
}

void
event_s(CPT_Event) {
  // 's' : toggle state sorting by putting everything into an 'unsorted' bin.
  static bool sorting_off = false;

  sorting_off = !sorting_off;
  if (sorting_off) {
    nout << "Disabling state sorting.\n";
    render.set_bin("unsorted", 0, override_priority);
  } else {
    nout << "Enabling state sorting.\n";
    render.clear_bin();
  }
}

void
event_S(CPT_Event) {
  // shift 'S' : active PStats.
#ifdef DO_PSTATS
  nout << "Connecting to stats host" << endl;
  PStatClient::connect();
#else
  nout << "Stats host not supported." << endl;
#endif
}

void
event_A(CPT_Event) {
  // shift 'A' : deactive PStats.
#ifdef DO_PSTATS
  if (PStatClient::is_connected()) {
    nout << "Disconnecting from stats host" << endl;
    PStatClient::disconnect();
  } else {
    nout << "Stats host is already disconnected." << endl;
  }
#else
  nout << "Stats host not supported." << endl;
#endif
}

int
main(int argc, char *argv[]) {
  // First, we need a GraphicsPipe, before we can open a window.
  PT(GraphicsPipe) pipe = make_pipe();

  // We also need a GraphicsEngine to manage the rendering process.
  GraphicsEngine *engine = new GraphicsEngine;

  // Now open a window and get a camera.
  PT(GraphicsWindow) window = make_window(pipe, engine);
  PT(qpCamera) camera = make_camera(window);

  // Now we just need to make a scene graph for the camera to render.
  render = qpNodePath(new PandaNode("render"));
  render.attach_new_node(camera);
  camera->set_scene(qpNodePath(render));

  // We will take advantage of this bin if the user toggles state
  // sorting, above, in event_s().
  CullBinManager::get_global_ptr()->add_bin("unsorted", CullBinManager::BT_unsorted, 0);

  // Set up a data graph for tracking user input.  For now, this uses
  // the old-style graph interface.
  PT(NamedNode) data_root = new NamedNode("data_root");
  NamedNode *mouse = setup_mouse(data_root, window);
  setup_trackball(mouse, camera);

  // Use an event handler to manage keyboard events.
  EventHandler event_handler(EventQueue::get_global_event_queue());
  event_handler.add_hook("escape", event_esc);
  event_handler.add_hook("q", event_esc);
  event_handler.add_hook("f", event_f);
  event_handler.add_hook("t", event_t);
  event_handler.add_hook("w", event_w);
  event_handler.add_hook("s", event_s);
  event_handler.add_hook("shift-s", event_S);
  event_handler.add_hook("shift-a", event_A);


  // Put something in the scene graph to look at.
  get_models(render.node(), argc, argv);

  // If we happened to load up both a character file and its matching
  // animation file, attempt to bind them together now and start the
  // animations looping.
  AnimControlCollection anim_controls;
  auto_bind(render.node(), anim_controls, ~0);
  anim_controls.loop_all(true);


  // Tick the clock once so we won't count the time spent loading up
  // files, above, in our frame rate average.
  ClockObject::get_global_clock()->tick();

  // This is our main update loop.  Loop here until someone
  // (e.g. event_esc) sets run_flag to false.
  while (run_flag) {
    traverse_data_graph(data_root);
    event_handler.process_events();
    engine->render_frame();
  } 

  report_frame_rate();
  delete engine;
  return (0);
}
