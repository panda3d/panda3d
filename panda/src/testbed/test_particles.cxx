// Filename: test_particles.cxx
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

#include <framework.h>

#include <eventHandler.h>
#include <chancfg.h>
#include <string>
#include <renderModeTransition.h>
#include <colorTransition.h>
#include <cullFaceTransition.h>
#include <depthTestTransition.h>
#include <depthWriteTransition.h>
#include <textureTransition.h>
#include <lightTransition.h>
#include <materialTransition.h>
#include <transformTransition.h>
#include <get_rel_pos.h>
#include <boundingSphere.h>
#include <geomSphere.h>
#include <geomNode.h>
#include "notify.h"
#include <directionalLight.h>
#include <renderRelation.h>
#include <camera.h>
#include <frustum.h>
#include <orthoProjection.h>
#include <perspectiveProjection.h>
#include <textNode.h>
#include <physicsManager.h>
#include <particleSystem.h>
#include <emitters.h>
#include <particlefactories.h>
#include <forces.h>
#include <sparkleParticleRenderer.h>
#include <pointParticleRenderer.h>
#include <lineParticleRenderer.h>
#include <geomParticleRenderer.h>
#include <spriteParticleRenderer.h>
#include <colorTransition.h>
#include <particleSystemManager.h>
#include <clockObject.h>
#include "nodePath.h"
#include <pointShapeTransition.h>
#include <texture.h>
#include <texturePool.h>
#include <physicalNode.h>
#include <forceNode.h>
#include <linearEulerIntegrator.h>

// physics.  particle systems.

/////////////////////////////////////////////////

// particle system manager params
#define PARTICLE_SYSTEM_MANAGER_FRAME_STEPPING 1

// particle system params
#define PARTICLE_SYSTEM_POOL_SIZE 1000
#define PARTICLE_SYSTEM_BIRTH_RATE  0.01f
//#define PARTICLE_SYSTEM_BIRTH_RATE  0.5f
#define PARTICLE_SYSTEM_LITTER_SIZE 10
#define PARTICLE_SYSTEM_LITTER_SPREAD 0


//#define WIND_FORCE  0.5, 0, 0
//#define NOISE_FORCE 0.02f

/////////////////////////////////////////////////

// particle factory selection
#define POINT_PARTICLES
//#define ZSPIN_PARTICLES
//#define ORIENTED_PARTICLES

// particle renderer selection
//#define POINT_PARTICLE_RENDERER
#define LINE_PARTICLE_RENDERER
//#define SPARKLE_PARTICLE_RENDERER
//#define SPRITE_PARTICLE_RENDERER
//#define GEOM_PARTICLE_RENDERER

// particle emitter selection
//#define BOX_EMITTER
//#define DISC_EMITTER
//#define LINE_EMITTER
//#define POINT_EMITTER
//#define RECTANGLE_EMITTER
//#define RING_EMITTER
//#define SPHERE_SURFACE_EMITTER
#define SPHERE_VOLUME_EMITTER
//#define TANGENT_RING_EMITTER

/////////////////////////////////////////////////

// particle factory params
#define PARTICLE_FACTORY_LIFESPAN_BASE   5.0f
//#define PARTICLE_FACTORY_LIFESPAN_BASE   3.0f
//#define PARTICLE_FACTORY_LIFESPAN_SPREAD 1.0f
//#define PARTICLE_FACTORY_MASS_BASE       1.0f
//#define PARTICLE_FACTORY_MASS_SPREAD     0.2f
//#define PARTICLE_FACTORY_TERMINAL_VELOCITY_BASE   PhysicsObject::_default_terminal_velocity
//#define PARTICLE_FACTORY_TERMINAL_VELOCITY_SPREAD 0.0f

#ifdef ZSPIN_PARTICLES
  #define ZSPIN_PARTICLES_INITIAL_ANGLE         0.0f
  #define ZSPIN_PARTICLES_FINAL_ANGLE           0.0f
  #define ZSPIN_PARTICLES_INITIAL_ANGLE_SPREAD  90.0f
  #define ZSPIN_PARTICLES_FINAL_ANGLE_SPREAD    90.0f
#endif

// particle renderer params
#define SPRITE_PARTICLE_RENDERER_ALPHA_DISABLE      false
#define PARTICLE_RENDERER_ALPHA_MODE  BaseParticleRenderer::PR_ALPHA_NONE
//#define PARTICLE_RENDERER_ALPHA_MODE  BaseParticleRenderer::PR_ALPHA_IN
//#define PARTICLE_RENDERER_ALPHA_MODE  BaseParticleRenderer::PR_ALPHA_OUT
//#define PARTICLE_RENDERER_ALPHA_MODE  BaseParticleRenderer::PR_ALPHA_USER
//#define PARTICLE_RENDERER_USER_ALPHA  1.0

#ifdef GEOM_PARTICLE_RENDERER
#elif defined POINT_PARTICLE_RENDERER
  #define POINT_PARTICLE_RENDERER_POINT_SIZE    1.0f
//  #define POINT_PARTICLE_RENDERER_STARTCOLOR    Colorf(1.0f, 0.0f, 0.0f, 1.0f)
//  #define POINT_PARTICLE_RENDERER_ENDCOLOR      Colorf(1.0f, 1.0f, 0.0f, 1.0f)
//  #define POINT_PARTICLE_RENDERER_BLEND_TYPE    BaseParticleRenderer::PP_BLEND_LIFE
//  #define POINT_PARTICLE_RENDERER_BLEND_METHOD  BaseParticleRenderer::PP_BLEND_LINEAR
//  #define POINT_PARTICLE_RENDERER_BLEND_METHOD  BaseParticleRenderer::PP_BLEND_CUBIC

#elif defined LINE_PARTICLE_RENDERER
  #define LINE_PARTICLE_RENDERER_HEAD_COLOR   Colorf(1.0f, 1.0f, 1.0f, 1.0f)
  #define LINE_PARTICLE_RENDERER_TAIL_COLOR   Colorf(1.0f, 1.0f, 1.0f, 1.0f)
#elif defined SPARKLE_PARTICLE_RENDERER
//  #define SPARKLE_PARTICLE_RENDERER_CENTER_COLOR  Colorf(1.0f, 0.0f, 0.0f, 1.0f)
//  #define SPARKLE_PARTICLE_RENDERER_EDGE_COLOR    Colorf(0.0f, 0.0f, 1.0f, 1.0f)
//  #define SPARKLE_PARTICLE_RENDERER_BIRTH_RADIUS  0.004f
//  #define SPARKLE_PARTICLE_RENDERER_DEATH_RADIUS  1.0f
//  #define SPARKLE_PARTICLE_RENDERER_LIFE_SCALE    SP_SCALE
#elif defined SPRITE_PARTICLE_RENDERER
//  #define SPRITE_PARTICLE_RENDERER_TEXTURE_FILE       "smoke.rgba"
//  #define SPRITE_PARTICLE_RENDERER_TEXTURE_FILE       "lilsmiley.rgba"
  #define SPRITE_PARTICLE_RENDERER_TEXTURE_FILE       "rock-floor.rgb"
  #define SPRITE_PARTICLE_RENDERER_COLOR              Colorf(1.0f, 1.0f, 1.0f, 1.0f)
//  #define SPRITE_PARTICLE_RENDERER_X_SCALE_FLAG       true
//  #define SPRITE_PARTICLE_RENDERER_Y_SCALE_FLAG       true
  #define SPRITE_PARTICLE_RENDERER_ANIM_ANGLE_FLAG    true
//  #define SPRITE_PARTICLE_RENDERER_INITIAL_X_SCALE    0.1
//  #define SPRITE_PARTICLE_RENDERER_FINAL_X_SCALE      0.0
//  #define SPRITE_PARTICLE_RENDERER_INITIAL_Y_SCALE    0.0
//  #define SPRITE_PARTICLE_RENDERER_FINAL_Y_SCALE      0.5
  #define SPRITE_PARTICLE_RENDERER_NONANIMATED_THETA  45.0f
  #define SPRITE_PARTICLE_RENDERER_BLEND_METHOD       BaseParticleRenderer::PP_BLEND_LINEAR
//  #define SPRITE_PARTICLE_RENDERER_BLEND_METHOD       BaseParticleRenderer::PP_BLEND_CUBIC
//  #define SPRITE_PARTICLE_RENDERER_ALPHA_DISABLE      false
#endif

// particle emitter params

#define EMISSION_TYPE_EXPLICIT
//#define EMISSION_TYPE_RADIATE
//#define EMISSION_TYPE_CUSTOM

//#define EMITTER_AMPLITUDE 10.0
#define EMITTER_AMPLITUDE 1.0
//#define EMITTER_AMPLITUDE 0.5
//#define EMITTER_AMPLITUDE_SPREAD 2.0
//#define EMITTER_OFFSET_FORCE LVector3f(1.0, 1.0, 1.0)

#if defined EMISSION_TYPE_EXPLICIT
  #define EXPLICIT_LAUNCH_VEC LVector3f(-1.0f, -1.0f, 1.0f)
//  #define EXPLICIT_LAUNCH_VEC LVector3f(0.0f, 0.0f, 0.0f)
#elif defined EMISSION_TYPE_RADIATE
//  #define RADIATE_ORIGIN LPoint3f(0.0f, 0.0f, 0.5f)
  #define RADIATE_ORIGIN LPoint3f(0.0f, 0.0f, 0.0f)
#endif

#ifdef BOX_EMITTER
  #define BOX_EMITTER_MINBOUND LPoint3f(-1.0f, -1.0f, -1.0f)
  #define BOX_EMITTER_MAXBOUND LPoint3f(1.0f, 1.0f, 1.0f)
#elif defined DISC_EMITTER
  #define DISC_EMITTER_RADIUS           1.0f
  #ifdef EMISSION_TYPE_CUSTOM
    #define DISC_EMITTER_OUTER_ANGLE      45.0f
    #define DISC_EMITTER_INNER_ANGLE      90.0f
    #define DISC_EMITTER_OUTER_MAGNITUDE  1.5f
    #define DISC_EMITTER_INNER_MAGNITUDE  1.5f
    #define DISC_EMITTER_CUBIC_LERPING    false
  #endif
#elif defined LINE_EMITTER
  #define LINE_EMITTER_ENDPOINT1 LPoint3f(-2.0f, 0.0f, 2.0f)
  #define LINE_EMITTER_ENDPOINT2 LPoint3f(2.0f, 0.0f, 2.0f)
#elif defined POINT_EMITTER
  #define POINT_EMITTER_LOCATION LPoint3f(1.0f, 1.0f, 1.0f)
#elif defined RECTANGLE_EMITTER
  #define RECTANGLE_EMITTER_MINBOUND LPoint2f(-1.0f, -1.0f)
  #define RECTANGLE_EMITTER_MAXBOUND LPoint2f(1.0f, 1.0f)
#elif defined RING_EMITTER
  #define RING_EMITTER_RADIUS     0.5f
  #ifdef EMISSION_TYPE_CUSTOM
    #define RING_EMITTER_ANGLE      90.0f
  #endif
#elif defined SPHERE_SURFACE_EMITTER
  #define SPHERE_SURFACE_EMITTER_RADIUS 1.0f
#elif defined SPHERE_VOLUME_EMITTER
  #define SPHERE_VOLUME_EMITTER_RADIUS 1.0f
#elif defined TANGENT_RING_EMITTER
  #define TANGENT_RING_EMITTER_RADIUS 1.0f
#endif

/////////////////////////////////////////////////


PhysicsManager physics_manager;
ParticleSystemManager ps_manager;

PT(ParticleSystem) particle_system = new ParticleSystem(PARTICLE_SYSTEM_POOL_SIZE);

static int particles_added = 0;

/*
#if defined POINT_PARTICLES
  PT(PointParticleFactory) pf = new PointParticleFactory;
#elif defined ZSPIN_PARTICLES
  PT(ZSpinParticleFactory) pf = new ZSpinParticleFactory;
#elif defined ORIENTED_PARTICLES
  PT(OrientedParticleFactory) pf = new OrientedParticleFactory;
#endif
*/
#if defined POINT_PARTICLES
  PT(BaseParticleFactory) pf = new PointParticleFactory;
#elif defined ZSPIN_PARTICLES
  PT(BaseParticleFactory) pf = new ZSpinParticleFactory;
#elif defined ORIENTED_PARTICLES
  PT(BaseParticleFactory) pf = new OrientedParticleFactory;
#endif

#if defined GEOM_PARTICLE_RENDERER
  PT(GeomParticleRenderer) pr = new GeomParticleRenderer;
#elif defined POINT_PARTICLE_RENDERER
  PT(PointParticleRenderer) pr = new PointParticleRenderer;
#elif defined LINE_PARTICLE_RENDERER
  PT(LineParticleRenderer) pr = new LineParticleRenderer;
#elif defined SPARKLE_PARTICLE_RENDERER
  PT(SparkleParticleRenderer) pr = new SparkleParticleRenderer;
#elif defined SPRITE_PARTICLE_RENDERER
  PT(SpriteParticleRenderer) pr = new SpriteParticleRenderer;
#endif

#if defined BOX_EMITTER
  PT(BoxEmitter) pe = new BoxEmitter;
#elif defined DISC_EMITTER
  PT(DiscEmitter) pe = new DiscEmitter;
#elif defined LINE_EMITTER
  PT(LineEmitter) pe = new LineEmitter;
#elif defined POINT_EMITTER
  PT(PointEmitter) pe = new PointEmitter;
#elif defined RECTANGLE_EMITTER
  PT(RectangleEmitter) pe = new RectangleEmitter;
#elif defined RING_EMITTER
  PT(RingEmitter) pe = new RingEmitter;
#elif defined SPHERE_SURFACE_EMITTER
  PT(SphereSurfaceEmitter) pe = new SphereSurfaceEmitter;
#elif defined TANGENT_RING_EMITTER
  PT(TangentRingEmitter) pe = new TangentRingEmitter;
#elif defined SPHERE_VOLUME_EMITTER
  PT(SphereVolumeEmitter) pe = new SphereVolumeEmitter;
#endif

PT(Texture) texture;

#ifdef WIND_FORCE
PT(LinearVectorForce) wind_force = new LinearVectorForce(WIND_FORCE);
#endif
#ifdef NOISE_FORCE
PT(LinearNoiseForce) noise_force = new LinearNoiseForce(NOISE_FORCE);
#endif

PT(PhysicalNode) pn = new PhysicalNode;
PT(ForceNode) fn = new ForceNode;

static void
event_csn_update(CPT_Event) {
  float dt = ClockObject::get_global_clock()->get_dt();

  physics_manager.do_physics(dt);
  ps_manager.do_particles(dt);
}

static void
event_add_particles(CPT_Event) {

  // guard against additional "P" presses (bad things happen)
  if(particles_added) return;
  particles_added = 1;

  // renderer setup
  #ifdef PARTICLE_RENDERER_ALPHA_MODE
    pr->set_alpha_mode(PARTICLE_RENDERER_ALPHA_MODE);
  #endif
  #ifdef PARTICLE_RENDERER_USER_ALPHA
    pr->set_user_alpha(PARTICLE_RENDERER_USER_ALPHA);
  #endif

  #ifdef GEOM_PARTICLE_RENDERER
  #elif defined POINT_PARTICLE_RENDERER
    #ifdef POINT_PARTICLE_RENDERER_POINT_SIZE
      pr->set_point_size(POINT_PARTICLE_RENDERER_POINT_SIZE);
    #endif
    #ifdef POINT_PARTICLE_RENDERER_STARTCOLOR
      pr->set_color1(POINT_PARTICLE_RENDERER_STARTCOLOR);
    #endif
    #ifdef POINT_PARTICLE_RENDERER_ENDCOLOR
      pr->set_color2(POINT_PARTICLE_RENDERER_ENDCOLOR);
    #endif
    #ifdef POINT_PARTICLE_RENDERER_BLEND_TYPE
      pr->set_blend_type(POINT_PARTICLE_RENDERER_BLEND_TYPE);
    #endif
    #ifdef POINT_PARTICLE_RENDERER_BLEND_METHOD
      pr->set_blend_method(POINT_PARTICLE_RENDERER_BLEND_METHOD);
    #endif
  #elif defined LINE_PARTICLE_RENDERER
    #ifdef LINE_PARTICLE_RENDERER_HEAD_COLOR
      pr->set_head_color(LINE_PARTICLE_RENDERER_HEAD_COLOR);
    #endif
    #ifdef LINE_PARTICLE_RENDERER_TAIL_COLOR
      pr->set_tail_color(LINE_PARTICLE_RENDERER_TAIL_COLOR);
    #endif
  #elif defined SPARKLE_PARTICLE_RENDERER
    #ifdef SPARKLE_PARTICLE_RENDERER_CENTER_COLOR
      pr->set_center_color(SPARKLE_PARTICLE_RENDERER_CENTER_COLOR);
    #endif
    #ifdef SPARKLE_PARTICLE_RENDERER_EDGE_COLOR
      pr->set_edge_color(SPARKLE_PARTICLE_RENDERER_EDGE_COLOR);
    #endif
    #ifdef SPARKLE_PARTICLE_RENDERER_LIFE_SCALE
      pr->set_life_scale(SPARKLE_PARTICLE_RENDERER_LIFE_SCALE);
    #endif
    #ifdef SPARKLE_PARTICLE_RENDERER_BIRTH_RADIUS
      pr->set_birth_mag(SPARKLE_PARTICLE_RENDERER_BIRTH_RADIUS);
    #endif
    #ifdef SPARKLE_PARTICLE_RENDERER_DEATH_RADIUS
      pr->set_death_mag(SPARKLE_PARTICLE_RENDERER_DEATH_RADIUS);
    #endif
  #elif defined SPRITE_PARTICLE_RENDERER
    pr->set_texture(texture);
    #ifdef SPRITE_PARTICLE_RENDERER_COLOR
      pr->set_color(SPRITE_PARTICLE_RENDERER_COLOR);
    #endif
    #ifdef SPRITE_PARTICLE_RENDERER_X_SCALE_FLAG
      pr->set_x_scale_flag(SPRITE_PARTICLE_RENDERER_X_SCALE_FLAG);
    #endif
    #ifdef SPRITE_PARTICLE_RENDERER_Y_SCALE_FLAG
      pr->set_y_scale_flag(SPRITE_PARTICLE_RENDERER_Y_SCALE_FLAG);
    #endif
    #ifdef SPRITE_PARTICLE_RENDERER_ANIM_ANGLE_FLAG
      pr->set_anim_angle_flag(SPRITE_PARTICLE_RENDERER_ANIM_ANGLE_FLAG);
    #endif
    #ifdef SPRITE_PARTICLE_RENDERER_INITIAL_X_SCALE
      pr->set_initial_x_scale(SPRITE_PARTICLE_RENDERER_INITIAL_X_SCALE);
    #endif
    #ifdef SPRITE_PARTICLE_RENDERER_FINAL_X_SCALE
      pr->set_final_x_scale(SPRITE_PARTICLE_RENDERER_FINAL_X_SCALE);
    #endif
    #ifdef SPRITE_PARTICLE_RENDERER_INITIAL_Y_SCALE
      pr->set_initial_y_scale(SPRITE_PARTICLE_RENDERER_INITIAL_Y_SCALE);
    #endif
    #ifdef SPRITE_PARTICLE_RENDERER_FINAL_Y_SCALE
      pr->set_final_y_scale(SPRITE_PARTICLE_RENDERER_FINAL_Y_SCALE);
    #endif
    #ifdef SPRITE_PARTICLE_RENDERER_NONANIMATED_THETA
      pr->set_nonanimated_theta(SPRITE_PARTICLE_RENDERER_NONANIMATED_THETA);
    #endif
    #ifdef SPRITE_PARTICLE_RENDERER_BLEND_METHOD
      pr->set_alpha_blend_method(SPRITE_PARTICLE_RENDERER_BLEND_METHOD);
    #endif
    #ifdef SPRITE_PARTICLE_RENDERER_ALPHA_DISABLE
      pr->set_alpha_disable(SPRITE_PARTICLE_RENDERER_ALPHA_DISABLE);
    #endif
  #endif

  // factory setup

  #ifdef PARTICLE_FACTORY_LIFESPAN_BASE
    pf->set_lifespan_base(PARTICLE_FACTORY_LIFESPAN_BASE);
  #endif
  #ifdef PARTICLE_FACTORY_LIFESPAN_SPREAD
    pf->set_lifespan_spread(PARTICLE_FACTORY_LIFESPAN_SPREAD);
  #endif
  #ifdef PARTICLE_FACTORY_MASS_BASE
    pf->set_mass_base(PARTICLE_FACTORY_MASS_BASE);
  #endif
  #ifdef PARTICLE_FACTORY_MASS_SPREAD
    pf->set_mass_spread(PARTICLE_FACTORY_MASS_SPREAD);
  #endif
  #ifdef PARTICLE_FACTORY_TERMINAL_VELOCITY_BASE
    pf->set_terminal_velocity_base(PARTICLE_FACTORY_TERMINAL_VELOCITY_BASE);
  #endif
  #ifdef PARTICLE_FACTORY_TERMINAL_VELOCITY_SPREAD
    pf->set_terminal_velocity_spread(PARTICLE_FACTORY_TERMINAL_VELOCITY_SPREAD);
  #endif

  #if defined ZSPIN_PARTICLES
    #ifdef ZSPIN_PARTICLES_INITIAL_ANGLE
      pf->set_initial_angle(ZSPIN_PARTICLES_INITIAL_ANGLE);
    #endif
    #ifdef ZSPIN_PARTICLES_FINAL_ANGLE
      pf->set_final_angle(ZSPIN_PARTICLES_FINAL_ANGLE);
    #endif
    #ifdef ZSPIN_PARTICLES_INITIAL_ANGLE_SPREAD
      pf->set_initial_angle_spread(ZSPIN_PARTICLES_INITIAL_ANGLE_SPREAD);
    #endif
    #ifdef ZSPIN_PARTICLES_FINAL_ANGLE_SPREAD
      pf->set_final_angle_spread(ZSPIN_PARTICLES_FINAL_ANGLE_SPREAD);
    #endif
  #elif defined ORIENTED_PARTICLES
  #endif

  // emitter setup
  #ifdef EMITTER_AMPLITUDE
    pe->set_amplitude(EMITTER_AMPLITUDE);
  #endif
  #ifdef EMITTER_AMPLITUDE_SPREAD
    pe->set_amplitude_spread(EMITTER_AMPLITUDE_SPREAD);
  #endif
  #ifdef EMITTER_OFFSET_FORCE
    pe->set_offset_force(EMITTER_OFFSET_FORCE);
  #endif

  #if defined EMISSION_TYPE_EXPLICIT
    pe->set_emission_type(BaseParticleEmitter::ET_EXPLICIT);
    #ifdef EXPLICIT_LAUNCH_VEC
    pe->set_explicit_launch_vector(EXPLICIT_LAUNCH_VEC);
    #endif
  #elif defined EMISSION_TYPE_RADIATE
    pe->set_emission_type(BaseParticleEmitter::ET_RADIATE);
    #ifdef RADIATE_ORIGIN
      pe->set_radiate_origin(RADIATE_ORIGIN);
    #endif
  #elif defined EMISSION_TYPE_CUSTOM
    pe->set_emission_type(BaseParticleEmitter::ET_CUSTOM);
  #endif

  #ifdef BOX_EMITTER
    #ifdef BOX_EMITTER_MINBOUND
      pe->set_min_bound(BOX_EMITTER_MINBOUND);
    #endif
    #ifdef BOX_EMITTER_MAXBOUND
      pe->set_max_bound(BOX_EMITTER_MAXBOUND);
    #endif
  #elif defined DISC_EMITTER
    #ifdef DISC_EMITTER_RADIUS
      pe->set_radius(DISC_EMITTER_RADIUS);
    #endif
    #ifdef EMISSION_TYPE_CUSTOM
      #ifdef DISC_EMITTER_OUTER_ANGLE
        pe->set_outer_angle(DISC_EMITTER_OUTER_ANGLE);
      #endif
      #ifdef DISC_EMITTER_INNER_ANGLE
        pe->set_inner_angle(DISC_EMITTER_INNER_ANGLE);
      #endif
      #ifdef DISC_EMITTER_OUTER_MAGNITUDE
        pe->set_outer_magnitude(DISC_EMITTER_OUTER_MAGNITUDE);
      #endif
      #ifdef DISC_EMITTER_INNER_MAGNITUDE
        pe->set_inner_magnitude(DISC_EMITTER_INNER_MAGNITUDE);
      #endif
      #ifdef DISC_EMITTER_CUBIC_LERPING
        pe->set_cubic_lerping(DISC_EMITTER_CUBIC_LERPING);
      #endif
    #endif
  #elif defined LINE_EMITTER
    #ifdef LINE_EMITTER_ENDPOINT1
      pe->set_endpoint1(LINE_EMITTER_ENDPOINT1);
    #endif
    #ifdef LINE_EMITTER_ENDPOINT2
      pe->set_endpoint2(LINE_EMITTER_ENDPOINT2);
    #endif
  #elif defined POINT_EMITTER
    #ifdef POINT_EMITTER_LOCATION
      pe->set_location(POINT_EMITTER_LOCATION);
    #endif
  #elif defined RECTANGLE_EMITTER
    #ifdef RECTANGLE_EMITTER_MINBOUND
      pe->set_min_bound(RECTANGLE_EMITTER_MINBOUND);
    #endif
    #ifdef RECTANGLE_EMITTER_MAXBOUND
      pe->set_max_bound(RECTANGLE_EMITTER_MAXBOUND);
    #endif
  #elif defined RING_EMITTER
    #ifdef RING_EMITTER_RADIUS
      pe->set_radius(RING_EMITTER_RADIUS);
    #endif
    #ifdef EMISSION_TYPE_CUSTOM
      #ifdef RING_EMITTER_ANGLE
        pe->set_aoe(RING_EMITTER_ANGLE);
      #endif
    #endif
  #elif defined SPHERE_SURFACE_EMITTER
    #ifdef SPHERE_SURFACE_EMITTER_RADIUS
      pe->set_radius(SPHERE_SURFACE_EMITTER_RADIUS);
    #endif
  #elif defined TANGENT_RING_EMITTER
    #ifdef TANGENT_RING_EMITTER_RADIUS
      pe->set_radius(TANGENT_RING_EMITTER_RADIUS);
    #endif
  #elif defined SPHERE_VOLUME_EMITTER
    #ifdef SPHERE_VOLUME_EMITTER_RADIUS
      pe->set_radius(SPHERE_VOLUME_EMITTER_RADIUS);
    #endif
  #endif // emitter type

  // system setup
  particle_system->set_birth_rate(PARTICLE_SYSTEM_BIRTH_RATE);
  particle_system->set_litter_size(PARTICLE_SYSTEM_LITTER_SIZE);
  particle_system->set_litter_spread(PARTICLE_SYSTEM_LITTER_SPREAD);
  particle_system->set_emitter(pe);
  particle_system->set_renderer(pr);
  particle_system->set_factory(pf);

  particle_system->set_render_parent(render);

  pn->add_physical(particle_system);
  new RenderRelation(render, pn);

#ifdef WIND_FORCE
  fn->add_force(wind_force);
#endif
#ifdef NOISE_FORCE
  fn->add_force(noise_force);
#endif
  new RenderRelation(render, fn);

  physics_manager.attach_linear_integrator(new LinearEulerIntegrator);
  physics_manager.attach_physical(particle_system);
#ifdef WIND_FORCE
  physics_manager.add_linear_force(wind_force);
#endif

#ifdef PARTICLE_SYSTEM_MANAGER_FRAME_STEPPING
  ps_manager.set_frame_stepping(PARTICLE_SYSTEM_MANAGER_FRAME_STEPPING);
#endif
  ps_manager.attach_particlesystem(particle_system);

#ifdef NOISE_FORCE
  particle_system->add_linear_force(noise_force);
#endif

  nout << "Added particles." << endl;
  event_handler.add_hook("NewFrame", event_csn_update);
}

static void set_pool_size(int size) {
  nout << "setting pool size to " << size << endl;
  particle_system->set_pool_size(size);
}

static void
event_more_particles(CPT_Event) {
  static int index = 0;
  static int sizes[] = {
    10,
    999,
    998,
    999,
    1000,
    1001,
    1002,
    1003,
    1004,
    1000,
    16*1000,
    4*1000,
    3*1000,
    2*1000,
    1000,
    0,
  };

  if(!particles_added) return;

  if (0 == sizes[index]) index = 0;
  set_pool_size(sizes[index]);
  index++;
}

static void
event_switch_particle_factory_type(CPT_Event) {
  static int index = 0;

  if(!particles_added) return;

  cout << "Switching to a";

  switch (index) {
    case 0:
      cout << " point";
      pf = new PointParticleFactory;
      particle_system->set_factory(pf);
      break;
    case 1:
      cout << " z-spin";
      pf = new ZSpinParticleFactory;
      particle_system->set_factory(pf);
      break;
    case 2:
      cout << "n oriented";
      pf = new OrientedParticleFactory;
      particle_system->set_factory(pf);
      break;
  }

  cout << " particle factory" << endl;

  index++;
  if (index > 2) index = 0;
}

void demo_keys(EventHandler&) {
  new RenderRelation( lights, dlight );
  have_dlight = true;

  event_handler.add_hook("p", event_add_particles);
  event_handler.add_hook("m", event_more_particles);
  event_handler.add_hook(",", event_switch_particle_factory_type);
}

int main(int argc, char *argv[]) {
  define_keys = &demo_keys;

#ifdef SPRITE_PARTICLE_RENDERER
  texture = TexturePool::load_texture(SPRITE_PARTICLE_RENDERER_TEXTURE_FILE);
#endif

  return framework_main(argc, argv);
}
