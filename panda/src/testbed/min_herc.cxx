// Filename: min_herc.cxx
// Created by:  jason (28Jun00)
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

//Shaders
#include "projtexShader.h"
#include "spotlightShader.h"
#include "projtexShadower.h"
#include "planarReflector.h"

//Channel stuff
#include "chancfg.h"

//Textures
#include "texture.h"
#include "texturePool.h"

//Transitions
#include "textureTransition.h"
#include "shaderTransition.h"
#include "lightTransition.h"
#include "transformTransition.h"
#include "colorBlendTransition.h"

//Light stuff
#include "light.h"
#include "spotlight.h"

//Nodes
#include "pt_NamedNode.h"
#include "geomNode.h"

//Relations (arcs)
#include "renderRelation.h"
#include "nodeRelation.h"
#include "dataRelation.h"

//Misc
#include "dconfig.h"
#include "framework.h"
#include "loader.h"
#include "eventHandler.h"
#include "geomLine.h"

//Math/Matrix/Vector/Transformation stuff
#include "transform2sg.h"
#include "look_at.h"
#include "perspectiveLens.h"
#include "get_rel_pos.h"

//Control/IO
#include "planarSlider.h"
#include "mouse.h"

//Animation
#include "animControl.h"
#include "animControlCollection.h"
#include "auto_bind.h"

Configure(min_herc);
ConfigureFn(min_herc) {
}

PT(LensNode) tex_proj;
PT(Trackball) tex_proj_trackball;
PT(ProjtexShader) proj_shader;
PT(ProjtexShadower) proj_shadow;
PT(PlanarReflector) preflect;
PT(GeomNode) proj_geom_node;
PT_NamedNode spot_shaft;

PT_NamedNode herc;
PT_NamedNode ball;
PT_NamedNode hide_ball;

ShaderTransition shader_trans;

PT(Spotlight) tex_proj_spot;
PT(Trackball) tex_spot_trackball;
PT(SpotlightShader) spot_shader;

PT(PlanarSlider) ball_slider;
bool follow_ball;

RenderRelation* room_arc;
RenderRelation* spot_arc;
RenderRelation* ball_arc;
RenderRelation* hide_ball_arc;
RenderRelation* herc_arc;
RenderRelation* herc_rot_arc;
RenderRelation* camera_model_arc;

extern PT_NamedNode render;
extern NodeAttributes initial_state;
extern PT_NamedNode lights;
extern PT_NamedNode egg_root;
extern PT(GeomNode) geomnode;
extern PT_NamedNode cameras;
extern PT(MouseAndKeyboard) mak;

extern void set_alt_trackball(Node *trackball);

extern PT(GraphicsWindow) main_win;

extern int framework_main(int argc, char *argv[]);
extern void (*extra_display_func)();
extern void (*define_keys)(EventHandler&);
extern void (*extra_overrides_func)(ChanCfgOverrides&, std::string&);
extern void (*additional_idle)();

void herc_overrides_func(ChanCfgOverrides &override, std::string&) {
  override.setField(ChanCfgOverrides::Mask,
        ((unsigned int)(W_DOUBLE|W_DEPTH|W_MULTISAMPLE|W_STENCIL)));
  override.setField(ChanCfgOverrides::Title, "Lighting Demo");
}


void herc_idle() {
  static const double walk_speed = 4.0;  // feet per second

  if (follow_ball) {
    LPoint3f bp = get_rel_pos(ball, herc);
    LVector2f bv2(bp[0], bp[1]);
    float dist = length(bv2);
    if (dist > 0.0001) {
      LMatrix4f mat;
      look_at(mat, LPoint3f(-bp[0], -bp[1], 0.0));
      herc_rot_arc->set_transition(new TransformTransition(mat));

      float stride = walk_speed * ClockObject::get_global_clock()->get_dt();
      if (dist > stride) {
        LVector2f step = bv2 / dist * stride;

        const TransformTransition *tt;
        if (!get_transition_into(tt, herc_arc)) {
          herc_arc->set_transition
            (new TransformTransition
             (LMatrix4f::translate_mat(step[0], step[1], 0.0)));
        } else {
          LMatrix4f mat = tt->get_matrix();
          mat(3, 0) += step[0];
          mat(3, 1) += step[1];
          herc_arc->set_transition(new TransformTransition(mat));
        }
      }
    }
  }
}

void event_p(CPT_Event) {
  // The "p" key was pressed.  Toggle projected texture.
  static bool projtex_mode = false;

  projtex_mode = !projtex_mode;
  if (!projtex_mode) {
    // Set the normal mode on the render arc.
    clear_shader(room_arc, proj_shader);
    clear_shader(herc_arc, proj_shader);
    clear_shader(ball_arc, proj_shader);

    set_alt_trackball(NULL);
    remove_child(tex_proj, proj_geom_node, RenderRelation::get_class_type());

  } else {
    set_shader(room_arc, proj_shader);
    set_shader(herc_arc, proj_shader);
    set_shader(ball_arc, proj_shader);

    set_alt_trackball(tex_proj_trackball);
    // Display the texture projector frustum
    RenderRelation *prr = new RenderRelation(tex_proj, proj_geom_node);
    LightTransition *plt = new LightTransition(LightTransition::all_off());
    prr->set_transition(plt);
  }
}

void event_s(CPT_Event) {
  // The "s" key was pressed.  Toggle projected texture spotlight.
  static bool projtexspot_mode = false;
  LightTransition light_trans;

  projtexspot_mode = !projtexspot_mode;
  if (!projtexspot_mode) {
    // Set the normal mode on the render arc.
    clear_shader(room_arc, spot_shader);
    clear_shader(herc_arc, spot_shader);
    clear_shader(ball_arc, spot_shader);

    set_alt_trackball(NULL);
    remove_child(tex_proj_spot, spot_shaft, RenderRelation::get_class_type());

  } else {
    set_shader(room_arc, spot_shader);
    set_shader(herc_arc, spot_shader);
    set_shader(ball_arc, spot_shader);

    set_alt_trackball(tex_spot_trackball);
    new RenderRelation(tex_proj_spot, spot_shaft, 10);
  }
}

void event_d(CPT_Event) {
  // The "d" key was pressed.  Toggle projected texture shadows.
  static bool projtex_shadow_mode = false;

  projtex_shadow_mode = !projtex_shadow_mode;
  if (!projtex_shadow_mode) {
    // Set the normal mode on the render arc.
    clear_shader(room_arc, proj_shadow);
    set_alt_trackball(NULL);

  } else {
    set_shader(room_arc, proj_shadow);
    set_alt_trackball(tex_spot_trackball);
  }
}

void event_r(CPT_Event) {
  // The "r" key was pressed.  Toggle planar reflection.
  static bool plane_reflect_mode = false;

  plane_reflect_mode = !plane_reflect_mode;
  if (!plane_reflect_mode) {
    // Set the normal mode on the render arc.
    clear_shader(room_arc, preflect);

  } else {
    // Set an override on the initial state.
    set_shader(room_arc, preflect);
  }
}

void event_z(CPT_Event) {
  // The "z" key was pressed.  Allow the user to move the ball around.
  set_alt_trackball(ball_slider);
}

void event_Z(CPT_Event) {
  // The "Z" key was pressed.  Toggle follow-ball mode.
  follow_ball = !follow_ball;

  if (follow_ball) {
    // Hide the ball while we're following it.
    remove_arc(hide_ball_arc);
  } else {
    // Reveal the ball when we're done following it.
    hide_ball_arc = new RenderRelation(render, hide_ball);
  }
}

void herc_keys(EventHandler &eh) {
  Loader loader;

  eh.add_hook("p", event_p); // Projected Texture Shader
  eh.add_hook("s", event_s); // Projected Texture Spotlight
  eh.add_hook("d", event_d); // Projected Texture Shadower
  eh.add_hook("r", event_r); // Planar Reflector
  eh.add_hook("z", event_z); // Move ball
  eh.add_hook("Z", event_Z); // Follow ball

//==========================================================================
// Models
//==========================================================================
  // Load herc
  PT_NamedNode herc_model = DCAST(NamedNode, loader.load_sync("herc-6000.egg"));
  PT_NamedNode herc_anim = DCAST(NamedNode, loader.load_sync("HB_1_HE1.egg"));
  assert(herc_model != (NamedNode *)NULL &&
         herc_anim != (NamedNode *)NULL);
  PT_NamedNode herc_parent = new NamedNode("herc_parent");
  new RenderRelation(herc_parent, herc_model);
  new RenderRelation(herc_parent, herc_anim);

  AnimControlCollection anim_controls;
  auto_bind(herc_parent, anim_controls, ~0);

  // And start looping any animations we successfully bound.
  anim_controls.loop_all(true);

  PT_NamedNode herc_rot = new NamedNode("herc_rot");
  RenderRelation *herc_arc1 = new RenderRelation(herc_rot, herc_parent);
  herc_arc1->set_transition
    (new TransformTransition
     (LMatrix4f::scale_mat(LVector3f(0.25, 0.25, 0.25))));
  herc = new NamedNode("herc");
  herc_rot_arc = new RenderRelation(herc, herc_rot);
  herc_arc = new RenderRelation(render, herc);

  // Load ball
  ball = DCAST(NamedNode, loader.load_sync("marble_ball.egg"));
  assert(ball != (NamedNode *)NULL);
  PT_NamedNode scaled_ball = new NamedNode("scaled_ball");
  RenderRelation *ball_arc1 = new RenderRelation(scaled_ball, ball);
  ball_arc1->set_transition
    (new TransformTransition(LMatrix4f::scale_mat(0.2)));
  hide_ball = new NamedNode("hide_ball");
  ball_arc = new RenderRelation(hide_ball, scaled_ball);
  ball_arc->set_transition
    (new TransformTransition(LMatrix4f::translate_mat(4., 2., 1.)));
  hide_ball_arc = new RenderRelation(render, hide_ball);

  // Control the ball using a PlanarSlider tform.
  ball_slider = new PlanarSlider("ball_slider");
  ball_slider->set_transform(LMatrix4f::translate_mat(0.0, 0.0, 1.0) *
                             LMatrix4f::scale_mat(7.0, -7.0, 1.0));
  ball_slider->set_mouse_pos(LPoint2f(4.0 / 7.0, 2.0 / -7.0));
  Transform2SG *slider2ball = new Transform2SG("slider2ball");
  slider2ball->set_arc(ball_arc);
  new RenderRelation(ball_slider, slider2ball);
  follow_ball = false;

  // Load the room file
  PT_NamedNode room = DCAST(NamedNode, loader.load_sync("lfloor.egg"));
  assert(room != (NamedNode *)NULL);
  room_arc = new RenderRelation(render, room);

  // Load up a camera model to visualize our eyepoint.
  PT_NamedNode camera_model = DCAST(NamedNode, loader.load_sync("camera.egg"));
  assert(camera_model != (NamedNode *)NULL);
  camera_model_arc = new RenderRelation(cameras, camera_model);

  // Remove any model that was loaded by default or on command line
  if (root != (NamedNode *)NULL)
    remove_child(render, root, RenderRelation::get_class_type());
  if (geomnode != (GeomNode *)NULL)
    remove_child(render, geomnode, RenderRelation::get_class_type());


//==========================================================================
// Projected Texture Shader
//==========================================================================
  // Load a texture to project
  Texture* tex = TexturePool::load_texture("smiley.rgba");
  tex->set_minfilter(Texture::FT_linear);
  tex->set_magfilter(Texture::FT_linear);
  tex->set_wrapu(Texture::WM_clamp);
  tex->set_wrapv(Texture::WM_clamp);

  // Put the texture projector into the scene graph
  tex_proj = new LensNode("texture_projector");
  RenderRelation* proj_arc = new RenderRelation(render, tex_proj);

  // Create a trackball to spin this around.
  tex_proj_trackball = new Trackball("tex_proj_trackball");
  tex_proj_trackball->set_invert(false);
  tex_proj_trackball->set_rel_to(cameras);
  Transform2SG *tball2cam = new Transform2SG("tball2cam");
  tball2cam->set_arc(proj_arc);
  new RenderRelation(tex_proj_trackball, tball2cam);

  // Raise it and aim it at the origin
  LMatrix4f proj_mat;
  LPoint3f proj_pos = LPoint3f::rfu(2., 3., 8.);
  LVector3f fwd_vec = proj_pos - LPoint3f::origin();
  look_at(proj_mat, -fwd_vec);
  proj_mat.set_row(3, proj_pos);
  tex_proj_trackball->set_mat(proj_mat);
  proj_arc->set_transition(new TransformTransition(proj_mat));

  // Create a shader for the texture projector
  proj_shader = new ProjtexShader(tex);
  proj_shader->add_frustum(tex_proj);

  // Create a wireframe representation of the texture projector frustum
  PT(Geom) proj_geom =
    tex_proj->get_lens()->make_geometry();
  proj_geom_node = new GeomNode("proj_geometry");
  proj_geom_node->add_geom(proj_geom);


//==========================================================================
// Projected Texture Spotlight Shader
//==========================================================================
  tex_proj_spot = new Spotlight("tex_proj_spotlight");
  spot_arc = new RenderRelation(render, tex_proj_spot, 10);

  // Create a trackball to spin this around.
  tex_spot_trackball = new Trackball("tex_spot_trackball");
  tex_spot_trackball->set_invert(false);
  tex_spot_trackball->set_rel_to(cameras);
  tball2cam = new Transform2SG("tball2cam");
  tball2cam->set_arc(spot_arc);
  new RenderRelation(tex_spot_trackball, tball2cam);

  // Raise it and aim it at the origin
  LMatrix4f spot_mat;
  LPoint3f spot_pos = LPoint3f::rfu(-4., -3., 8.);
  LVector3f spot_vec = spot_pos - LPoint3f::origin();
  look_at(spot_mat, -spot_vec);
  spot_mat.set_row(3, spot_pos);
  tex_spot_trackball->set_mat(spot_mat);
  spot_arc->set_transition(new TransformTransition(spot_mat));

  // Create a shader for the spotlight
  spot_shader = new SpotlightShader;
  spot_shader->add_frustum(tex_proj_spot);

  // Create a light shaft for the spotlight
  spot_shaft = tex_proj_spot->make_geometry(0.05, 8.0, 36);


//==========================================================================
// Projected Texture Shadower
//==========================================================================
  proj_shadow = new ProjtexShadower;
  proj_shadow->add_frustum(tex_proj_spot);
  proj_shadow->add_caster(herc);
  proj_shadow->add_caster(ball);
  proj_shadow->add_caster(camera_model);

//==========================================================================
// Planar Reflector
//==========================================================================
  // Create a plane that corresponds to the floor of the room
  Planef p(LVector3f::up(), LPoint3f::origin());
  PlaneNode* plane_node = new PlaneNode;
  plane_node->set_plane(p);
  new RenderRelation(room, plane_node);
  preflect = new PlanarReflector(plane_node);
  preflect->add_caster(ball);
  preflect->add_caster(herc);
  preflect->add_caster(camera_model);

  additional_idle = &herc_idle;
}

int main(int argc, char *argv[]) {
  define_keys = &herc_keys;
  extra_overrides_func = &herc_overrides_func;
  return framework_main(argc, argv);
}
