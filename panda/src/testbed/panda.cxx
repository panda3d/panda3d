// Filename: panda.cxx
// Created by:  
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

#include "pandabase.h"
#include "framework.h"
#include "eventHandler.h"
#include "chancfg.h"
#include "lightTransition.h"
#include "transformTransition.h"
#include "get_rel_pos.h"
#include "directionalLight.h"
#include "renderRelation.h"
#include "frustum.h"
#include "perspectiveLens.h"
#include "shaderTransition.h"
#include "texture.h"
#include "texturePool.h"
#include "spotlight.h"
#include "pt_Node.h"
#include "pt_NamedNode.h"
#include "loader.h"
#include "auto_bind.h"
#include "animControlCollection.h"
#include "trackball.h"
#include "planarSlider.h"
#include "transform2sg.h"
#include "projtexShader.h"
#include "spotlightShader.h"
#include "projtexShadower.h"
#include "spheretexHighlighter.h"
#include "spheretexReflector.h"
#include "plane.h"
#include "planeNode.h"
#include "planarReflector.h"
#include "outlineShader.h"
#include "geomLine.h"
#include "look_at.h"
#include "geomNode.h"

//From framework
extern PT(GeomNode) geomnode;
extern RenderRelation* first_arc;
extern Loader loader;

PT_NamedNode panda;
PT_NamedNode ball;
PT_NamedNode room;
PT_NamedNode hide_ball;
PT_NamedNode camera_model;

PT(RenderRelation) panda_arc;
PT(RenderRelation) panda_rot_arc;
PT(RenderRelation) room_arc;
PT(RenderRelation) ball_arc;
PT(RenderRelation) hide_ball_arc;

bool follow_ball;
PT(PlanarSlider) ball_slider;

PT(LensNode) tex_proj;
PT(ProjtexShader) proj_shader;
PT(GeomNode) proj_geom_node;
PT(Trackball) tex_proj_trackball;
PT(SpotlightShader) spot_shader;
PT_NamedNode spot_shaft;
PT(Spotlight) tex_proj_spot;
PT(Trackball) tex_spot_trackball;
PT(ProjtexShadower) proj_shadow;
PT(SpheretexHighlighter) highlight;
PT(SpheretexShader) spheretex;
PT(SpheretexReflector) sreflect;
PT(PlanarReflector) preflect;
PT(OutlineShader) outline_shader;

void panda_overrides_func(ChanCfgOverrides& override, std::string&) {
  override.setField(ChanCfgOverrides::Mask,
                   ((unsigned int)(W_DOUBLE|W_DEPTH|W_MULTISAMPLE|W_STENCIL)));
  override.setField(ChanCfgOverrides::Title, "Panda Demo");
}

void panda_idle(void) {
  static const double walk_speed = 4.0;  // feet per second
  if (follow_ball) {
    LPoint3f bp = get_rel_pos(ball, panda);
    LVector2f bv2(bp[0], bp[1]);
    float dist = length(bv2);
    if (dist > 0.0001) {
      LMatrix4f mat;
      look_at(mat, LPoint3f(-bp[0], -bp[1], 0.));
      panda_rot_arc->set_transition(new TransformTransition(mat));

      float stride = walk_speed * ClockObject::get_global_clock()->get_dt();
      if (dist > stride) {
        LVector2f step = bv2 / dist * stride;
        const TransformTransition *tt;
        if (!get_transition_into(tt, panda_arc)) {
          panda_arc->set_transition
            (new TransformTransition
             (LMatrix4f::translate_mat(step[0], step[1], 0.)));
        } else {
          LMatrix4f mat = tt->get_matrix();
          mat(3, 0) += step[0];
          mat(3, 1) += step[1];
          panda_arc->set_transition(new TransformTransition(mat));
        }
      }
    }
  }
}

void event_p(CPT_Event) {
  static bool projtex_mode = false;

  projtex_mode = !projtex_mode;
  if (!projtex_mode) {
    // disable the projtex shader
    clear_shader(room_arc, proj_shader);
    clear_shader(panda_arc, proj_shader);
    clear_shader(ball_arc, proj_shader);

    set_alt_trackball((Node*)0L);
    remove_child(tex_proj, proj_geom_node, RenderRelation::get_class_type());
  } else {
    // enable the projtex shader
    set_shader(room_arc, proj_shader);
    set_shader(panda_arc, proj_shader);
    set_shader(ball_arc, proj_shader);

    set_alt_trackball(tex_proj_trackball);
    // Display the texture projector frustum
    RenderRelation *prr = new RenderRelation(tex_proj, proj_geom_node);
    LightTransition *plt = new LightTransition(LightTransition::all_off());
    prr->set_transition(plt);
  }
}

void event_s(CPT_Event) {
  static bool projtexspot_mode = false;

  projtexspot_mode = !projtexspot_mode;
  if (!projtexspot_mode) {
    // disable the projtex spotlight shader
    clear_shader(room_arc, spot_shader);
    clear_shader(panda_arc, spot_shader);
    clear_shader(ball_arc, spot_shader);

    set_alt_trackball((Node*)0L);
    remove_child(tex_proj_spot, spot_shaft, RenderRelation::get_class_type());
  } else {
    // enable the projtex spotlight shader
    set_shader(room_arc, spot_shader);
    set_shader(panda_arc, spot_shader);
    set_shader(ball_arc, spot_shader);

    set_alt_trackball(tex_spot_trackball);
    new RenderRelation(tex_proj_spot, spot_shaft, 10);
  }
}

void event_d(CPT_Event) {
  static bool projtex_shadow_mode = false;

  projtex_shadow_mode = !projtex_shadow_mode;
  if (!projtex_shadow_mode) {
    // disable projtex shadows
    clear_shader(room_arc, proj_shadow);
    set_alt_trackball((Node*)0L);
  } else {
    // enable projtex shadows
    set_shader(room_arc, proj_shadow);
    set_alt_trackball(tex_spot_trackball);
  }
}

void event_h(CPT_Event) {
  static bool highlight_mode = false;

  highlight_mode = !highlight_mode;
  if (!highlight_mode) {
    // disable highlight shader
    clear_shader(ball_arc, highlight);
  } else {
    // enable highlight shader
    set_shader(ball_arc, highlight);
  }
}

void event_e(CPT_Event) {
  static bool spheretex_mode = false;

  spheretex_mode = !spheretex_mode;
  if (!spheretex_mode) {
    // enable spheretex shader
    clear_shader(ball_arc, spheretex);
  } else {
    // disable spheretex shader
    set_shader(ball_arc, spheretex);
  }
}

void event_m(CPT_Event) {
  static bool sphere_reflect_mode = false;

  sphere_reflect_mode = !sphere_reflect_mode;
  if (!sphere_reflect_mode) {
    // disable sphere reflections
    clear_shader(ball_arc, sreflect);
  } else {
    // enable sphere reflections
    set_shader(ball_arc, sreflect);
  }
}

void event_r(CPT_Event) {
  static bool plane_reflect_mode = false;

  plane_reflect_mode = !plane_reflect_mode;
  if (!plane_reflect_mode) {
    // disable planar reflections
    clear_shader(room_arc, preflect);
  } else {
    // enable planar reflections
    set_shader(room_arc, preflect);
  }
}

void event_z(CPT_Event) {
  set_alt_trackball(ball_slider);
}

void event_Z(CPT_Event) {
  follow_ball = !follow_ball;
  if (follow_ball) {
    // hide the ball while following it.
    remove_arc(hide_ball_arc);
  } else {
    // reveal the ball when we're done following it
    hide_ball_arc = new RenderRelation(render, hide_ball);
  }
}

void event_o(CPT_Event) {
  static bool outline_mode = false;

  outline_mode = !outline_mode;
  if (!outline_mode) {
    // disable outline shader
    clear_shader(panda_arc, outline_shader);
  } else {
    // enable outline shader
    set_shader(panda_arc, outline_shader);
  }
}

void load_our_models(void) {
  // load the room
  PT_Node rroom = loader.load_sync("lfloor.egg");
  assert(rroom != (Node*)0L);
  room = new NamedNode("The_room");
  new RenderRelation(room, rroom);
  room_arc = new RenderRelation(root, room);

  // load the ball
  PT_Node rball = loader.load_sync("marble_ball.egg");
  assert(rball != (Node*)0L);
  ball = new NamedNode("scaled_ball");
  PT(RenderRelation) ball_arc1 = new RenderRelation(ball, rball);
  ball_arc1->set_transition
    (new TransformTransition(LMatrix4f::scale_mat(0.2)));
  hide_ball = new NamedNode("hide_ball");
  ball_arc = new RenderRelation(hide_ball, ball);
  ball_arc->set_transition
    (new TransformTransition(LMatrix4f::translate_mat(4., 2., 1.)));
  hide_ball_arc = new RenderRelation(root, hide_ball);

  // load a camera model to visualize our eyepoint
  PT_Node rcamera_model = loader.load_sync("camera.egg");
  assert(rcamera_model != (Node*)0L);
  camera_model = new NamedNode("camera_model");
  new RenderRelation(camera_model, rcamera_model);
  new RenderRelation(cameras, camera_model);

  // load the panda
  PT_Node pmodel = loader.load_sync("panda-3k.egg");
  assert(pmodel != (Node*)0L);
  PT_Node panim = loader.load_sync("panda-walk.egg");
  assert(panim != (Node*)0L);
  PT_NamedNode pparent = new NamedNode("panda_parent");
  new RenderRelation(pparent, pmodel);
  new RenderRelation(pparent, panim);
  PT_NamedNode prot = new NamedNode("panda_rot");
  PT(RenderRelation) p_arc1 = new RenderRelation(prot, pparent);
  p_arc1->set_transition
    (new TransformTransition(LMatrix4f::scale_mat(0.35)));
  panda = new NamedNode("Panda");
  panda_rot_arc = new RenderRelation(panda, prot);
  panda_arc = new RenderRelation(root, panda);
  AnimControlCollection anim_controls;
  auto_bind(pparent, anim_controls, ~0);
  anim_controls.loop_all(true);

  // control the ball using a PlanarSlider tform
  ball_slider = new PlanarSlider("ball_slider");
  ball_slider->set_transform(LMatrix4f::translate_mat(0., 0., 1.) *
                             LMatrix4f::scale_mat(7., -7., 1.));
  ball_slider->set_mouse_pos(LPoint2f(4. / 7., 2. / -7.));
  Transform2SG* slider2ball = new Transform2SG("slider2ball");
  slider2ball->set_arc(ball_arc);
  new RenderRelation(ball_slider, slider2ball);
  follow_ball = false;
}

void setup_shaders(void) {
  // Projected texture shader
  Texture* tex = TexturePool::load_texture("smiley.rgba");
  assert(tex != (Texture*)0L);
  tex_proj = new LensNode("texture_projector");
  RenderRelation* proj_arc = new RenderRelation(root, tex_proj);
  tex_proj_trackball = new Trackball("tex_proj_trackball");
  tex_proj_trackball->set_invert(false);
  tex_proj_trackball->set_rel_to(cameras);
  Transform2SG* tball2cam = new Transform2SG("tball2cam");
  tball2cam->set_arc(proj_arc);
  new RenderRelation(tex_proj_trackball, tball2cam);
  LMatrix4f proj_mat;
  LPoint3f proj_pos = LPoint3f::rfu(2., 3., 8.);
  LVector3f fwd_vec = proj_pos - LPoint3f::origin();
  look_at(proj_mat, -fwd_vec);
  proj_mat.set_row(3, proj_pos);
  tex_proj_trackball->set_mat(proj_mat);
  proj_arc->set_transition(new TransformTransition(proj_mat));
  proj_shader = new ProjtexShader(tex);
  proj_shader->add_frustum(tex_proj);
  PT(Geom) proj_geom =
    tex_proj->get_lens()->make_geometry();
  proj_geom_node = new GeomNode("proj_geometry");
  proj_geom_node->add_geom(proj_geom);
  proj_shader->set_priority(150);

  cerr << "done with projected texture setup" << endl;

  // projected texture spotlight shader
  tex_proj_spot = new Spotlight("tex_proj_spotlight");
  PT(Lens) lens = new PerspectiveLens;
  lens->set_fov(45.0f);
  lens->set_near(f._fnear);
  lens->set_far(13.0f);
  tex_proj_spot->set_lens(lens);
  RenderRelation* spot_arc = new RenderRelation(root, tex_proj_spot, 10);
  tex_spot_trackball = new Trackball("tex_spot_trackball");
  tex_spot_trackball->set_invert(false);
  tex_spot_trackball->set_rel_to(cameras);
  tball2cam = new Transform2SG("tball2cam");
  tball2cam->set_arc(spot_arc);
  new RenderRelation(tex_spot_trackball, tball2cam);
  LMatrix4f spot_mat;
  LPoint3f spot_pos = LPoint3f::rfu(-4., -3., 8.);
  LVector3f spot_vec = spot_pos - LPoint3f::origin();
  look_at(spot_mat, -spot_vec);
  spot_mat.set_row(3, spot_pos);
  tex_spot_trackball->set_mat(spot_mat);
  spot_arc->set_transition(new TransformTransition(spot_mat));
  spot_shader = new SpotlightShader;
  spot_shader->add_frustum(tex_proj_spot);
  spot_shaft = tex_proj_spot->make_geometry(0.05, 8., 36);
  spot_shader->set_priority(150);

  cerr << "done with projected texture spotlight setup" << endl;

  // projected texture shadower
  proj_shadow = new ProjtexShadower;
  proj_shadow->add_frustum(tex_proj_spot);
  proj_shadow->add_caster(panda);
  proj_shadow->add_caster(ball);
  proj_shadow->add_caster(camera_model);
  proj_shadow->set_priority(150);

  cerr << "done with projected texture shadower setup" << endl;

  // sphere texture shader
  spheretex = new SpheretexShader(tex);
  spheretex->set_priority(150);

  cerr << "done with sphere texture shader" << endl;

  // sphere texture highlighter
  highlight = new SpheretexHighlighter;
  highlight->add_frustum(tex_proj_spot);
  highlight->set_priority(150);

  cerr << "done with sphere texture highlighter setup" << endl;

  // sphere texture reflector
  sreflect = new SpheretexReflector;
  sreflect->add_frustum(tex_proj_spot);
  sreflect->add_caster(room);
  sreflect->add_caster(panda);
  sreflect->add_caster(camera_model);
  sreflect->set_priority(150);

  cerr << "done with sphere texture reflector setup" << endl;

  // planar reflector
  Planef p(LVector3f::up(), LPoint3f::origin());
  PlaneNode* plane_node = new PlaneNode;
  plane_node->set_plane(p);
  new RenderRelation(room, plane_node);
  preflect = new PlanarReflector(plane_node);
  preflect->add_caster(ball);
  preflect->add_caster(panda);
  preflect->add_caster(camera_model);
  preflect->set_priority(150);

  cerr << "done with planar reflector setup" << endl;

  // outline shader
  outline_shader = new OutlineShader;
  outline_shader->set_priority(150);

  cerr << "done with outline shader setup" << endl;
}

void panda_keys(EventHandler& eh) {
  new RenderRelation( lights, dlight );
  have_dlight = true;

  eh.add_hook("p", event_p);   // projected texture shader
  eh.add_hook("s", event_s);   // projected texture spotlight shader
  eh.add_hook("d", event_d);   // projected texture shadow shader
  eh.add_hook("h", event_h);   // fake phong highlight
  eh.add_hook("e", event_e);   // sphere texture shader
  eh.add_hook("m", event_m);   // sphere reflection shader
  eh.add_hook("r", event_r);   // planar reflection shader
  eh.add_hook("o", event_o);   // outline shader

  eh.add_hook("z", event_z);   // user controls the ball
  eh.add_hook("Z", event_Z);   // follow the ball around

  load_our_models();
  setup_shaders();
}

int main(int argc, char *argv[]) {
  define_keys = &panda_keys;
  additional_idle = &panda_idle;
  extra_overrides_func = &panda_overrides_func;
  return framework_main(argc, argv);
}
