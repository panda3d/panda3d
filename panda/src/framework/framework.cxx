// Filename: framework.cxx
// Created by:  cary (25Mar99)
// 
////////////////////////////////////////////////////////////////////

// We need to include bitMask.h first to avoid a VC++ compiler bug
// related to 2 parameter templates
#include <bitMask.h>

#include "framework.h"
#include "config_framework.h"

#include <pystub.h>
// Since framework.cxx includes pystub.h, no program that links with
// framework needs to do so.  No Python code should attempt to link
// with libframework.so.

#include <cullTraverser.h>
#include <appTraverser.h>
#include <directRenderTraverser.h>
#include <mouse.h>
#include <mouseWatcher.h>
#include <buttonThrower.h>
#include <eventHandler.h>
#include <throw_event.h>
#include <camera.h>
#include <geom.h>
#include <geomprimitives.h>
#include <renderRelation.h>
#include <dataRelation.h>
#include <geomNode.h>
#include <namedNode.h>
#include <pt_NamedNode.h>
#include <colorTransition.h>
#include <renderModeTransition.h>
#include <renderModeAttribute.h>
#include <materialTransition.h>
#include <dataGraphTraversal.h>
#include <trackball.h>
#include <driveInterface.h>
#include <transform2sg.h>
#include <texture.h>
#include <texturePool.h>
#include <textureTransition.h>
#include <textureAttribute.h>
#include <interactiveGraphicsPipe.h>
#include <noninteractiveGraphicsPipe.h>
#include <graphicsWindow.h>
#include <list>
#include <lightTransition.h>
#include <lightAttribute.h>
#include <materialTransition.h>
#include <materialAttribute.h>
#include <animControl.h>
#include <animControlCollection.h>
#include <auto_bind.h>
#include <ambientLight.h>
#include <directionalLight.h>
#include <pointLight.h>
#include <spotlight.h>
#include <dconfig.h>
#include <cullFaceTransition.h>
#include <cullFaceAttribute.h>
#include <pruneTransition.h>
#include <dftraverser.h>
#include <renderBuffer.h>
#include <loader.h>
#include <fogTransition.h>
#include <fogAttribute.h>
#include <clockObject.h>
#include <compose_matrix.h>
#include <notify.h>
#include <nullTransitionWrapper.h>
#include <nullAttributeWrapper.h>
#include <nullLevelState.h>
#include <sceneGraphReducer.h>
#include <textNode.h>
#include <depthTestTransition.h>
#include <depthWriteTransition.h>
#include <orthoProjection.h>
#include <transparencyTransition.h>
#include <bamReader.h>
#include <collisionRay.h>
#include <collisionNode.h>
#include <collisionTraverser.h>
#include <collisionHandlerFloor.h>
#include <nodePath.h>
#include <multiplexStream.h>

#ifdef USE_IPC
#include <ipc_file.h>
#include <ipc_mutex.h>
#include <ipc_thread.h>
#endif

Configure(framework);

ConfigureFn(framework) {
}

AppTraverser *app_traverser;
PT_NamedNode data_root;
PT_NamedNode root;
PT(GeomNode) geomnode;
PT_NamedNode render;
PT_NamedNode cameras;
PT(MouseAndKeyboard) mak;
PT(MouseWatcher) mouse_watcher;
PT(Trackball) trackball;
PT(DriveInterface) drive_interface;

static Node *current_trackball = NULL;
static Node *alt_trackball = NULL;

NodeAttributes initial_state;
Texture* ttex;
PT(GraphicsPipe) main_pipe;
PT(GraphicsPipe) rib_pipe;
PT(GraphicsWindow) main_win;
PT(GraphicsWindow) rib_win;
RenderRelation* first_arc;

PT_NamedNode lights;

PT(AmbientLight) light;
PT(DirectionalLight) dlight;
bool have_dlight = false;
PT(PointLight) plight;
PT(Spotlight) slight;

PT(Material) material;

PT(Fog) fog;

// Framerate vars

PT_NamedNode framerate_top;
RenderRelation *framerate_arc = (RenderRelation*)0L;
PT_NamedNode framerate_node;

PT(GraphicsLayer) framerate_layer;
PT_Node framerate_font;
PT(TextNode) framerate_text;

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
  bool reached_node(Node*, NullAttributeWrapper&, NullLevelState&);

  // No need to declare a forward_arc() function that simply returns
  // true; this is the default behavior.

public:
  GraphicsStateGuardian *_gsg;
};

bool NormalAddTraverser::
reached_node(Node *node, NullAttributeWrapper &, NullLevelState &) {
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
      PTA_Vertexf verts(2 * vert_count);
      for (i = 0; i < geom->get_num_geoms(); i++) {
	dDrawable *d = geom->get_geom(i);
	if (d->is_of_type(Geom::get_class_type())) {
	  PTA_Vertexf lverts;
	  PTA_ushort iverts;
	  GeomBindType vbond;
	  Geom *g = DCAST(Geom, d);
	  g->get_coords(lverts, vbond, iverts);
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
      gn->set_coords(verts, G_PER_VERTEX);
    }
    geom->add_geom(gn);
  }
  return true;
}

class NormalDelTraverser : 
  public TraverserVisitor<NullTransitionWrapper, NullLevelState> {
public:
  NormalDelTraverser(GraphicsStateGuardian *gsg) : _gsg(gsg) {}
  bool reached_node(Node*, NullAttributeWrapper&, NullLevelState&);
public:
  GraphicsStateGuardian *_gsg;
};

bool NormalDelTraverser::
reached_node(Node *node, NullAttributeWrapper &, NullLevelState &) {
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

void render_frame(GraphicsPipe *pipe) {
  app_traverser->traverse(render);
  int num_windows = pipe->get_num_windows();
  for (int w = 0; w < num_windows; w++) {
    GraphicsWindow *win = pipe->get_window(w);
    win->get_gsg()->render_frame(initial_state);
  }
  ClockObject::get_global_clock()->tick();
  throw_event("NewFrame");
}

// to be used with new display callback system
class DisplayCallback : public GraphicsWindow::Callback {
  public:
    virtual void draw(bool) {
      render_frame(main_pipe);
      if (extra_display_func != NULL)
        extra_display_func();
    }
};

// to be used with old GLUT callback system
void display_func( void ) {
  render_frame(main_pipe);
  if (extra_display_func != NULL)
    extra_display_func();
}

void set_lighting(bool enabled) {
  if (enabled) {
    // Enable the lights on the initial state.
    LightAttribute *la = new LightAttribute;
    la->set_on(light.p());

    if (have_dlight) {
      la->set_on(dlight.p());
    }
    initial_state.set_attribute(LightTransition::get_class_type(), la);

  } else {
    // Remove the lights from the initial state.
    initial_state.clear_attribute(LightTransition::get_class_type());
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
	col_trans->traverse(render);
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

void resize_func(int w, int h)
{
  main_win->resized(w, h);
}

void unpause_draw(void);

void event_esc(CPT_Event) {
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
  main_pipe = NULL;
  main_win = NULL;

  rib_pipe = NULL;
  rib_win = NULL;

#ifdef HAVE_NET
  if (PStatClient::get_global_pstats()->is_connected()) {
    framework_cat.info() << "Disconnecting from stats host" << endl;
    PStatClient::get_global_pstats()->disconnect();
  }
#endif

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
#ifdef HAVE_NET
  framework_cat.info() << "Connecting to stats host" << endl;
  PStatClient::get_global_pstats()->connect();
#else
  framework_cat.error() << "Stats host not supported." << endl;
#endif
}

void event_A(CPT_Event) {
#ifdef HAVE_NET
  if (PStatClient::get_global_pstats()->is_connected()) {
    framework_cat.info() << "Disconnecting from stats host" << endl;
    PStatClient::get_global_pstats()->disconnect();
  } else {
    framework_cat.error() << "Stats host is already disconnected." << endl;
  }
#else
  framework_cat.error() << "Stats host not supported." << endl;
#endif
}

void setup_framerate(void) {
  if (framerate_top != (NamedNode*)0L)
    return;

  framerate_top = new NamedNode("framerate_top");
  framerate_node = new NamedNode("framerate");
  framerate_arc = new RenderRelation(framerate_top, framerate_node);

  // Setup some overrides to turn off certain properties which we probably
  // won't need for 2-d objects.
  framerate_arc->set_transition(new DepthTestTransition(DepthTestProperty::M_none), 1);
  framerate_arc->set_transition(new DepthWriteTransition(DepthWriteTransition::off()), 1);
  framerate_arc->set_transition(new LightTransition(LightTransition::all_off()), 1);
  framerate_arc->set_transition(new MaterialTransition(MaterialTransition::off()), 1);
  framerate_arc->set_transition(new CullFaceTransition(CullFaceProperty::M_cull_none), 1);

  // create a 2-d camera.
  PT(Camera) cam2d = new Camera("framerate_cam");
  new RenderRelation(framerate_node, cam2d);
  cam2d->set_scene(framerate_top);

  Frustumf frustum2d;
  frustum2d.make_ortho_2D();
  cam2d->set_projection(OrthoProjection(frustum2d));

  // Now create a new layer
  // eventually this should be done through chanconfig'
  GraphicsChannel *chan = main_win->get_channel(0);
  nassertv(chan != (GraphicsChannel*)0L);

  framerate_layer = chan->make_layer();
  nassertv(framerate_layer != (GraphicsLayer *)0L);
  framerate_layer->set_active(true);

  DisplayRegion *dr = framerate_layer->make_display_region();
  nassertv(dr != (DisplayRegion *)0L);
  dr->set_camera(cam2d);

  // load the font
  framerate_font = loader.load_sync("cmtt12");

  if (framerate_font != (NamedNode *)0L) {
    framerate_text = new TextNode("framerate_text");
    new RenderRelation(framerate_node, framerate_text);

    LMatrix4f mat = LMatrix4f::scale_mat(0.05) *
      LMatrix4f::translate_mat(-0.95, 0.0, 0.95);

    framerate_text->set_transform(mat);
    framerate_text->set_font(framerate_font.p());
    framerate_text->set_card_color(0.5, 0.5, 0.5, 0.5);
    framerate_text->set_card_as_margin(0.5, 0.5, 0.2, 0.2);
    framerate_text->set_frame_color(1., 0., 0., 1.);
    framerate_text->set_frame_as_margin(0.5, 0.5, 0.2, 0.2);
    framerate_text->set_align(TM_ALIGN_LEFT);
    framerate_text->set_text_color(1., 1., 1., 1.);
    framerate_text->set_text("blah");
  }
}

void handle_framerate(void) {
  static bool first_time = true;
  static int buffer_count;
  static int buffer_size = framework.GetInt("framerate-buffer", 60);
  static double *prev_times = (double*)0L;
  static double *deltas = (double*)0L;

  if (framerate_layer == (GraphicsLayer*)0L)
    return;

  if (!framerate_layer->is_active()) {
    first_time = true;
    return;
  }

  double now = ClockObject::get_global_clock()->get_frame_time();

  if (first_time) {
    if (prev_times == (double*)0L) {
      prev_times = new double[buffer_size];
      deltas = new double[buffer_size];
    }
    buffer_count = 0;
    prev_times[buffer_count++] = now;
    first_time = false;
  } else if (buffer_count < buffer_size) {
    deltas[buffer_count-1] = now - prev_times[buffer_count-1];
    prev_times[buffer_count++] = now;
  } else {
    deltas[buffer_size-1] = now - prev_times[buffer_size-1];
    double delta = 0.;
    for (int i=0; i<buffer_size; ++i)
      delta += deltas[i];
    delta = delta / (double)buffer_size;
    double fps = 1. / delta;

    delta *= 1000.;

    ostringstream os;
    // I've decided that one digit to the right of the decimal should be
    // enough for now.  If we need more, it's easy to extend.
    int ifps = (int)fps;
    fps = fps - (double)ifps;
    fps *= 10.;
    int rfps = (int)fps;
    int idelta = (int)delta;
    delta = delta - (double)idelta;
    delta *= 10.;
    int rdelta = (int)delta;
    os << ifps << "." << rfps << " fps (" << idelta << "." << rdelta
       << " ms)";
    framerate_text->set_text(os.str());

    // now roll everything down one
    for (int j=0; j<buffer_size-1; ++j) {
      prev_times[j] = prev_times[j+1];
      deltas[j] = deltas[j+1];
    }
    prev_times[buffer_size-1] = now;
  }
}

void event_f_full(CPT_Event) {
  static bool is_on = true;
  setup_framerate();
  framerate_layer->set_active(is_on);
  is_on = !is_on;
}

void event_t(CPT_Event) {
  // The "t" key was pressed.  Toggle the showing of textures.
  static bool textures_enabled = true;

  textures_enabled = !textures_enabled;
  if (textures_enabled) {
    // Remove the override from the initial state.
    initial_state.clear_attribute(TextureTransition::get_class_type());
  } else {
    // Set an override on the initial state to disable texturing.
    TextureAttribute *ta = new TextureAttribute;
    ta->set_priority(100);
    initial_state.set_attribute(TextureTransition::get_class_type(), ta);
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
    RenderModeAttribute *rma = new RenderModeAttribute;
    rma->set_mode(RenderModeProperty::M_filled);
    CullFaceAttribute *cfa = new CullFaceAttribute;
    cfa->set_mode(CullFaceProperty::M_cull_clockwise);
    initial_state.set_attribute(RenderModeTransition::get_class_type(), rma);
    initial_state.set_attribute(CullFaceTransition::get_class_type(), cfa);

  } else {
    // Set the initial state up for wireframe mode. 
    RenderModeAttribute *rma = new RenderModeAttribute;
    rma->set_mode(RenderModeProperty::M_wireframe);
    CullFaceAttribute *cfa = new CullFaceAttribute;
    cfa->set_mode(CullFaceProperty::M_cull_none);
    initial_state.set_attribute(RenderModeTransition::get_class_type(), rma);
    initial_state.set_attribute(CullFaceTransition::get_class_type(), cfa);
  }
}

void event_b(CPT_Event) {
  // The 'b' key was pressed.  Toggle backface culling.
  static bool backface_mode = false;

  backface_mode = !backface_mode;
  if (backface_mode) {
    material->set_twoside(true);
    CullFaceAttribute *cfa = new CullFaceAttribute;
    cfa->set_mode(CullFaceProperty::M_cull_none);
    initial_state.set_attribute(CullFaceTransition::get_class_type(), cfa);
  } else {
    material->set_twoside(false);
    CullFaceAttribute *cfa = new CullFaceAttribute;
    cfa->set_mode(CullFaceProperty::M_cull_clockwise);
    initial_state.set_attribute(CullFaceTransition::get_class_type(), cfa);
  }
}

void event_R(CPT_Event) {
  // The "R" key was pressed.  Dump a RIB file.
  if (rib_win == (GraphicsWindow*)0L)
    return;
  nout << "Writing RIB frame " << rib_win->get_frame_number() << "\n";
  render_frame(rib_pipe);
}

void event_grave(CPT_Event) {
  PixelBuffer p;
  GraphicsStateGuardian* g = main_win->get_gsg();
  const RenderBuffer& r = g->get_render_buffer(RenderBuffer::T_front);

  p.set_xsize(main_win->get_width());
  p.set_ysize(main_win->get_height());
  p._image = PTA_uchar(main_win->get_width() * main_win->get_height() * 3);

  p.copy(main_win->get_gsg(), 
	main_win->get_gsg()->get_current_display_region(),r);
  ostringstream s;
  s << "frame" << main_win->get_frame_number() << ".pnm";
  p.write(s.str());
}

void event_n(CPT_Event) {
  static bool normals_on = false;

  normals_on = !normals_on;
  if (normals_on) {
    NormalAddTraverser trav(main_win->get_gsg());
    df_traverse(render, trav, NullAttributeWrapper(), NullLevelState(),
		RenderRelation::get_class_type());
  } else {
    NormalDelTraverser trav(main_win->get_gsg());
    df_traverse(render, trav, NullAttributeWrapper(), NullLevelState(),
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
  gr.apply_transitions(root);
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

    NodeRelation *arc = new RenderRelation(cameras, ray_node);

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
    FogAttribute *fa = new FogAttribute;
    fa->set_on(fog);
    initial_state.set_attribute(FogTransition::get_class_type(), fa);
  } else {
    FogAttribute *fa = new FogAttribute;
    initial_state.set_attribute(FogTransition::get_class_type(), fa);
  }
}

#ifdef USE_IPC
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
    handle_framerate();
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
#endif

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

  typedef vector<Filename> Files;
  Files files;

  if (first_init != NULL)
    first_init();

  for (int a = 1; a < argc; a++)
    if ((argv[a] != (char*)0L) && ((argv[a])[0] != '-') &&
	((argv[a])[0] != '+') && ((argv[a])[0] != '#'))
      files.push_back(Filename::from_os_specific(argv[a]));

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
    << "Opened a '" << main_pipe->get_type().get_name()
    << "' interactive graphics pipe." << endl;

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

  ChanCfgOverrides override;

  // need to find a better way to differentiate unsigned int from regular
  override.setField(ChanCfgOverrides::Mask,
		    ((unsigned int)(W_DOUBLE|W_DEPTH|W_MULTISAMPLE)));
  override.setField(ChanCfgOverrides::Title, "Demo");

  std::string conf = framework.GetString("chan-config", chan_config);
  if (extra_overrides_func != NULL)
    extra_overrides_func(override, conf);

  // Create the render node
  render = new NamedNode("render");

  // make a node for the cameras to live under
  cameras = new NamedNode("cameras");
  RenderRelation* arc1 = new RenderRelation(render, cameras);

  main_win = ChanConfig(main_pipe, conf, cameras, render, override);
  assert(main_win != (GraphicsWindow*)0L);

  // is ok if this doesn't work or returns NULL
  if (rib_pipe != (GraphicsPipe*)0L) { 
    Node *rib_cameras = new NamedNode("rib_cameras");
    new RenderRelation(render, rib_cameras);
    rib_win = ChanConfig(rib_pipe, "single", rib_cameras, render, override);
  }

  // Make a node for the lights to live under.  We put the lights in
  // with the cameras, so they'll stay locked to our point-of-view.

  lights = new NamedNode("lights");
  new RenderRelation(cameras, lights);

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
  CullFaceAttribute *cfa = new CullFaceAttribute;
  cfa->set_mode(CullFaceProperty::M_cull_clockwise);
  initial_state.set_attribute(CullFaceTransition::get_class_type(), cfa);

  // Set up a default material
  material = new Material;
  material->set_ambient( Colorf( 1, 1, 1, 1 ) );
  MaterialAttribute *ma = new MaterialAttribute;
  ma->set_on(material);
  initial_state.set_attribute(MaterialTransition::get_class_type(), ma);

  // Set up a default fog
  fog = new Fog;

  // Create the data graph root.
  data_root = new NamedNode( "data" );

  // Create a mouse and put it in the data graph.
  mak = new MouseAndKeyboard( main_win, 0 );
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
  tball2cam->set_arc(arc1);
  new DataRelation(trackball, tball2cam);

  PT(Transform2SG) drive2cam = new Transform2SG("drive2cam");
  drive2cam->set_arc(arc1);
  new DataRelation(drive_interface, drive2cam);

  // Create a ButtonThrower to throw events from the keyboard.
  PT(ButtonThrower) et = new ButtonThrower("kb-events");
  new DataRelation(mouse_watcher, et);

  root = new NamedNode("root");
  first_arc = new RenderRelation(render, root, 100);

  if (files.empty() && framework.GetBool("have-omnitriangle", true)) {
    // The user did not specify an file.  Create some default
    // geometry.
      
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
    geom->set_coords(coords, G_PER_VERTEX);
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

    Files::const_iterator fi;
    for (fi = files.begin();
	 fi != files.end();
	 ++fi) {
      const Filename &filename = (*fi);
      PT_Node node = loader.load_sync(filename);

      if (node == (Node *)NULL) {
	framework_cat.error()
	  << "Unable to load file " << filename << "\n";
      } else {
	new RenderRelation(root, node);
      }
    }

    // If we happened to load up both a character file and its
    // matching animation file, attempt to bind them together now.
    AnimControlCollection anim_controls;
    auto_bind(root, anim_controls, ~0);

    // And start looping any animations we successfully bound.
    anim_controls.loop_all(true);

    // Now prepare all the textures with the GSG.
    NodePath render_path(render);
    render_path.prepare_scene(main_win->get_gsg());
  }

  // Set up keyboard events.
  event_handler.add_hook("escape", event_esc);
  event_handler.add_hook("q", event_esc);
  event_handler.add_hook("f", event_f);
  event_handler.add_hook("F", event_f_full);
  event_handler.add_hook("t", event_t);
  event_handler.add_hook("l", event_l);
  event_handler.add_hook("w", event_w);
  event_handler.add_hook("b", event_b);
  event_handler.add_hook("R", event_R);
  event_handler.add_hook("grave", event_grave);
  event_handler.add_hook("n", event_n);
  event_handler.add_hook("c", event_c);
  event_handler.add_hook("D", event_D);
  event_handler.add_hook("g", event_g);
  event_handler.add_hook("C", event_C);
  event_handler.add_hook("N", event_N);
  event_handler.add_hook("S", event_S);
  event_handler.add_hook("A", event_A);
  event_handler.add_hook("p", event_p);
  event_handler.add_hook("P", event_P);

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

  if (framework.Defined("clear-value")) {
    float cf = framework.GetFloat("clear-value", 0.);
    Colorf c(cf, cf, cf, 1.);
    main_win->get_gsg()->set_color_clear_value(c);
  }

  if (!main_win->supports_update()) {
    framework_cat.info()
      << "Window type " << main_win->get_type()
      << " supports only the glut-style main loop interface.\n";

    main_win->register_draw_function(display_func);
    main_win->register_idle_function(idle_func);
    main_win->register_resize_function(resize_func);
    main_win->main_loop();

  } else {
    DisplayCallback dcb;
    main_win->set_draw_callback(&dcb);
    IdleCallback icb;

#ifdef USE_IPC
    if (forked_draw) {
      framework_cat.info() << "forking draw thread" << endl;
      draw_thread = thread::create(draw_loop);
      for (;;)
	icb.idle();
    } else 
#endif
    {
      main_win->set_idle_callback(&icb);
      for (;;) {
	main_win->update();
	handle_framerate();
      }
    }
  }
  return 1;
}
