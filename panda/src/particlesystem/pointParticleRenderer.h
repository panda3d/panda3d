/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pointParticleRenderer.h
 * @author charles
 * @date 2000-06-20
 */

#ifndef POINTPARTICLERENDERER_H
#define POINTPARTICLERENDERER_H

#include "baseParticleRenderer.h"
#include "baseParticle.h"
#include "renderModeAttrib.h"
#include "pointerTo.h"
#include "pointerToArray.h"
#include "luse.h"
#include "geom.h"
#include "geomVertexData.h"
#include "geomPoints.h"
#include "pStatCollector.h"

/**
 * Simple point/point particle renderer.  Does NOT handle billboards- use
 * BillboardParticleRenderer for that.
 */

class EXPCL_PANDA_PARTICLESYSTEM PointParticleRenderer : public BaseParticleRenderer {
PUBLISHED:
  enum PointParticleBlendType {
    PP_ONE_COLOR,
    PP_BLEND_LIFE,
    PP_BLEND_VEL
  };

  PointParticleRenderer(const PointParticleRenderer& copy);
  explicit PointParticleRenderer(ParticleRendererAlphaMode ad = PR_ALPHA_NONE,
                                 PN_stdfloat point_size = 1.0f,
                                 PointParticleBlendType bt = PP_ONE_COLOR,
                                 ParticleRendererBlendMethod bm = PP_NO_BLEND,
                                 const LColor& sc = LColor(1.0f, 1.0f, 1.0f, 1.0f),
                                 const LColor& ec = LColor(1.0f, 1.0f, 1.0f, 1.0f));

  virtual ~PointParticleRenderer();

public:
  virtual BaseParticleRenderer *make_copy();

PUBLISHED:
  INLINE void set_point_size(PN_stdfloat point_size);
  INLINE void set_start_color(const LColor& sc);
  INLINE void set_end_color(const LColor& ec);
  INLINE void set_blend_type(PointParticleBlendType bt);
  INLINE void set_blend_method(ParticleRendererBlendMethod bm);

  INLINE PN_stdfloat get_point_size() const;
  INLINE const LColor& get_start_color() const;
  INLINE const LColor& get_end_color() const;
  INLINE PointParticleBlendType get_blend_type() const;
  INLINE ParticleRendererBlendMethod get_blend_method() const;

  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out, int indent_level = 0) const;

private:
  LColor _start_color;
  LColor _end_color;
  PN_stdfloat _point_size;
  CPT(RenderAttrib) _thick;

  PT(Geom) _point_primitive;
  PT(GeomPoints) _points;
  PT(GeomVertexData) _vdata;

  int _max_pool_size;

  PointParticleBlendType _blend_type;
  ParticleRendererBlendMethod _blend_method;

  LPoint3 _aabb_min;
  LPoint3 _aabb_max;

  LColor create_color(const BaseParticle *p);

  virtual void birth_particle(int index);
  virtual void kill_particle(int index);
  virtual void init_geoms();
  virtual void render(pvector< PT(PhysicsObject) >& po_vector,
                      int ttl_particles);
  virtual void resize_pool(int new_size);

  static PStatCollector _render_collector;
};

#include "pointParticleRenderer.I"

#endif // POINTPARTICLERENDERER_H
