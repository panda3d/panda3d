// Filename: showBase.cxx
// Created by:  shochet (02Feb00)
// 
////////////////////////////////////////////////////////////////////

#include "showBase.h"

#include <throw_event.h>
#include <camera.h>
#include <renderRelation.h>
#include <namedNode.h>
#include <renderModeTransition.h>
#include <renderModeAttribute.h>
#include <textureTransition.h>
#include <textureAttribute.h>
#include <interactiveGraphicsPipe.h>
#include <graphicsWindow.h>
#include <chancfg.h>
#include <cullFaceTransition.h>
#include <cullFaceAttribute.h>
#include <dftraverser.h>
#include <renderBuffer.h>
#include <clockObject.h>
#include <animControl.h>
#include <nodeRelation.h>
#include <dataGraphTraversal.h>
#include <depthTestTransition.h>
#include <depthTestAttribute.h>
#include <depthWriteTransition.h>
#include <depthWriteAttribute.h>
#include <lightTransition.h>
#include <materialTransition.h>
#include <camera.h>
#include <frustum.h>
#include <orthoProjection.h>
#include <appTraverser.h>
#include <collisionTraverser.h>

static CollisionTraverser *collision_traverser = NULL;

void render_frame(GraphicsPipe *pipe,
		  NodeAttributes &initial_state) {
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
  DisplayCallback(GraphicsPipe *pipe, Node *render, NodeAttributes *initial_state) :
    _pipe(pipe),
    _render(render),
    _initial_state(initial_state),
    _app_traverser(RenderRelation::get_class_type()) { }
  
  virtual void draw(bool) {
    _app_traverser.traverse(_render);
    render_frame(_pipe, *_initial_state);
  }
  
  PT(GraphicsPipe) _pipe;
  PT(Node) _render;
  NodeAttributes *_initial_state;
  AppTraverser _app_traverser;
};



// to be used with new display callback system
class IdleCallback : public GraphicsWindow::Callback {
public:
  IdleCallback(Node *render, Node *data_root) {
    _render = render;
    _data_root = data_root;
  }
  
  virtual void idle(void) {
    // Initiate the data traversal, to send device data down its
    // respective pipelines.
    traverse_data_graph(_data_root);
    if (collision_traverser != (CollisionTraverser *)NULL) {
      collision_traverser->traverse(_render);
    }
  }
  
  PT(Node) _render;
  PT(Node) _data_root;
};


PT(GraphicsPipe) make_graphics_pipe() {
  PT(GraphicsPipe) main_pipe;
  
  // load display modules
  GraphicsPipe::resolve_modules();

  nout << "Known pipe types:" << endl;
  GraphicsPipe::_factory.write_types(nout, 2);

  // Create a window
  main_pipe = GraphicsPipe::_factory.
    make_instance(InteractiveGraphicsPipe::get_class_type());

  if (main_pipe == (GraphicsPipe*)0L) {
    nout << "No interactive pipe is available!  Check your Configrc!\n";
    return NULL;
  }

  nout << "Opened a '" << main_pipe->get_type().get_name()
       << "' interactive graphics pipe." << endl;
  return main_pipe;
}

PT(GraphicsWindow) make_graphics_window(GraphicsPipe *pipe, 
					NamedNode *render,
					NamedNode *camera,
					NamedNode *data_root,
					NodeAttributes &initial_state) {
  PT(GraphicsWindow) main_win;
  ChanCfgOverrides override;

  // Turn on backface culling
  CullFaceAttribute *cfa = new CullFaceAttribute;
  cfa->set_mode(CullFaceProperty::M_cull_clockwise);
  initial_state.set_attribute(CullFaceTransition::get_class_type(), cfa);
  DepthTestAttribute *dta = new DepthTestAttribute;
  initial_state.set_attribute(DepthTestTransition::get_class_type(), dta);
  DepthWriteAttribute *dwa = new DepthWriteAttribute;
  initial_state.set_attribute(DepthWriteTransition::get_class_type(), dwa);

  override.setField(ChanCfgOverrides::Mask,
		    ((unsigned int)(W_DOUBLE|W_DEPTH|W_MULTISAMPLE)));

  main_win = ChanConfig(pipe, "single", camera, render, override);
  assert(main_win != (GraphicsWindow*)0L);

  DisplayCallback *dcb = new DisplayCallback(pipe, render, &initial_state);
  IdleCallback *icb = new IdleCallback(render, data_root);

  // Set draw and idle callbacks
  main_win->set_draw_callback(dcb);
  main_win->set_idle_callback(icb);

  return main_win;
}

// Create a scene graph, associated with the indicated window, that
// can contain 2-d geometry and will be rendered on top of the
// existing 3-d window.  Returns the top node of the scene graph.
NodePath
setup_panda_2d(PT(GraphicsWindow) win) {
  PT(Node) render2d_top;
  
  render2d_top = new NamedNode("render2d_top");
  Node *render2d = new NamedNode("render2d");
  RenderRelation *render2d_arc = new RenderRelation(render2d_top, render2d);

  // Set up some overrides to turn off certain properties which we
  // probably won't need for 2-d objects.
  render2d_arc->set_transition(new DepthTestTransition(DepthTestProperty::M_none), 1);
  render2d_arc->set_transition(new DepthWriteTransition(DepthWriteTransition::off()), 1);
  render2d_arc->set_transition(new LightTransition(LightTransition::all_off()), 1);
  render2d_arc->set_transition(new MaterialTransition(MaterialTransition::off()), 1);
  render2d_arc->set_transition(new CullFaceTransition(CullFaceProperty::M_cull_none), 1);

  // Create a 2-d camera.
  Camera *cam2d = new Camera("cam2d");
  new RenderRelation(render2d, cam2d);
  cam2d->set_scene(render2d_top);

  Frustumf frustum2d;
  frustum2d.make_ortho(-1000,1000);
  cam2d->set_projection(OrthoProjection(frustum2d));

  // Now create a new layer.
  GraphicsChannel *chan = win->get_channel(0);
  nassertr(chan != (GraphicsChannel *)NULL, NodePath());

  GraphicsLayer *layer = chan->make_layer();
  nassertr(layer != (GraphicsLayer *)NULL, NodePath());

  DisplayRegion *dr = layer->make_display_region();
  nassertr(dr != (DisplayRegion *)NULL, NodePath());
  dr->set_camera(cam2d);

  return NodePath(render2d_arc);
}

// Enable the collision traversal using a particular traverser.
void set_collision_traverser(CollisionTraverser *traverser) {
  collision_traverser = traverser;
}
// Stop the collision traversal.
void clear_collision_traverser() {
  collision_traverser = NULL;
}


void toggle_wireframe(NodeAttributes &initial_state) {
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


void toggle_backface(NodeAttributes &initial_state) {
  static bool backface_mode = false;

  // Toggle the state variable
  backface_mode = !backface_mode;

  if (backface_mode) {
    // Turn backface culling off
    CullFaceAttribute *cfa = new CullFaceAttribute;
    cfa->set_mode(CullFaceProperty::M_cull_none);
    initial_state.set_attribute(CullFaceTransition::get_class_type(), cfa);
  } else {
    // Turn backface culling on
    CullFaceAttribute *cfa = new CullFaceAttribute;
    cfa->set_mode(CullFaceProperty::M_cull_clockwise);
    initial_state.set_attribute(CullFaceTransition::get_class_type(), cfa);
  }
}


void toggle_texture(NodeAttributes &initial_state) {
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
