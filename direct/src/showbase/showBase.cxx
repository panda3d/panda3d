// Filename: showBase.cxx
// Created by:  shochet (02Feb00)
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

#include "showBase.h"

#include <throw_event.h>
#include <camera.h>
#include <renderRelation.h>
#include <namedNode.h>
#include <renderModeTransition.h>
#include <textureTransition.h>
#include <textureTransition.h>
#include <interactiveGraphicsPipe.h>
#include <graphicsWindow.h>
#include <chancfg.h>
#include <cullFaceTransition.h>
#include <dftraverser.h>
#include <renderBuffer.h>
#include <clockObject.h>
#include <animControl.h>
#include <nodeRelation.h>
#include <dataGraphTraversal.h>
#include <depthTestTransition.h>
#include <depthWriteTransition.h>
#include <lightTransition.h>
#include <materialTransition.h>
#include <camera.h>
#include <orthographicLens.h>
#include <appTraverser.h>
#include <get_config_path.h>
#include <allTransitionsWrapper.h>
#include <dataGraphTraversal.h>

ConfigureDef(config_showbase);
ConfigureFn(config_showbase) {
}

DSearchPath &
get_particle_path() {
  static DSearchPath *particle_path = NULL;
  return get_config_path("particle-path", particle_path);
}

// Default channel config
std::string chan_config = "single";
std::string window_title = "Panda3D";

void render_frame(GraphicsPipe *pipe) {
  int num_windows = pipe->get_num_windows();
  for (int w = 0; w < num_windows; w++) {
    GraphicsWindow *win = pipe->get_window(w);
    win->get_gsg()->render_frame();
  }
  // clock tick moved to igloop in ShowBase.py because
  // clock must tick while app is iconified and draw
  // callback is not being called by panda gsg

  //  ClockObject::get_global_clock()->tick();
  throw_event("NewFrame");
}

class WindowCallback : public GraphicsWindow::Callback {
public:
  WindowCallback(GraphicsPipe *pipe, Node *render_top) :
    _pipe(pipe),
    _render_top(render_top),
    _app_traverser(RenderRelation::get_class_type()) { }
  virtual ~WindowCallback() { }

  virtual void draw(bool) {
    _app_traverser.traverse(_render_top);
    render_frame(_pipe);
  }

  PT(GraphicsPipe) _pipe;
  PT(Node) _render_top;
  AppTraverser _app_traverser;
};


PT(GraphicsPipe) make_graphics_pipe() {
  PT(GraphicsPipe) main_pipe;

  // load display modules
  GraphicsPipe::resolve_modules();

  nout << "Known pipe types:" << endl;
  GraphicsPipe::get_factory().write_types(nout, 2);

  // Create a window
  main_pipe = GraphicsPipe::get_factory().
    make_instance(InteractiveGraphicsPipe::get_class_type());

  if (main_pipe == (GraphicsPipe*)0L) {
    nout << "No interactive pipe is available!  Check your Configrc!\n";
    return NULL;
  }

  nout << "Opened a '" << main_pipe->get_type().get_name()
       << "' interactive graphics pipe." << endl;
  return main_pipe;
}

ChanConfig make_graphics_window(GraphicsPipe *pipe, NodeRelation *render_arc) {
  PT(GraphicsWindow) main_win;
  ChanCfgOverrides override;

  // Turn on backface culling and depth buffer
  CullFaceTransition *cfa = new CullFaceTransition(CullFaceProperty::M_cull_clockwise);
  render_arc->set_transition(cfa);
  DepthTestTransition *dta = new DepthTestTransition;
  render_arc->set_transition(dta);
  DepthWriteTransition *dwa = new DepthWriteTransition;
  render_arc->set_transition(dwa);

  // Now use ChanConfig to create the window.
  Node *render_top = render_arc->get_parent();

  override.setField(ChanCfgOverrides::Mask,
                    ((unsigned int)(W_DOUBLE|W_DEPTH|W_MULTISAMPLE)));

  std::string title = config_showbase.GetString("window-title", window_title);
  override.setField(ChanCfgOverrides::Title, title);

  std::string conf = config_showbase.GetString("chan-config", chan_config);
  ChanConfig chan_config(pipe, conf, render_top, override);
  main_win = chan_config.get_win();
  assert(main_win != (GraphicsWindow*)0L);

  WindowCallback *wcb = new WindowCallback(pipe, render_top);

  // Set draw callback.  Currently there is no reason to use the idle callback.
  main_win->set_draw_callback(wcb);

  return chan_config;
}

// Create a scene graph, associated with the indicated window, that
// can contain 2-d geometry and will be rendered on top of the
// existing 3-d window.  Returns the top node of the scene graph.
NodePath
setup_panda_2d(GraphicsWindow *win, const string &graph_name) {
  PT(Node) render2d_top;

  render2d_top = new NamedNode(graph_name + "_top");
  Node *render2d = new NamedNode(graph_name);
  RenderRelation *render2d_arc = new RenderRelation(render2d_top, render2d);

  // Set up some overrides to turn off certain properties which we
  // probably won't need for 2-d objects.

  // It's particularly important to turn off the depth test, since
  // we'll be keeping the same depth buffer already filled by the
  // previously-drawn 3-d scene--we don't want to pay for a clear
  // operation, but we also don't want to collide with that depth
  // buffer.
  render2d_arc->set_transition(new DepthTestTransition(DepthTestProperty::M_none), 1);
  render2d_arc->set_transition(new DepthWriteTransition(DepthWriteTransition::off()), 1);
  render2d_arc->set_transition(new LightTransition(LightTransition::all_off()), 1);
  render2d_arc->set_transition(new MaterialTransition(MaterialTransition::off()), 1);
  render2d_arc->set_transition(new CullFaceTransition(CullFaceProperty::M_cull_none), 1);

  // Create a 2-d camera.
  Camera *cam2d = new Camera("cam2d");
  new RenderRelation(render2d, cam2d);

  PT(Lens) lens = new OrthographicLens;
  lens->set_film_size(2.0);
  lens->set_near_far(-1000.0f, 1000.0f);
  cam2d->set_lens(lens);

  add_render_layer(win, render2d_top, cam2d);

  NodePath render2d_np = NodePath();
  render2d_np.extend_by(render2d_arc);
  return render2d_np;
}

// Create an auxiliary scene graph starting at the indicated node,
// layered on top of the previously-created layers.
void
add_render_layer(GraphicsWindow *win, Node *render_top, Camera *camera) {
  GraphicsChannel *chan = win->get_channel(0);
  nassertv(chan != (GraphicsChannel *)NULL);

  GraphicsLayer *layer = chan->make_layer();
  nassertv(layer != (GraphicsLayer *)NULL);

  DisplayRegion *dr = layer->make_display_region();
  nassertv(dr != (DisplayRegion *)NULL);
  camera->set_scene(render_top);
  dr->set_camera(camera);
}


bool
toggle_wireframe(NodeRelation *render_arc) {
  static bool wireframe_mode = false;

  wireframe_mode = !wireframe_mode;
  if (!wireframe_mode) {
    // Set the normal, filled mode on the render arc.
    RenderModeTransition *rma = new RenderModeTransition(RenderModeProperty::M_filled);
    CullFaceTransition *cfa = new CullFaceTransition(CullFaceProperty::M_cull_clockwise);
    render_arc->set_transition(rma);
    render_arc->set_transition(cfa);

  } else {
    // Set the initial state up for wireframe mode.
    RenderModeTransition *rma = new RenderModeTransition(RenderModeProperty::M_wireframe);
    CullFaceTransition *cfa = new CullFaceTransition(CullFaceProperty::M_cull_none);
    render_arc->set_transition(rma);
    render_arc->set_transition(cfa);
  }
  return wireframe_mode;
}


bool
toggle_backface(NodeRelation *render_arc) {
  static bool backface_mode = false;

  // Toggle the state variable
  backface_mode = !backface_mode;

  if (backface_mode) {
    // Turn backface culling off
    CullFaceTransition *cfa = new CullFaceTransition(CullFaceProperty::M_cull_none);
    render_arc->set_transition(cfa);
  } else {
    // Turn backface culling on
    CullFaceTransition *cfa = new CullFaceTransition(CullFaceProperty::M_cull_clockwise);
    render_arc->set_transition(cfa);
  }
  return backface_mode;
}


bool toggle_texture(NodeRelation *render_arc) {
  static bool textures_enabled = true;

  textures_enabled = !textures_enabled;
  if (textures_enabled) {
    // Remove the override from the initial state.
    render_arc->clear_transition(TextureTransition::get_class_type());
  } else {
    // Set an override on the initial state to disable texturing.
    TextureTransition *ta = new TextureTransition;
    ta->set_priority(100);
    render_arc->set_transition(ta);
  }
  return textures_enabled;
}

void take_snapshot(GraphicsWindow *win, const string &name) {
  GraphicsStateGuardian* gsg = win->get_gsg();
  const RenderBuffer& rb = gsg->get_render_buffer(RenderBuffer::T_front);

  //  CPT(DisplayRegion) dr = gsg->get_current_display_region();
  CPT(DisplayRegion) dr = win->get_channel(0)->get_layer(0)->get_dr(0);
  nassertv(dr != (DisplayRegion *)NULL);

  int width = dr->get_pixel_width();
  int height = dr->get_pixel_height();

  PixelBuffer p(width, height, 3, 1, PixelBuffer::T_unsigned_byte,
                PixelBuffer::F_rgb);

  p.copy(gsg, dr, rb);
  p.write(name);
}

// Returns the configure object for accessing config variables from a
// scripting language.
ConfigShowbase &
get_config_showbase() {
  return config_showbase;
}

// klunky interface since we cant pass array from python->C++ to use verify_window_sizes directly
static unsigned int num_fullscreen_testsizes=0;
#define MAX_FULLSCREEN_TESTS 10
static unsigned int fullscreen_testsizes[MAX_FULLSCREEN_TESTS*2];

void add_fullscreen_testsize(unsigned int xsize,unsigned int ysize) {
    if((xsize==0) && (ysize==0)) {
        num_fullscreen_testsizes=0;
        return;
    }

    // silently fail if maxtests exceeded
    if(num_fullscreen_testsizes<MAX_FULLSCREEN_TESTS) {
        fullscreen_testsizes[num_fullscreen_testsizes*2]=xsize;
        fullscreen_testsizes[num_fullscreen_testsizes*2+1]=ysize;
        num_fullscreen_testsizes++;
    }
}

void runtest_fullscreen_sizes(GraphicsWindow *win) {
    (void) win->verify_window_sizes(num_fullscreen_testsizes,fullscreen_testsizes);
}

bool query_fullscreen_testresult(unsigned int xsize,unsigned int ysize) {
    // stupid linear search that works ok as long as total tests are small
    unsigned int i;
    for(i=0;i<num_fullscreen_testsizes;i++) {
        if((fullscreen_testsizes[i*2]==xsize) &&
           (fullscreen_testsizes[i*2+1]==ysize))
          return true;
    }
    return false;
}

