/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file baseParticleRenderer.h
 * @author charles
 * @date 2000-06-20
 */

#ifndef BASEPARTICLERENDERER_H
#define BASEPARTICLERENDERER_H

#include "pandabase.h"
#include "referenceCount.h"
#include "physicsObject.h"
#include "renderState.h"
#include "geomNode.h"
#include "colorBlendAttrib.h"
#include "nodePath.h"
#include "particleCommonFuncs.h"
#include "baseParticle.h"

#include "pvector.h"

/**
 * Pure virtual particle renderer base class
 */
class EXPCL_PANDA_PARTICLESYSTEM BaseParticleRenderer : public ReferenceCount {
PUBLISHED:
  enum ParticleRendererAlphaMode {
    PR_ALPHA_NONE,
    PR_ALPHA_OUT,
    PR_ALPHA_IN,
    PR_ALPHA_IN_OUT,
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
  INLINE NodePath get_render_node_path() const;

  INLINE void set_alpha_mode(ParticleRendererAlphaMode am);
  INLINE ParticleRendererAlphaMode get_alpha_mode() const;

  INLINE void set_user_alpha(PN_stdfloat ua);
  INLINE PN_stdfloat get_user_alpha() const;

  INLINE void set_color_blend_mode(ColorBlendAttrib::Mode bm, ColorBlendAttrib::Operand oa = ColorBlendAttrib::O_zero, ColorBlendAttrib::Operand ob = ColorBlendAttrib::O_zero);

  void set_ignore_scale(bool ignore_scale);
  INLINE bool get_ignore_scale() const;

  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out, int indent=0) const;

public:
  virtual BaseParticleRenderer *make_copy() = 0;

protected:
  ParticleRendererAlphaMode _alpha_mode;

  BaseParticleRenderer(ParticleRendererAlphaMode alpha_decay = PR_ALPHA_NONE);
  BaseParticleRenderer(const BaseParticleRenderer& copy);

  void update_alpha_mode(ParticleRendererAlphaMode am);

  void enable_alpha();
  void disable_alpha();

  INLINE PN_stdfloat get_cur_alpha(BaseParticle* bp);

  virtual void resize_pool(int new_size) = 0;

  CPT(RenderState) _render_state;

private:
  PT(GeomNode) _render_node;
  NodePath _render_node_path;

  PN_stdfloat _user_alpha;
  bool _ignore_scale;

  // birth and kill particle are for renderers that might do maintenance
  // faster if it was notified on a per-event basis.  An example:
  // geomParticleRenderer maintains an arc for every particle.  Instead of
  // visiting EVERY entry in the arc array, individual arcs are changed on
  // birth and death.  Brings it down a little from O(N) every update.

  virtual void birth_particle(int index) = 0;
  virtual void kill_particle(int index) = 0;


  virtual void init_geoms() = 0;
  virtual void render(pvector< PT(PhysicsObject) >& po_vector,
                      int ttl_particles) = 0;

  friend class ParticleSystem;
};

#include "baseParticleRenderer.I"

#endif // BASEPARTICLERENDERER_H
