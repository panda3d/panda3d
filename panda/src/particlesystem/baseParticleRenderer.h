// Filename: baseParticleRenderer.h
// Created by:  charles (20Jun00)
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

#ifndef BASEPARTICLERENDERER_H
#define BASEPARTICLERENDERER_H

#include "pandabase.h"
#include "referenceCount.h"
#include "physicsObject.h"
#include "renderState.h"
#include "geomNode.h"

#include "particleCommonFuncs.h"
#include "baseParticle.h"

#include "pvector.h"

////////////////////////////////////////////////////////////////////
//       Class : BaseParticleRenderer
// Description : Pure virtual particle renderer base class
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS BaseParticleRenderer : public ReferenceCount {
PUBLISHED:
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

  virtual ~BaseParticleRenderer();

  INLINE GeomNode *get_render_node() const;

  INLINE void set_alpha_mode(ParticleRendererAlphaMode am);
  INLINE ParticleRendererAlphaMode get_alpha_mode() const;

  INLINE void set_user_alpha(float ua);
  INLINE float get_user_alpha(void) const;
  
  virtual void output(ostream &out, unsigned int indent=0) const;

protected:
  ParticleRendererAlphaMode _alpha_mode;

  BaseParticleRenderer(ParticleRendererAlphaMode alpha_decay = PR_ALPHA_NONE);
  BaseParticleRenderer(const BaseParticleRenderer& copy);

  void update_alpha_mode(ParticleRendererAlphaMode am);

  void enable_alpha();
  void disable_alpha();

  INLINE float get_cur_alpha(BaseParticle* bp);

  virtual void resize_pool(int new_size) = 0;

  CPT(RenderState) _render_state;

private:
  PT(GeomNode) _render_node;

  float _user_alpha;

  // birth and kill particle are for renderers that might do maintenance
  // faster if it was notified on a per-event basis.  An example:
  // geomParticleRenderer maintains an arc for every particle.  Instead
  // of visiting EVERY entry in the arc array, individual arcs are
  // changed on birth and death.  Brings it down a little from O(N) every
  // update.

  virtual void birth_particle(int index) = 0;
  virtual void kill_particle(int index) = 0;


  virtual void init_geoms() = 0;
  virtual void render(pvector< PT(PhysicsObject) >& po_vector,
                      int ttl_particles) = 0;

public:
  virtual BaseParticleRenderer *make_copy() = 0;

  friend class ParticleSystem;
};

#include "baseParticleRenderer.I"

#endif // BASEPARTICLERENDERER_H

