// Filename: test_sprite_particles.cxx
// Created by:  
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

#include "framework.h"

#include "eventHandler.h"
#include "chancfg.h"
#include <string>
#include "renderModeTransition.h"
#include "colorTransition.h"
#include "cullFaceTransition.h"
#include "depthTestTransition.h"
#include "depthWriteTransition.h"
#include "textureTransition.h"
#include "lightTransition.h"
#include "materialTransition.h"
#include "transformTransition.h"
#include "get_rel_pos.h"
#include "boundingSphere.h"
#include "geomSphere.h"
#include "geomNode.h"
#include "notify.h"
#include "directionalLight.h"
#include "renderRelation.h"
#include "camera.h"
#include "frustum.h"
#include "orthoProjection.h"
#include "perspectiveProjection.h"
#include "textNode.h"
#include "physicsManager.h"
#include "particleSystem.h"
#include "emitters.h"
#include "particlefactories.h"
#include "forces.h"
#include "sparkleParticleRenderer.h"
#include "pointParticleRenderer.h"
#include "lineParticleRenderer.h"
#include "geomParticleRenderer.h"
#include "spriteParticleRenderer.h"
#include "colorTransition.h"
#include "particleSystemManager.h"
#include "clockObject.h"
#include "nodePath.h"
#include "pointShapeTransition.h"
#include "texture.h"
#include "texturePool.h"
#include "physicalNode.h"
#include "forceNode.h"
#include "linearEulerIntegrator.h"

// physics.  particle systems.

/////////////////////////////////////////////////

// particle system manager params
#define PARTICLE_SYSTEM_MANAGER_FRAME_STEPPING 1

// particle system params
#define PARTICLE_SYSTEM_POOL_SIZE 1024
#define PARTICLE_SYSTEM_BIRTH_RATE  0.02f
#define PARTICLE_SYSTEM_LITTER_SIZE 10
#define PARTICLE_SYSTEM_LITTER_SPREAD 0


/////////////////////////////////////////////////

// particle factory selection
#define POINT_PARTICLES

// particle renderer selection
#define POINT_PARTICLE_RENDERER
//#define SPRITE_PARTICLE_RENDERER

// particle emitter selection
#define SPHERE_VOLUME_EMITTER

/////////////////////////////////////////////////

// particle factory params
#define PARTICLE_FACTORY_LIFESPAN_BASE   0.5f

#define SPRITE_PARTICLE_RENDERER_ALPHA_DISABLE      false
#define PARTICLE_RENDERER_ALPHA_MODE  BaseParticleRenderer::PR_ALPHA_NONE
//#define PARTICLE_RENDERER_ALPHA_MODE  BaseParticleRenderer::PR_ALPHA_IN
//#define PARTICLE_RENDERER_ALPHA_MODE  BaseParticleRenderer::PR_ALPHA_OUT
//#define PARTICLE_RENDERER_ALPHA_MODE  BaseParticleRenderer::PR_ALPHA_USER
//#define PARTICLE_RENDERER_USER_ALPHA  1.0

#if defined POINT_PARTICLE_RENDERER
  #define POINT_PARTICLE_RENDERER_POINT_SIZE    1.0f
#elif defined SPRITE_PARTICLE_RENDERER
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
  #define SPRITE_PARTICLE_RENDERER_BLEND_METHOD        BaseParticleRenderer::PP_BLEND_LINEAR
//  #define SPRITE_PARTICLE_RENDERER_BLEND_METHOD      BaseParticleRenderer::PP_BLEND_CUBIC
//  #define SPRITE_PARTICLE_RENDERER_ALPHA_DISABLE     false
#endif

// particle emitter params

#define EMISSION_TYPE_RADIATE
//#define EMISSION_TYPE_EXPLICIT

#define EMITTER_AMPLITUDE 1.0

#if defined EMISSION_TYPE_EXPLICIT
  #define EXPLICIT_LAUNCH_VEC LVector3f(-1.0f, -1.0f, 1.0f)
//  #define EXPLICIT_LAUNCH_VEC LVector3f(0.0f, 0.0f, 0.0f)
#elif defined EMISSION_TYPE_RADIATE
  #define RADIATE_ORIGIN LPoint3f(0.0f, 0.0f, 0.0f)
#endif

#if defined SPHERE_VOLUME_EMITTER
  #define SPHERE_VOLUME_EMITTER_RADIUS 1.0f
#endif

/////////////////////////////////////////////////


PhysicsManager physics_manager;
ParticleSystemManager ps_manager;

PT(ParticleSystem) particle_system = new ParticleSystem(1024);
PT(PointParticleFactory) pf = new PointParticleFactory;
PT(SphereVolumeEmitter) pe = new SphereVolumeEmitter;

#if defined POINT_PARTICLE_RENDERER
  PT(PointParticleRenderer) pr = new PointParticleRenderer;
#elif defined SPRITE_PARTICLE_RENDERER
  PT(SpriteParticleRenderer) pr = new SpriteParticleRenderer;
#endif

PT(Texture) texture;

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
  static initialized = 0;

  // guard against additional "P" presses (bad things happen)
  if(initialized) return;
  initialized = 1;

  // renderer setup
  pr->set_alpha_mode(BaseParticleRenderer::PR_ALPHA_USER);
  pr->set_user_alpha(1.0);

  #if defined POINT_PARTICLE_RENDERER
    pr->set_point_size(1.0);
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
      //pr->set_alpha_blend_method(SPRITE_PARTICLE_RENDERER_BLEND_METHOD);
    #endif
    #define SPRITE_PARTICLE_RENDERER_ALPHA_DISABLE      false
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
  #endif

  pe->set_radius(SPHERE_VOLUME_EMITTER_RADIUS);

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

  new RenderRelation(render, fn);

  physics_manager.attach_linear_integrator(new LinearEulerIntegrator);
  physics_manager.attach_physical(particle_system);

#ifdef PARTICLE_SYSTEM_MANAGER_FRAME_STEPPING
  ps_manager.set_frame_stepping(PARTICLE_SYSTEM_MANAGER_FRAME_STEPPING);
#endif
  ps_manager.attach_particlesystem(particle_system);

  nout << "Added particles." << endl;
  event_handler.add_hook("NewFrame", event_csn_update);
}

void demo_keys(EventHandler&) {
  new RenderRelation( lights, dlight );
  have_dlight = true;

  event_handler.add_hook("p", event_add_particles);
}

int main(int argc, char *argv[]) {
  define_keys = &demo_keys;

#ifdef SPRITE_PARTICLE_RENDERER
  texture = TexturePool::load_texture(SPRITE_PARTICLE_RENDERER_TEXTURE_FILE);
#endif

  return framework_main(argc, argv);
}
