// Filename: framework.cxx
// Created by:  cary (25Mar99)
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

// We need to include bitMask.h first to avoid a VC++ compiler bug
// related to 2 parameter templates
#include "bitMask.h"

#include "framework.h"
#include "config_framework.h"

#include "pystub.h"
#include "time.h"
// Since framework.cxx includes pystub.h, no program that links with
// framework needs to do so.  No Python code should attempt to link
// with libframework.so.

#include "cullTraverser.h"
#include "appTraverser.h"
#include "directRenderTraverser.h"
#include "mouse.h"
#include "mouseWatcher.h"
#include "buttonThrower.h"
#include "keyboardButton.h"
#include "eventHandler.h"
#include "throw_event.h"
#include "camera.h"
#include "geom.h"
#include "geomprimitives.h"
#include "renderRelation.h"
#include "dataRelation.h"
#include "geomNode.h"
#include "namedNode.h"
#include "pt_NamedNode.h"
#include "colorTransition.h"
#include "renderModeTransition.h"
#include "materialTransition.h"
#include "dataGraphTraversal.h"
#include "trackball.h"
#include "driveInterface.h"
#include "transform2sg.h"
#include "texture.h"
#include "texturePool.h"
#include "textureTransition.h"
#include "interactiveGraphicsPipe.h"
#include "noninteractiveGraphicsPipe.h"
#include "graphicsWindow.h"
#include "plist.h"
#include "lightTransition.h"
#include "materialTransition.h"
#include "animControl.h"
#include "animControlCollection.h"
#include "auto_bind.h"
#include "ambientLight.h"
#include "directionalLight.h"
#include "pointLight.h"
#include "spotlight.h"
#include "dconfig.h"
#include "cullFaceTransition.h"
#include "pruneTransition.h"
#include "dftraverser.h"
#include "renderBuffer.h"
#include "loader.h"
#include "fogTransition.h"
#include "clockObject.h"
#include "compose_matrix.h"
#include "notify.h"
#include "nullTransitionWrapper.h"
#include "nullLevelState.h"
#include "sceneGraphReducer.h"
#include "textNode.h"
#include "depthTestTransition.h"
#include "depthWriteTransition.h"
#include "orthographicLens.h"
#include "transparencyTransition.h"
#include "bamReader.h"
#include "collisionRay.h"
#include "collisionNode.h"
#include "collisionTraverser.h"
#include "collisionHandlerFloor.h"
#include "nodePath.h"
#include "multiplexStream.h"
#include "dSearchPath.h"
#include "camera.h"
#include "perspectiveLens.h"
#include "wdxGraphicsPipe.h"
#include "wdxGraphicsWindow.h"

#ifdef USE_IPC
#include "ipc_file.h"
#include "ipc_mutex.h"
#include "ipc_thread.h"
#endif

Configure(framework);

ConfigureFn(framework) {
}

int NumWindows=1;

AppTraverser *app_traverser;
PT_NamedNode data_root;
PT_NamedNode root;
PT(GeomNode) geomnode;
PT_NamedNode render_top;
PT_NamedNode render;
NodeRelation *render_arc;
PT(MouseAndKeyboard) mak;
PT(MouseWatcher) mouse_watcher;
PT(Trackball) trackball;
PT(DriveInterface) drive_interface;
PT_NamedNode camera_top;
NodeRelation *camera_top_arc;

static Node *current_trackball = NULL;
static Node *alt_trackball = NULL;

Texture* ttex;
//PT(GraphicsPipe) main_pipe;
PT(GraphicsPipe) rib_pipe;
//PT(GraphicsWindow) main_win;
PT(GraphicsWindow) rib_win;
RenderRelation* first_arc;
PT(wdxGraphicsWindow) main_window;
wdxGraphicsWindowGroup *pWinGrp;

PT_NamedNode lights;

PT(AmbientLight) light;
PT(DirectionalLight) dlight;
bool have_dlight = false;
PT(PointLight) plight;
PT(Spotlight) slight;

PT(Material) material;

PT(Fog) fog;

Loader loader;

EventHandler event_handler(EventQueue::get_global_event_queue());

std::string chan_config = "single";

static double start_time = 0.0;
static int start_frame_count = 0;

void (*extra_display_func)() = NULL;
void (*define_keys)(EventHandler&) = NULL;
void (*extra_overrides_func)(ChanCfgOverrides&, std::string&) = NULL;
void (*first_init)() = NULL;
void (*additional_idle)() = NULL;

#ifdef USE_IPC
static bool forked_draw = framework.GetBool("fork-draw", false);
static mutex run_render;
static bool render_running = true;
static bool quit_draw = false;
static thread* draw_thread;
#endif

static CollisionTraverser *col_trans = NULL;
static CollisionHandlerFloor *col_handler = NULL;
static CollisionNode *ray_node = NULL;

class GeomNorms : public GeomLine
{
public:
  GeomNorms(void) : GeomLine() {}
  virtual Geom *explode() const {
    return new GeomNorms(*this);
  }

  static TypeHandle get_class_type(void) {
    return _type_handle;
  }
  static void init_type(void) {
    GeomLine::init_type();
    register_type(_type_handle, "GeomNorms",
                  GeomLine::get_class_type());
  }
  virtual TypeHandle get_type(void) const {
    return get_class_type();
  }
private:
  static TypeHandle _type_handle;
};

TypeHandle GeomNorms::_type_handle;

// Since the Normal*Traversers don't care about state, we don't need
// to accumulate the RenderTransitions, so it will template on
// NullTransition.
class NormalAddTraverser :
  public TraverserVisitor<NullTransitionWrapper, NullLevelState> {
public:
  NormalAddTraverser(GraphicsStateGuardian *gsg) : _gsg(gsg) {}
  bool reached_node(Node*, NullTransitionWrapper&, NullLevelState&);

  // No need to declare a forward_arc() function that simply returns
  // true; this is the default behavior.

public:
  GraphicsStateGuardian *_gsg;
};

bool NormalAddTraverser::
reached_node(Node *node, NullTransitionWrapper &, NullLevelState &) {
  if (node->is_of_type(GeomNode::get_class_type())) {
    GeomNorms *gn = new GeomNorms;
    GeomNode *geom = DCAST(GeomNode, node);
    int vert_count = 0;
    int i;
    for (i = 0; i < geom->get_num_geoms(); i++) {
      dDrawable *d = geom->get_geom(i);
      if (d->is_of_type(Geom::get_class_type())) {
        Geom *g = DCAST(Geom, d);
        for (int j=0; j<g->get_num_prims(); ++j)
          vert_count += g->get_length(j);
      }
    }
    if (vert_count > 0) {
      PTA_Vertexf verts=PTA_Vertexf::empty_array(2 * vert_count);
      for (i = 0; i < geom->get_num_geoms(); i++) {
        dDrawable *d = geom->get_geom(i);
        if (d->is_of_type(Geom::get_class_type())) {
          PTA_Vertexf lverts;
          PTA_ushort iverts;
          Geom *g = DCAST(Geom, d);
          g->get_coords(lverts, iverts);
          int vert_idx = 0;
          if (g->get_binding(G_NORMAL) == G_OFF) {
            for (int j=0; j<g->get_num_prims(); ++j) {
              for (int k=0; k<g->get_length(j); ++k, ++vert_idx) {
                verts[2 * vert_idx] = lverts[vert_idx];
                verts[(2 * vert_idx) + 1] = lverts[vert_idx];
              }
            }
          } else {
            PTA_Normalf lnorms;
            PTA_ushort inorms;
            GeomBindType nbond;
            g->get_normals(lnorms, nbond, inorms);
            for (int j=0; j<g->get_num_prims(); ++j) {
              for (int k=0; k<g->get_length(j); ++k, ++vert_idx) {
                verts[2 * vert_idx] = lverts[vert_idx];
                verts[(2 * vert_idx) + 1] = lverts[vert_idx]
                  + lnorms[vert_idx];
              }
            }
          }
        }
      }
      gn->set_num_prims(vert_count);
      gn->set_coords(verts);
    }
    geom->add_geom(gn);
  }
  return true;
}

class NormalDelTraverser :
  public TraverserVisitor<NullTransitionWrapper, NullLevelState> {
public:
  NormalDelTraverser(GraphicsStateGuardian *gsg) : _gsg(gsg) {}
  bool reached_node(Node*, NullTransitionWrapper&, NullLevelState&);
public:
  GraphicsStateGuardian *_gsg;
};

bool NormalDelTraverser::
reached_node(Node *node, NullTransitionWrapper &, NullLevelState &) {
  if (node->is_of_type(GeomNode::get_class_type())) {
    GeomNode *geom = DCAST(GeomNode, node);
    int i, j;
    do {
      for (i = 0, j = -1;
           i < geom->get_num_geoms();
           ++i) {
        if (geom->get_geom(i)->is_of_type(GeomNorms::get_class_type())) {
          j = i;
        }
      }
      if (j != -1) {
        geom->remove_geom(j);
      }
    } while (j != -1);
  }
  return true;
}

//hacks.  the callback architecture should be changed to make this more straightfwd
static wdxGraphicsWindow *g_pCurRenderWin=NULL;
static bool g_bDoAppTraversal=false;
void render_frame(void){ //GraphicsPipe *pipe) {

    // I think we want to traverse only once/frame, so use flag
  if(g_bDoAppTraversal)
      app_traverser->traverse(render_top); 
  g_bDoAppTraversal=false;

/*  int num_windows = pipe->get_num_windows();
  for (int w = 0; w < num_windows; w++) {        ?????
    GraphicsWindow *win = pipe->get_window(w);

  }
*/  

  g_pCurRenderWin->get_gsg()->render_frame();
}

// to be used with new display callback system
class DisplayCallback : public GraphicsWindow::Callback {
  public:
    virtual void draw(bool) {
      render_frame(/*main_pipe*/);
      if (extra_display_func != NULL)
        extra_display_func();
    }
};

// to be used with old GLUT callback system
void display_func( void ) {
  render_frame(/*main_pipe*/);
  if (extra_display_func != NULL)
    extra_display_func();
}

void set_lighting(bool enabled) {
  if (enabled) {
    // Enable the lights on the initial state.
    PT(LightTransition) la = new LightTransition;
    la->set_on(light.p());

    if (have_dlight) {
      la->set_on(dlight.p());
    }
    render_arc->set_transition(la);

  } else {
    // Remove the lights from the initial state.
    render_arc->clear_transition(LightTransition::get_class_type());
  }
}

// to be used with new display callback system
class IdleCallback : public GraphicsWindow::Callback {
  public:
    virtual void idle(void) {
      // Initiate the data traversal, to send device data down its
      // respective pipelines.
      traverse_data_graph(data_root);

      // Perform the collision traversal, if we have a collision
      // traverser standing by.
      if (col_trans != (CollisionTraverser *)NULL) {
        col_trans->traverse(render_top);
      }

      // Throw any events generated recently.
      event_handler.process_events();

      if (additional_idle != NULL) {
        (*additional_idle)();
      }
    }
};

// to be used with old GLUT callback system
void idle_func( void )
{
    // Initiate the data traversal, to send device data down its
    // respective pipelines.
    traverse_data_graph(data_root);

    // Throw any events generated recently.
    event_handler.process_events();
}

void unpause_draw(void);

void event_esc(CPT_Event ev) {
#ifdef USE_IPC
  if (forked_draw) {
    quit_draw = true;
    unpause_draw();
    mutex_lock m(run_render);
  }
#endif

  double now = ClockObject::get_global_clock()->get_frame_time();
  double delta = now - start_time;

  int frame_count = ClockObject::get_global_clock()->get_frame_count();
  int num_frames = frame_count - start_frame_count;
  if (num_frames > 0) {
    nout << endl << num_frames << " frames in " << delta << " seconds" << endl;
    double x = ((double)num_frames) / delta;
    nout << x << " fps average (" << 1000.0 / x << "ms)" << endl;
  }

  // The Escape key was pressed.  Exit the application.

  rib_pipe = NULL;
  rib_win = NULL;

#ifdef DO_PSTATS
  if (PStatClient::is_connected()) {
    framework_cat.info() << "Disconnecting from stats host" << endl;
    PStatClient::disconnect();
  }
#endif

   if(ev->get_name()=="q") {
     // if app ever exits using exit() without close_window, hopefully this will work
     framework_cat.debug() << "Testing unsafe, no close_window(), direct exit() of framework\n";
   } else {
     delete pWinGrp;  // calls close_window
     pWinGrp=NULL;
     framework_cat.debug() << "Exiting framework\n";
   }

//   main_win = NULL;
//   main_pipe = NULL;

   exit(0);
}

void event_f(CPT_Event) {
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

void event_S(CPT_Event) {
#ifdef DO_PSTATS
  framework_cat.info() << "Connecting to stats host" << endl;
  PStatClient::connect();
#else
  framework_cat.error() << "Stats host not supported." << endl;
#endif
}

void event_A(CPT_Event) {
#ifdef DO_PSTATS
  if (PStatClient::is_connected()) {
    framework_cat.info() << "Disconnecting from stats host" << endl;
    PStatClient::disconnect();
  } else {
    framework_cat.error() << "Stats host is already disconnected." << endl;
  }
#else
  framework_cat.error() << "Stats host not supported." << endl;
#endif
}

void event_t(CPT_Event) {
  // The "t" key was pressed.  Toggle the showing of textures.
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
}

void event_l(CPT_Event) {
  // The "l" key was pressed.  Toggle lighting.
  static bool lighting_enabled = false;

  lighting_enabled = !lighting_enabled;
  set_lighting(lighting_enabled);
}

void event_w(CPT_Event) {
  // The "w" key was pressed.  Toggle wireframe mode.
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
}

void event_b(CPT_Event) {
  // The 'b' key was pressed.  Toggle backface culling.
  static bool backface_mode = false;

  backface_mode = !backface_mode;
  if (backface_mode) {
    material->set_twoside(true);
    CullFaceTransition *cfa = new CullFaceTransition(CullFaceProperty::M_cull_none);
    render_arc->set_transition(cfa);
  } else {
    material->set_twoside(false);
    CullFaceTransition *cfa = new CullFaceTransition(CullFaceProperty::M_cull_clockwise);
    render_arc->set_transition(cfa);
  }
}

void event_R(CPT_Event) {
  /* The "R" key was pressed.  Dump a RIB file.
  if (rib_win == (GraphicsWindow*)0L)
    return;
  nout << "Writing RIB frame " << rib_win->get_frame_number() << "\n";
  render_frame(rib_pipe);
  */
}

void event_grave(CPT_Event) {
  GraphicsStateGuardian *gsg = main_window->get_gsg();
  const RenderBuffer &rb = gsg->get_render_buffer(RenderBuffer::T_front);

  // We simply grab the first DisplayRegion on the first Layer on the
  // window's main channel.
  GraphicsChannel *channel = main_window->get_channel(0);
  nassertv(channel != (GraphicsChannel *)NULL);

  if (channel->get_num_layers() == 0) {
    nout << "Channel has no layers!\n";
    return;
  }
  GraphicsLayer *layer = channel->get_layer(0);
  nassertv(layer != (GraphicsLayer *)NULL);

  if (layer->get_num_drs() == 0) {
    nout << "Layer has no display regions!\n";
    return;
  }
  DisplayRegion *dr = layer->get_dr(0);
  nassertv(dr != (DisplayRegion *)NULL);

  int width = dr->get_pixel_width();
  int height = dr->get_pixel_height();

  PixelBuffer p(width, height, 3, 1, PixelBuffer::T_unsigned_byte,
                PixelBuffer::F_rgb);

  nout << "Capturing frame.\n";

  p.copy(gsg, dr, rb);
  ostringstream s;
  s << "frame" << ClockObject::get_global_clock()->get_frame_count() << ".pnm";
  Filename filename = s.str();
  p.write(filename);

  cerr << "Wrote " << filename << "\n";
}

void event_n(CPT_Event) {
  static bool normals_on = false;

  normals_on = !normals_on;

  // do i do this for every window??  bugbug
  for(unsigned int j=0;j<pWinGrp->_windows.size();j++) 
      if (normals_on) {
          NormalAddTraverser trav(pWinGrp->_windows[j]->get_gsg());
          df_traverse(render, trav, NullTransitionWrapper(), NullLevelState(),
                RenderRelation::get_class_type());
      } else {
          NormalDelTraverser trav(pWinGrp->_windows[j]->get_gsg());
          df_traverse(render, trav, NullTransitionWrapper(), NullLevelState(),
              RenderRelation::get_class_type());
      }

}

void event_C(CPT_Event) {
  static bool showing_collision_solids = false;

  showing_collision_solids = !showing_collision_solids;
  if (showing_collision_solids) {
    // So we'll break down and use the NodePath interface in
    // framework.  We haven't used it here before, but it's such a
    // splendid interface; why not use it?
    NodePath render_path(render);
    render_path.show_collision_solids();

  } else {
    NodePath render_path(render);
    render_path.hide_collision_solids();
  }
}

void event_N(CPT_Event) {
  nout << "Reducing scene graph.\n";
  SceneGraphReducer gr(RenderRelation::get_class_type());
  gr.apply_transitions(first_arc);
  int num_reduced = gr.flatten(root, true);
  nout << "Removed " << num_reduced << " arcs.\n";
}

// switch_trackball() is a local function to fiddle with the dgraph
// arcs to make a different trackball be in control of the mouse.
static void
switch_trackball(Node *trackball) {
  if (current_trackball != NULL) {
    remove_child(mouse_watcher, current_trackball,
                 DataRelation::get_class_type());
  }
  current_trackball = trackball;
  if (current_trackball != NULL) {
    new DataRelation(mouse_watcher, current_trackball);
  }
}

// set_alt_trackball() should be called by user code to change the
// alternate trackball that is in effect when the user presses "c".
void
set_alt_trackball(Node *tb) {
  if (tb == NULL) {
    switch_trackball(trackball);
  } else {
    alt_trackball = tb;
    switch_trackball(alt_trackball);
  }
}


static void
start_drive() {
  // Extract the current position from the trackball.
  LMatrix4f mat = trackball->get_trans_mat();
  LPoint3f scale, hpr, xyz;
  decompose_matrix(mat, scale, hpr, xyz);
  if (hpr[2] > 90) {
    hpr[0] += 180.0;
  }
  hpr[1] = 0.0;
  hpr[2] = 0.0;

  drive_interface->set_pos(xyz);
  drive_interface->set_hpr(hpr);

  // Make sure the ray-downcaster is set, so we maintain a constant
  // height above the ground.
  if (col_trans == (CollisionTraverser *)NULL) {
    ray_node = new CollisionNode("ray");
    ray_node->set_into_collide_mask(0);
    ray_node->set_from_collide_mask(drive_mask);

    NodeRelation *arc = new RenderRelation(camera_top, ray_node);

    ray_node->add_solid(new CollisionRay(LPoint3f(0.0, 0.0, 0.0),
                                         LVector3f::down()));
    arc->set_transition(new PruneTransition);

    col_trans = new CollisionTraverser;
    col_handler = new CollisionHandlerFloor;
    col_handler->set_offset(drive_height);
    col_trans->add_collider(ray_node, col_handler);
    col_handler->add_collider(ray_node, drive_interface);
  }
}

static void
stop_drive() {
  // Extract the current position from the drive interface and
  // restore it to the trackball.
  LPoint3f xyz = drive_interface->get_pos();
  LPoint3f hpr = drive_interface->get_hpr();
  LPoint3f scale(1.0, 1.0, 1.0);
  LMatrix4f mat;
  compose_matrix(mat, scale, hpr, xyz);
  trackball->set_mat(invert(mat));
  trackball->reset_origin_here();
}

void event_c(CPT_Event) {
  // "c" key pressed: change to alternate controls.

  if (current_trackball == trackball) {
    if (alt_trackball != NULL) {
      switch_trackball(alt_trackball);
    }
  } else {
    if (current_trackball == drive_interface) {
      stop_drive();
    }
    switch_trackball(trackball);
  }
}

void event_D(CPT_Event) {
  // "D" key pressed: toggle drive controls.
  if (current_trackball == drive_interface) {
    stop_drive();
    switch_trackball(trackball);

  } else {
    start_drive();
    set_alt_trackball(drive_interface);
  }
}

//  sample code to verify and pick a new fullscreen size dynamically
#define NUMWINDOWSIZES 5
static int cur_winsize_idx=0;
static unsigned int window_sizearr[NUMWINDOWSIZES*2] = 
      {640,480, 1024,768, 800,600, 454,656, 1280,1024};

void event_3(CPT_Event) {
  do {
    cur_winsize_idx++;
    cur_winsize_idx %= NUMWINDOWSIZES;
    // skip over the ones marked as bad (0)
  } while(window_sizearr[cur_winsize_idx*2]==0);

  for(unsigned int j=0;j<pWinGrp->_windows.size();j++) 
      pWinGrp->_windows[j]->resize(window_sizearr[cur_winsize_idx*2],window_sizearr[cur_winsize_idx*2+1]);
}

void event_p(CPT_Event) {
  // "p" key pressed: print pos, hpr
  LPoint3f xyz;
  LPoint3f hpr;

  if (current_trackball == trackball) {
          xyz = trackball->get_pos();
          hpr = trackball->get_hpr();
  } else if (current_trackball == drive_interface) {
                xyz = drive_interface->get_pos();
                hpr = drive_interface->get_hpr();
  }

  printf("current pos, hpr:  %f %f %f    %f %f %f\n",xyz[0],xyz[1],xyz[2],hpr[0],hpr[1],hpr[2]);
}

void event_P(CPT_Event) {
  // "P" key pressed: set pos, hpr
  LPoint3f xyz;
  LPoint3f hpr;

  printf("input new pos, hpr in fmt:   f f f  f f f\n");
  scanf("%f %f %f %f %f %f",&xyz[0],&xyz[1],&xyz[2],&hpr[0],&hpr[1],&hpr[2]);

  if (current_trackball == trackball) {
          trackball->set_pos(xyz);
          trackball->set_hpr(hpr);
  } else if (current_trackball == drive_interface) {
                drive_interface->set_pos(xyz);
                drive_interface->set_hpr(hpr);

  }
}

void event_g(CPT_Event) {
  // "g" key pressed: toggle fog.
  static bool fog_mode = false;

  fog_mode = !fog_mode;
  if (fog_mode) {
    FogTransition *fa = new FogTransition(fog);
    render_arc->set_transition(fa);
  } else {
    FogTransition *fa = new FogTransition;
    render_arc->set_transition(fa);
  }
}

#ifdef USE_IPC
/*
void pause_draw(void) {
  if (!render_running)
    return;
  run_render.lock();
  render_running = false;
  framework_cat.info() << "draw thread paused" << endl;
}

void unpause_draw(void) {
  if (render_running)
    return;
  run_render.unlock();
  render_running = true;
  framework_cat.info() << "draw thread continuing" << endl;
}

void draw_loop(void*) {
  for (;!quit_draw;) {
    mutex_lock m(run_render);
    main_win->update();
  }
  framework_cat.info() << "draw thread exiting" << endl;
}

void event_x(CPT_Event) {
  // "x" key pressed: pause/unpause draw
  if (!forked_draw)
    return;
  if (render_running)
    pause_draw();
  else
    unpause_draw();
}
*/
#endif

#define RANDFRAC (rand()/(float)(RAND_MAX))

typedef struct {
        // for rot moving
        float xcenter,ycenter;
        float xoffset,yoffset;
        float ang1,ang1_vel;
        float ang2,ang2_vel;

        float radius;

        // for moving
        float xstart,ystart;
        float xend,yend;
        float xdel,ydel,timedel;
        double starttime,endtime;
    double vel;
        LMatrix4f rotmat;
} gridded_file_info;

typedef enum {None,Rotation,LinearMotion} GriddedMotionType;

#define GRIDCELLSIZE 5.0
static int gridwidth;  // cells/side

#define MIN_WANDERAREA_DIMENSION 120.0

static float grid_pos_offset;  // origin of grid
static float wander_area_pos_offset;

// making these fns to get around ridiculous VC++ matrix inlining bugs at Opt2
static void move_gridded_stuff(GriddedMotionType gridmotiontype,gridded_file_info *InfoArr, RenderRelation **pRRptrArr, int size) {

  double now = ClockObject::get_global_clock()->get_frame_time();

  LMatrix4f tmat1,tmat2,xfm_mat;

  for(int i = 0; i < size; i++) {
  double time_delta = (now-InfoArr[i].starttime);
  #define DO_FP_MODULUS(VAL,MAXVAL)  \
    {if(VAL > MAXVAL) {int idivresult = (int)(VAL / (float)MAXVAL);  VAL=VAL-idivresult*MAXVAL;} else  \
    if(VAL < -MAXVAL) {int idivresult = (int)(VAL / (float)MAXVAL);  VAL=VAL+idivresult*MAXVAL;}}
  
  // probably should use panda lerps for this stuff, but I dont understand how

    if(gridmotiontype==Rotation) {

                InfoArr[i].ang1=time_delta*InfoArr[i].ang1_vel;
                DO_FP_MODULUS(InfoArr[i].ang1,360.0);
                InfoArr[i].ang2=time_delta*InfoArr[i].ang2_vel;
                DO_FP_MODULUS(InfoArr[i].ang2,360.0);

                // xforms happen left to right
                LVector2f new_center = LVector2f(InfoArr[i].radius,0.0) *
                  LMatrix3f::rotate_mat(InfoArr[i].ang1);

                LVector3f translate_vec(InfoArr[i].xcenter+new_center._v.v._0,
                                                                InfoArr[i].ycenter+new_center._v.v._1,
                                                                0.0);

                const LVector3f rotation_axis(0.0, 0.0, 1.0);

                tmat1 = LMatrix4f::rotate_mat_normaxis(InfoArr[i].ang2,rotation_axis);
                tmat2 = LMatrix4f::translate_mat(translate_vec);
                xfm_mat = tmat1 * tmat2;
        } else {

                  float xpos,ypos;

                  if(now>InfoArr[i].endtime) {
                          InfoArr[i].starttime = now;

                          xpos = InfoArr[i].xstart = InfoArr[i].xend;
                          ypos = InfoArr[i].ystart = InfoArr[i].yend;

                          InfoArr[i].xend = RANDFRAC*fabs(2.0*wander_area_pos_offset) + wander_area_pos_offset;
                          InfoArr[i].yend = RANDFRAC*fabs(2.0*wander_area_pos_offset) + wander_area_pos_offset;

                          float xdel = InfoArr[i].xdel = InfoArr[i].xend-InfoArr[i].xstart;
                          float ydel = InfoArr[i].ydel = InfoArr[i].yend-InfoArr[i].ystart;

                          InfoArr[i].endtime = now + csqrt(xdel*xdel+ydel*ydel)/InfoArr[i].vel;
                          InfoArr[i].timedel = InfoArr[i].endtime - InfoArr[i].starttime;

                          const LVector3f rotate_axis(0.0, 0.0, 1.0);

                          float ang = rad_2_deg(atan2(-xdel,ydel));

              InfoArr[i].rotmat= LMatrix4f::rotate_mat_normaxis(ang,rotate_axis);
                  } else {
                          float timefrac= time_delta/InfoArr[i].timedel;

                          xpos = InfoArr[i].xdel*timefrac+InfoArr[i].xstart;
                          ypos = InfoArr[i].ydel*timefrac+InfoArr[i].ystart;
                  }

                  LVector3f translate_vec(xpos, ypos, 0.0);
                  LMatrix4f tmat2 = LMatrix4f::translate_mat(translate_vec);

                  xfm_mat = InfoArr[i].rotmat * tmat2;
        }
    pRRptrArr[i]->set_transition(new TransformTransition(xfm_mat));
  }
}

int framework_main(int argc, char *argv[]) {
  pystub();

  /*
  // The first thing we should do is to set up a multiplexing Notify.
  MultiplexStream *mstream = new MultiplexStream;
  Notify::ptr()->set_ostream_ptr(mstream, true);
  mstream->add_standard_output();
  mstream->add_system_debug();

  string framework_notify_output = framework.GetString("framework-notify-output", "");
  if (!framework_notify_output.empty()) {
    if (!mstream->add_file(framework_notify_output)) {
      framework_cat.error()
        << "Unable to open " << framework_notify_output << " for output.\n";
    } else {
      framework_cat.info()
        << "Sending Notify output to " << framework_notify_output << "\n";
    }
  }
  */

  GeomNorms::init_type();

#ifndef DEBUG
  // This just makes sure that no one changed the value of a
  // _type_handle member after the type was registered.  It shouldn't
  // ever happen.  If it did, most likely two classes are sharing the
  // same _type_handle variable for some reason.
  TypeRegistry::reregister_types();
#endif

  app_traverser = new AppTraverser(RenderRelation::get_class_type());

  // Allow the specification of multiple files on the command
  // line.  This is handy, for instance, to load up both a character
  // and its animation file.

  typedef pvector<Filename> Files;
  Files files;
  Files gridded_files;

  if (first_init != NULL)
    first_init();

  Files *pFileCollection = &files;

  int gridrepeats=1;
  GriddedMotionType gridmotiontype = None;

  //  bool bRotateGriddedObjs = false;
  //  bool bMoveGriddedObjs = false;

  for (int a = 1; a < argc; a++) {
    if ((argv[a] != (char*)0L) && ((argv[a])[0] != '-') &&
        ((argv[a])[0] != '+') && ((argv[a])[0] != '#'))
      pFileCollection->push_back(Filename::from_os_specific(argv[a]));
        else switch((argv[a])[1]) {
                         case 'r':
                                 gridmotiontype = Rotation;
                                 break;

                         case 'm':
                                 gridmotiontype = LinearMotion;
                                 break;

                          case 'g': {
                                 pFileCollection = &gridded_files;

                                 char *pStr=(argv[a])+2;
                                 if (*pStr != '\0') {
                                         gridrepeats=atoi(pStr);
                                         if(gridrepeats<1)
                                                 gridrepeats=1;

                                 }
                                 break;
                          }
        }
  }

#if 0
  // load display modules
  GraphicsPipe::resolve_modules();

  framework_cat.info() << "Known pipe types:" << endl;
  GraphicsPipe::get_factory().write_types(framework_cat.info(false), 2);

  // Create a window
  main_pipe = GraphicsPipe::get_factory().
    make_instance(InteractiveGraphicsPipe::get_class_type());

  if (main_pipe == (GraphicsPipe*)0L) {
    framework_cat.error()
      << "No interactive pipe is available!  Check your Configrc!\n";
    exit(1);
  }

  framework_cat.info()
    << "Opened a '" << main_pipe->get_type().get_name() << "' interactive graphics pipe." << endl;

  rib_pipe = NULL;
#endif
#if 0
  rib_pipe = GraphicsPipe::get_factory().
    make_instance(NoninteractiveGraphicsPipe::get_class_type());

  if (rib_pipe == (GraphicsPipe*)0L)
    framework_cat.info()
      << "Did not open a non-interactive graphics pipe, features related"
      << " to that will\nbe disabled." << endl;
  else
    framework_cat.info()
      << "Opened a '" << rib_pipe->get_type().get_name()
      << "' non-interactive graphics pipe." << endl;
#endif

  // Create the render node
  render_top = new NamedNode("render_top");
  render = new NamedNode("render");
  render_arc = new RenderRelation(render_top, render);

  camera_top = new NamedNode("camera_top");
  camera_top_arc = new RenderRelation(render, camera_top);

  // bypass chancfg stuff and do this directly

  int NumWindows=framework.GetInt("num-windows", 1);

  GraphicsWindow::Properties *pWinProps = new GraphicsWindow::Properties[NumWindows];
  memset(pWinProps,0,NumWindows*sizeof(GraphicsWindow::Properties));
  
  pWinProps[0]._title=framework.GetString("win-title", "PandaWin");
  pWinProps[0]._xsize=framework.GetInt("win-width", 640);
  pWinProps[0]._ysize=framework.GetInt("win-height", 480);
  pWinProps[0]._fullscreen=framework.GetBool("fullscreen", true);
  pWinProps[0]._border=!framework.GetBool("no-border", false);
  pWinProps[0]._want_depth_bits=1;
  pWinProps[0]._want_color_bits=1;
  pWinProps[0]._mask = W_RGBA | W_DOUBLE | W_DEPTH;

  int jjj;
  for(jjj=1;jjj<NumWindows;jjj++) {
    memcpy(&pWinProps[jjj],&pWinProps[0],sizeof(GraphicsWindow::Properties));
    pWinProps[0]._xorg=jjj*pWinProps[0]._xsize;  // assumes horiz layout
  }

  for(jjj=0;jjj<NumWindows;jjj++) {
    char numstr[5];
    sprintf(numstr,"(%d)",jjj);
    pWinProps[jjj]._title += std::string(numstr);
  }

  PipeSpecifier pipe_spec;
//  PT(GraphicsPipe) pipe;
  wdxGraphicsPipe *pWDXpipe = new wdxGraphicsPipe(pipe_spec);

  pWinGrp = new wdxGraphicsWindowGroup(pWDXpipe,NumWindows,pWinProps);

  assert(pWinGrp!=NULL);

  //make channels,layers,disply regions
  for(jjj=0;jjj<NumWindows;jjj++) {
      PT(GraphicsChannel) channel = pWinGrp->_windows[jjj]->get_channel(0);
      // Make a layer on the channel to hold our display region.
      PT(GraphicsLayer) layer = channel->make_layer();

      // And create a display region that covers the entire window.
      PT(DisplayRegion) dr = layer->make_display_region();

      // Finally, we need a camera to associate with the display region.
      PT(Camera) camera = new Camera;
      PT(Lens) lens = new PerspectiveLens;
      lens->set_film_size(pWinProps->_xsize,pWinProps->_ysize);
      camera->set_lens(lens);
      dr->set_camera(camera);

      // Window setup is complete.  Now we just need to make a scene graph
      // for the camera to render.
      camera->set_scene(render_top);
      new RenderRelation(camera_top,camera);
  }

  main_window=pWinGrp->_windows[0];

  delete [] pWinProps;  // dont need these anymore
  pWinProps=NULL;

#if 0
  ChanCfgOverrides override;

  // need to find a better way to differentiate unsigned int from regular
  override.setField(ChanCfgOverrides::Mask,
                    ((unsigned int)(W_DOUBLE|W_DEPTH|W_MULTISAMPLE)));
  override.setField(ChanCfgOverrides::Title, "Demo");

  std::string conf = framework.GetString("chan-config", chan_config);
  if (extra_overrides_func != NULL)
    extra_overrides_func(override, conf);

  ChanConfig chanConfig = ChanConfig(main_pipe, conf, render_top, override);
  main_win = chanConfig.get_win();
  assert(main_win != (GraphicsWindow*)0L);
  camera_top = chanConfig.get_group_node(0);

  camera_top->set_name("camera_top");
  for(int group_node_index=1;group_node_index<chanConfig.get_num_groups();
      group_node_index++) {
    new RenderRelation(render, chanConfig.get_group_node(group_node_index));
  }
  RenderRelation* arc1 = new RenderRelation(render, camera_top);
#endif


#if 0
  // is ok if this doesn't work or returns NULL
  if (rib_pipe != (GraphicsPipe*)0L) { 
    ChanConfig chanConfig = ChanConfig(rib_pipe, "single", render_top, override);
    rib_win = chanConfig.get_win();
    NamedNode *rib_cameras = chanConfig.get_group_node(0);
    rib_cameras->set_name("rib_cameras");
    for(int rib_group_node_index=1;
        rib_group_node_index<chanConfig.get_num_groups();
        rib_group_node_index++) {
      new RenderRelation(render, 
                         chanConfig.get_group_node(rib_group_node_index));
    }
    new RenderRelation(render, rib_cameras);
  }
#endif

  // Make a node for the lights to live under.  We put the lights in
  // with the camera_top, so they'll stay locked to our point-of-view.

  lights = new NamedNode("lights");
  new RenderRelation(camera_top, lights);

  light = new AmbientLight( "ambient" );
  dlight = new DirectionalLight( "directional" );
  plight = new PointLight( "point" );
  plight->set_constant_attenuation( 2.0 );
  plight->set_linear_attenuation( 1.0 );
  plight->set_quadratic_attenuation( 0.5 );
  slight = new Spotlight( "spot" );
  new RenderRelation( lights, light );
#if 0
  new RenderRelation( lights, dlight );
  new RenderRelation( lights, plight );
  new RenderRelation( lights, slight );
#endif

  // Turn on culling.
  CullFaceTransition *cfa = new CullFaceTransition(CullFaceProperty::M_cull_clockwise);
  render_arc->set_transition(cfa);

  // Set up a default material.
  material = new Material;
  material->set_ambient( Colorf( 1, 1, 1, 1 ) );
  MaterialTransition *ma = new MaterialTransition(material);
  render_arc->set_transition(ma);

  // Set up a default fog
  fog = new Fog;

  // Create the data graph root.
  data_root = new NamedNode( "data" );

  // Create a mouse and put it in the data graph.
  mak = new MouseAndKeyboard( main_window, 0 );
  new DataRelation(data_root, mak);

  // Create a MouseWatcher underneath the mouse, so we can have some
  // 2-d control effects.
  mouse_watcher = new MouseWatcher("mouse_watcher");
  new DataRelation(mak, mouse_watcher);
  mouse_watcher->set_button_down_pattern("mw-%r-%b");
  mouse_watcher->set_button_up_pattern("mw-%r-%b-up");
  mouse_watcher->set_enter_pattern("mw-in-%r");
  mouse_watcher->set_leave_pattern("mw-out-%r");

  // Create a trackball to handle the mouse input.
  trackball = new Trackball("trackball");
  trackball->set_pos(LVector3f::forward() * 50.0);

  // Also create a drive interface.  The user might switch to this
  // later.
  drive_interface = new DriveInterface("drive_interface");

  new DataRelation(mouse_watcher, trackball);
  current_trackball = trackball;

  // Connect the trackball output to the camera's transform.
  PT(Transform2SG) tball2cam = new Transform2SG("tball2cam");
//  tball2cam->set_arc(arc1);
  tball2cam->set_arc(camera_top_arc);
  new DataRelation(trackball, tball2cam);

  PT(Transform2SG) drive2cam = new Transform2SG("drive2cam");
//  drive2cam->set_arc(arc1);
  drive2cam->set_arc(camera_top_arc);
  new DataRelation(drive_interface, drive2cam);

  // Create a ButtonThrower to throw events from the keyboard.
  PT(ButtonThrower) et = new ButtonThrower("kb-events");
  ModifierButtons mods;
  mods.add_button(KeyboardButton::shift());
  mods.add_button(KeyboardButton::control());
  mods.add_button(KeyboardButton::alt());
  et->set_modifier_buttons(mods);
  new DataRelation(mouse_watcher, et);

  root = new NamedNode("root");
  first_arc = new RenderRelation(render, root, 100);

  // This will hold the AnimControls for animated characters.
  AnimControlCollection anim_controls;

  ////// for gridded stuff
  PT_Node *pNodeArr=NULL;
  RenderRelation **pRRptrArr=NULL;
  gridded_file_info *InfoArr=NULL;
  int gridded_files_size=0;
  //////////////////

  if (files.empty() && gridded_files.empty() && framework.GetBool("have-omnitriangle", true)) {
    // The user did not specify a model file to load.  Create some
    // default geometry.

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

    geomnode = new GeomNode;
    geomnode->add_geom(geom.p());
    RenderRelation *arc = new RenderRelation(root, geomnode, 10);
    first_arc = arc;

    Texture *tex = TexturePool::load_texture("rock-floor.rgb");
    if (tex != (Texture *)NULL) {
      tex->set_minfilter(Texture::FT_linear);
      tex->set_magfilter(Texture::FT_linear);
      arc->set_transition(new TextureTransition(tex));
    }

  } else {
    // Load up some geometry from one or more files.
    DSearchPath local_path(".");


    Files::const_iterator fi;
    for (fi = files.begin(); fi != files.end(); ++fi) {
      Filename filename = (*fi);

      // First, we always try to resolve a filename from the current
      // directory.  This means a local filename will always be found
      // before the model path is searched.
      filename.resolve_filename(local_path);

      PT_Node node = loader.load_sync(filename);
      
      if (node == (Node *)NULL) {
        framework_cat.error() << "Unable to load file " << filename << "\n";
        continue;
      }
      
      new RenderRelation(root, node);
    }

    if(!gridded_files.empty()) {
      
      typedef RenderRelation *RenderRelationPtr;
      
      gridded_files_size= gridded_files.size();
      pNodeArr = new PT_Node[gridded_files.size()*gridrepeats];
      pRRptrArr = new RenderRelationPtr[gridded_files.size()*gridrepeats];
      InfoArr = new gridded_file_info[gridded_files.size()*gridrepeats];
      
      int j=0;
      
      for (fi = gridded_files.begin(); fi != gridded_files.end(); (++fi),j++) {
        Filename filename = (*fi);
        
        filename.resolve_filename(local_path);
        
        pNodeArr[j] = loader.load_sync(filename);
        
        if (pNodeArr[j] == (Node *)NULL) {
          framework_cat.error() << "Unable to load file " << filename << "\n";
          j--;
          gridded_files_size--;
          continue;
        }
      }
      
      gridwidth=1;
      while(gridwidth*gridwidth < gridded_files_size*gridrepeats) {
        gridwidth++;
      }
      
      grid_pos_offset = -gridwidth*GRIDCELLSIZE/2.0;
      wander_area_pos_offset = -max(fabs(grid_pos_offset),MIN_WANDERAREA_DIMENSION/2.0);
      
      float xpos = grid_pos_offset;
      float ypos = grid_pos_offset;
      int filenum=0;
      
      srand( (unsigned)time( NULL ) );
      
      double now = ClockObject::get_global_clock()->get_frame_time();

      for(int passnum=0;passnum<gridrepeats;passnum++) {
        for (j = 0; j < gridded_files_size; j++,filenum++) {

          if(passnum>0) {
                                // cant directly instance characters due to LOD problems,
                                // must copy using copy_subgraph for now

            pNodeArr[filenum] = pNodeArr[j]->copy_subgraph(RenderRelation::get_class_type());
          }

          pRRptrArr[filenum] = new RenderRelation(root, pNodeArr[filenum]);

          LMatrix4f xfm_mat,tmat1,tmat2;

          if(gridmotiontype==Rotation) {

#define MIN_REVOLUTION_ANGVEL 30
#define MAX_REVOLUTION_ANGVEL 60

#define MIN_ROTATION_ANGVEL 30
#define MAX_ROTATION_ANGVEL 600

#define MAX_RADIUS 4.0*GRIDCELLSIZE
#define MIN_RADIUS 0.1*GRIDCELLSIZE

            InfoArr[filenum].starttime = now;

            InfoArr[filenum].xcenter=xpos;
            InfoArr[filenum].ycenter=ypos;
            InfoArr[filenum].ang1=RANDFRAC * 360.0;
            InfoArr[filenum].ang1_vel=((MAX_REVOLUTION_ANGVEL-MIN_REVOLUTION_ANGVEL) * RANDFRAC) + MIN_REVOLUTION_ANGVEL;

            InfoArr[filenum].ang2=RANDFRAC * 360.0;
            InfoArr[filenum].ang2_vel=((MAX_ROTATION_ANGVEL-MIN_ROTATION_ANGVEL) * RANDFRAC) + MIN_ROTATION_ANGVEL;

            InfoArr[filenum].radius = (RANDFRAC * (MAX_RADIUS-MIN_RADIUS)) + MIN_RADIUS;

            if(RANDFRAC>0.5) {
              InfoArr[filenum].ang1_vel=-InfoArr[filenum].ang1_vel;
            }

            if(RANDFRAC>0.5) {
              InfoArr[filenum].ang2_vel=-InfoArr[filenum].ang2_vel;
            }

            // xforms happen left to right
            LVector2f new_center = LVector2f(InfoArr[filenum].radius,0.0) *
              LMatrix3f::rotate_mat(InfoArr[filenum].ang1);

            const LVector3f rotate_axis(0.0, 0.0, 1.0);

            LVector3f translate_vec(xpos+new_center._v.v._0,
                                    ypos+new_center._v.v._1,
                                    0.0);

            LMatrix4f::rotate_mat_normaxis(InfoArr[filenum].ang2,rotate_axis,tmat1);
            tmat2 = LMatrix4f::translate_mat(translate_vec);
            xfm_mat = tmat1 * tmat2;
          } else if(gridmotiontype==LinearMotion) {

#define MIN_VEL 2.0
#define MAX_VEL (fabs(wander_area_pos_offset))

            InfoArr[filenum].vel=((MAX_VEL-MIN_VEL) * RANDFRAC) + MIN_VEL;

            InfoArr[filenum].xstart=xpos;
            InfoArr[filenum].ystart=ypos;

            InfoArr[filenum].xend = RANDFRAC*fabs(2.0*wander_area_pos_offset) + wander_area_pos_offset;
            InfoArr[filenum].yend = RANDFRAC*fabs(2.0*wander_area_pos_offset) + wander_area_pos_offset;

            InfoArr[filenum].starttime = now;

            float xdel = InfoArr[filenum].xdel = InfoArr[filenum].xend-InfoArr[filenum].xstart;
            float ydel = InfoArr[filenum].ydel = InfoArr[filenum].yend-InfoArr[filenum].ystart;

            InfoArr[filenum].endtime = csqrt(xdel*xdel+ydel*ydel)/InfoArr[filenum].vel;

            InfoArr[filenum].timedel = InfoArr[filenum].endtime - InfoArr[filenum].starttime;

            const LVector3f rotate_axis(0.0, 0.0, 1.0);
            float ang = rad_2_deg(atan2(-xdel,ydel));

            LMatrix4f::rotate_mat_normaxis(ang,rotate_axis,InfoArr[filenum].rotmat);

            LVector3f translate_vec(xpos, ypos, 0.0);
            LMatrix4f tmat2 = LMatrix4f::translate_mat(translate_vec);

            xfm_mat = InfoArr[filenum].rotmat * tmat2;
          } else {
            LVector3f translate_vec(xpos, ypos, 0.0);
            xfm_mat = LMatrix4f::translate_mat(translate_vec);
          }

          pRRptrArr[filenum]->set_transition(new TransformTransition(xfm_mat));

          if(((filenum+1) % gridwidth) == 0) {
            xpos= -gridwidth*GRIDCELLSIZE/2.0;
            ypos+=GRIDCELLSIZE;
          } else {
            xpos+=GRIDCELLSIZE;
          }
        }
      }

    }

    // If we happened to load up both a character file and its
    // matching animation file, attempt to bind them together now.
    auto_bind(root, anim_controls, ~0);
    anim_controls.loop_all(true);
  }

  // Now prepare all the textures with the GSG.
  NodePath render_path(render);

  for(jjj=0;jjj<NumWindows;jjj++) {
      wdxGraphicsWindow *pWin=pWinGrp->_windows[jjj];
      render_path.prepare_scene(pWin->get_gsg());

      //  sample code to verify and pick a new fullscreen size dynamically
      if(pWin->verify_window_sizes(NUMWINDOWSIZES,window_sizearr)==0) {
          framework_cat.error() << "None of the potential new fullscreen sizes are valid\n";
          exit(1);
      }

      if (framework.Defined("clear-value")) {
        float cf = framework.GetFloat("clear-value", 0.0f);
        Colorf c(cf, cf, cf, 1.0f);
        pWin->get_gsg()->set_color_clear_value(c);
      }
  }

  // Set up keyboard events.
  event_handler.add_hook("escape", event_esc);
  event_handler.add_hook("q", event_esc);
  event_handler.add_hook("3", event_3);
  event_handler.add_hook("f", event_f);
  event_handler.add_hook("t", event_t);
  event_handler.add_hook("l", event_l);
  event_handler.add_hook("w", event_w);
  event_handler.add_hook("b", event_b);
  event_handler.add_hook("shift-R", event_R);
  event_handler.add_hook("`", event_grave);
  event_handler.add_hook("n", event_n);
  event_handler.add_hook("c", event_c);
  event_handler.add_hook("shift-D", event_D);
  event_handler.add_hook("g", event_g);
  event_handler.add_hook("shift-C", event_C);
  event_handler.add_hook("shift-N", event_N);
  event_handler.add_hook("shift-S", event_S);
  event_handler.add_hook("shift-A", event_A);
  event_handler.add_hook("p", event_p);
  event_handler.add_hook("shift-P", event_P);

#ifdef USE_IPC
  event_handler.add_hook("x", event_x);
#endif

  if (define_keys != NULL)
    define_keys(event_handler);

  // Now that we've called define_keys() (which might or might not
  // set have_dlight), we can turn on lighting.
  set_lighting(false);

  // Tick the clock once so we won't count the time spent loading up
  // files, above, in our frame rate average.
  ClockObject::get_global_clock()->tick();
  start_time = ClockObject::get_global_clock()->get_frame_time();
  start_frame_count = ClockObject::get_global_clock()->get_frame_count();

  DisplayCallback dcb;
  IdleCallback icb;

  for(jjj=0;jjj<NumWindows;jjj++) {
      wdxGraphicsWindow *pWin=pWinGrp->_windows[jjj];
      pWin->set_draw_callback(&dcb);
      pWin->set_idle_callback(&icb);
  }

#if 0
  if (!pWinGrp->_windows[jjj]->supports_update()) {
      framework_cat.info()
        << "Window type " << main_win->get_type()
      << " supports only the glut-style main loop interface.\n";

    pWin->register_draw_function(display_func);
    pWin->register_idle_function(idle_func);
    pWin->main_loop();

  } else 
#endif

  {
#ifdef USE_IPC
    if (forked_draw) {
      framework_cat.info() << "forking draw thread" << endl;
      draw_thread = thread::create(draw_loop);
      for (;;)
        icb.idle();
    } else
#endif
     {
       while(1) {
         g_bDoAppTraversal=true;
         for(jjj=0;jjj<NumWindows;jjj++) {
               g_pCurRenderWin=pWinGrp->_windows[jjj];
               g_pCurRenderWin->update();
         }
         ClockObject::get_global_clock()->tick();
         throw_event("NewFrame");

         if((!gridded_files.empty()) && gridmotiontype) {
           move_gridded_stuff(gridmotiontype,InfoArr, pRRptrArr, gridded_files_size*gridrepeats);
         }
       }
     }
  }

  if(!gridded_files.empty()) {
    delete [] pNodeArr;
    delete [] pRRptrArr;
    delete [] InfoArr;
  }
  return 1;
}
