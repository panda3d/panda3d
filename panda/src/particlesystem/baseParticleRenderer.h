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

#include "particleCommonFuncs.h"
#include "baseParticle.h"

#include <vector>

enum ParticleRendererAlphaMode {
  PR_ALPHA_NONE,
  PR_ALPHA_OUT,
  PR_ALPHA_IN,
  PR_ALPHA_USER,
  PR_NOT_INITIALIZED_YET
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

  float _user_alpha;

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

  ParticleRendererAlphaMode _alpha_mode;

  BaseParticleRenderer(ParticleRendererAlphaMode alpha_decay = PR_ALPHA_NONE);
  BaseParticleRenderer(const BaseParticleRenderer& copy);

  void update_alpha_mode(ParticleRendererAlphaMode am);

  void enable_alpha(void);
  INLINE void disable_alpha(void);

  INLINE float get_cur_alpha(BaseParticle* bp);

  virtual void resize_pool(int new_size) = 0;

public:

  virtual ~BaseParticleRenderer(void);

  INLINE GeomNode *get_render_node(void) const;

  INLINE void set_alpha_mode(ParticleRendererAlphaMode am);
  INLINE ParticleRendererAlphaMode get_alpha_mode(void) const;

  INLINE void set_user_alpha(float ua);
  INLINE float get_user_alpha(void) const;

  virtual BaseParticleRenderer *make_copy(void) = 0;

  friend class ParticleSystem;
};

#include "baseParticleRenderer.I"

#endif // BASEPARTICLERENDERER_H

