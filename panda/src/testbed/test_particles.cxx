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
#include <notify.h>
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
#include <geomParticleRenderer.h>
#include <spriteParticleRenderer.h>
#include <colorTransition.h>
#include <particleSystemManager.h>
#include <clockObject.h>
#include <nodePath.h>
#include <pointShapeTransition.h>
#include <texture.h>
#include <texturePool.h>
#include <physicalNode.h>
#include <forceNode.h>
#include <linearEulerIntegrator.h>

// physics.  particle systems.

PhysicsManager physics_manager;
ParticleSystemManager ps_manager;

PT(ParticleSystem) smoke_system = new ParticleSystem(768);
PT(ZSpinParticleFactory) zpf = new ZSpinParticleFactory;
PT(SpriteParticleRenderer) spr = new SpriteParticleRenderer;
PT(DiscEmitter) de = new DiscEmitter;

PT(Texture) smoke;

PT(LinearVectorForce) wind_force = new LinearVectorForce(0.5, 0, 0);
PT(LinearNoiseForce) noise_force = new LinearNoiseForce(0.02f);

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
  // renderer setup
  spr->set_texture(smoke);
  spr->set_animation_flags(true, true, true);
  spr->set_x_ratios(0.02f, 0.4f);
  spr->set_y_ratios(0.02f, 0.4f);
  spr->set_alpha_decay(PR_ALPHA_OUT);

  // factory setup
  zpf->set_lifespan_base(5.0f);
  zpf->set_mass_base(1.0f);
  zpf->set_mass_delta(0.25f);
  zpf->set_initial_theta(0.0f);
  zpf->set_final_theta(0.0f);
  zpf->set_theta_delta(90.0f);

  // emitter setup
  de->set_radius(0.2f);
  de->set_outer_aoe(70.0f);
  de->set_inner_aoe(90.0f);
  de->set_inner_magnitude(3.0f);
  de->set_outer_magnitude(2.5f);

  // system setup
  smoke_system->set_birth_rate(0.15f);
  smoke_system->set_litter_size(2);
  smoke_system->set_emitter(de);
  smoke_system->set_renderer(spr);
  smoke_system->set_factory(zpf);

  smoke_system->set_render_parent(render);

  pn->add_physical(smoke_system);
  new RenderRelation(render, pn);

  fn->add_force(wind_force);
  fn->add_force(noise_force);
  new RenderRelation(render, fn);

  physics_manager.attach_linear_integrator(new LinearEulerIntegrator);
  physics_manager.attach_physical(smoke_system);
  physics_manager.add_linear_force(wind_force);

  ps_manager.attach_particlesystem(smoke_system);

  smoke_system->add_linear_force(noise_force);

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

  smoke = TexturePool::load_texture("smoke.rgba");

  return framework_main(argc, argv);
}
