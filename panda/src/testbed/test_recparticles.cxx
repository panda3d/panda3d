// Filename: test_recparticles.cxx
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
#include <forces.h>
#include <sparkleParticleRenderer.h>
#include <pointParticleRenderer.h>
#include <spriteParticleRenderer.h>
#include <pointParticleFactory.h>
#include <particleSystemManager.h>
#include <clockObject.h>
#include "nodePath.h"
#include <memoryUsage.h>
#include <texture.h>
#include <texturePool.h>

// physics.  particle systems.

PhysicsManager physics_manager;
ParticleSystemManager ps_manager;

static void
event_csn_update(CPT_Event) {
  float dt = ClockObject::get_global_clock()->get_dt();

  physics_manager.do_physics(dt);
  ps_manager.do_particles(dt);
}

PT(ParticleSystem) explosion_system = new ParticleSystem(128);
PT(PointParticleFactory) explosion_factory = new PointParticleFactory;
PT(SpriteParticleRenderer) explosion_renderer = new SpriteParticleRenderer;
PT(SphereSurfaceEmitter) explosion_emitter = new SphereSurfaceEmitter;

PT(ParticleSystem) fireworks_system = new ParticleSystem(32);
PT(PointParticleFactory) fireworks_factory = new PointParticleFactory;
PT(PointParticleRenderer) fireworks_renderer = new PointParticleRenderer;
PT(LineEmitter) fireworks_emitter = new LineEmitter;
PT(Texture) fireworks_texture;

static void
event_add_particles(CPT_Event) {
  // set up the base system
  fireworks_factory->set_lifespan_base(1.5f);
  fireworks_factory->set_lifespan_delta(0.5f);
  fireworks_emitter->set_endpoints(LPoint3f(-2.0f, 0.0f, 0.0f),
                                   LPoint3f(2.0f, 0.0f, 0.0f));
  fireworks_emitter->set_launch_vec(LVector3f(0.0f, 0.0f, 1.0f));
  fireworks_renderer->set_point_size(4.0f);
  fireworks_renderer->set_color1(Colorf(1, 1, 1, 1));
  fireworks_renderer->set_color2(Colorf(1, 0, 0, 1));
  fireworks_renderer->set_blend_type(PP_BLEND_LIFE);
  fireworks_renderer->set_blend_method(PP_BLEND_LINEAR);
  fireworks_renderer->set_alpha_decay(PR_ALPHA_OUT);
  fireworks_system->set_birth_rate(0.25f);
  fireworks_system->set_litter_size(1);
  fireworks_system->set_birth_node(render);
  fireworks_system->set_emitter(fireworks_emitter);
  fireworks_system->set_renderer(fireworks_renderer);
  fireworks_system->set_factory(fireworks_factory);
  fireworks_system->set_render_parent(render);
  fireworks_system->set_spawn_on_death_flag(true);
  fireworks_system->set_spawn_render_node(render);

  // set up the explosion system
  explosion_factory->set_lifespan_base(1.0f);
  explosion_factory->set_lifespan_delta(0.0f);
  explosion_emitter->set_radius(0.1f);
  explosion_emitter->set_amplitude(2.0f);
  explosion_emitter->set_offset_force(LVector3f(2.0f, 0.0f, 2.0f));
  explosion_renderer->set_texture(fireworks_texture);
  explosion_renderer->set_alpha_decay(PR_ALPHA_OUT);
  explosion_renderer->set_color(Colorf(1, 0, 0, 1));
  explosion_system->set_birth_rate(5.0f);
  explosion_system->set_litter_size(64);
  explosion_system->set_template_system_flag(true);
  explosion_system->set_system_grows_older_flag(true);
  explosion_system->set_system_lifespan(1.0f);
  explosion_system->set_emitter(explosion_emitter);
  explosion_system->set_renderer(explosion_renderer);
  explosion_system->set_factory(explosion_factory);

  fireworks_system->add_spawn_template(explosion_system);

  //  fireworks_system->add_force(new VectorForce(0, 0, -9.8f));

  /*
  particle_system->add_spawn_template(explosion_system);
  particle_system->add_spawn_template(explosion_system2);
  particle_system->add_spawn_template(explosion_system3);

  particle_system->set_spawn_on_death_flag(true);
  particle_system->set_birth_node(render);
  particle_system->set_render_parent(render);
  particle_system->set_spawn_render_node(render);
  */

  ps_manager.attach_particlesystem(fireworks_system);
  physics_manager.attach_physical(fireworks_system);

  nout << "Added particles." << endl;
  event_handler.add_hook("NewFrame", event_csn_update);
}

void demo_keys(EventHandler&) {
  event_handler.add_hook("p", event_add_particles);
}

int main(int argc, char *argv[]) {
  fireworks_texture = TexturePool::load_texture("firework.rgba");

  define_keys = &demo_keys;
  return framework_main(argc, argv);
}






