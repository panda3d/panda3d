// Filename: baseParticleRenderer.h
// Created by:  charles (20Jun00)
// 
////////////////////////////////////////////////////////////////////

#ifndef BASEPARTICLERENDERER_H
#define BASEPARTICLERENDERER_H

#include <pandabase.h>
#include <referenceCount.h>
#include <physicsObject.h>
#include <geomNode.h>
#include <renderRelation.h>
#include <nodeRelation.h>
#include <transparencyTransition.h>

#include <vector>

enum ParticleRendererAlphaDecay {
  PR_NO_ALPHA,
  PR_ALPHA_OUT,
  PR_ALPHA_IN,
  PR_ALPHA_USER,
  PR_ALPHA_INVALID
};

enum ParticleRendererBlendMethod {
  PP_NO_BLEND,
  PP_BLEND_LINEAR,
  PP_BLEND_CUBIC
};

////////////////////////////////////////////////////////////////////
//       Class : BaseParticleRenderer
// Description : Pure virtual particle renderer base class
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS BaseParticleRenderer : public ReferenceCount {
private:
  PT(GeomNode) _render_node;

  // children of this class don't know anything about alpha.
  // they should just interact with _interface_node, and link geometry
  // from/to that.

  PT(GeomNode) _alpha_node;
  PT(RenderRelation) _alpha_arc;

  // birth and kill particle are for renderers that might do maintenance
  // faster if it was notified on a per-event basis.  An example:
  // geomParticleRenderer maintains an arc for every particle.  Instead
  // of visiting EVERY entry in the arc array, individual arcs are
  // changed on birth and death.  Brings it down a little from O(N) every
  // update.

  virtual void birth_particle(int index) = 0;
  virtual void kill_particle(int index) = 0;


  virtual void init_geoms(void) = 0;
  virtual void render(vector< PT(PhysicsObject) >& po_vector, 
		      int ttl_particles) = 0;

protected:
  GeomNode *_interface_node;
  ParticleRendererAlphaDecay _alpha_decay;

  BaseParticleRenderer(ParticleRendererAlphaDecay alpha_decay);
  BaseParticleRenderer(const BaseParticleRenderer& copy);

  void update_alpha_state(ParticleRendererAlphaDecay ad);

  INLINE Colorf color_lerp(float t, const Colorf& c1, const Colorf& c2);
  INLINE Colorf color_clerp(float t, const Colorf& c1, const Colorf& c2);
  INLINE float cubic_smooth(float t);
  INLINE float lerp(float t, float x0, float x1);

  void enable_alpha(void);
  INLINE void disable_alpha(void);

  virtual void resize_pool(int new_size) = 0;

public:

  virtual ~BaseParticleRenderer(void);

  INLINE GeomNode *get_render_node(void) const;
  INLINE ParticleRendererAlphaDecay get_alpha_decay(void) const;
  INLINE void set_alpha_decay(ParticleRendererAlphaDecay ad);

  virtual BaseParticleRenderer *make_copy(void) = 0;

  friend class ParticleSystem;
};

#include "baseParticleRenderer.I"

#endif // BASEPARTICLERENDERER_H

