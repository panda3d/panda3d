/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file sparkleParticleRenderer.h
 * @author charles
 * @date 2000-06-27
 */

#ifndef SPARKLEPARTICLERENDERER_H
#define SPARKLEPARTICLERENDERER_H

#include "baseParticle.h"
#include "baseParticleRenderer.h"
#include "pointerTo.h"
#include "pointerToArray.h"
#include "geom.h"
#include "geomVertexData.h"
#include "geomLines.h"
#include "pStatCollector.h"

enum SparkleParticleLifeScale {
  SP_NO_SCALE,
  SP_SCALE
};

/**
 * pretty sparkly things.
 */
class EXPCL_PANDA_PARTICLESYSTEM SparkleParticleRenderer : public BaseParticleRenderer {
PUBLISHED:
  enum SparkleParticleLifeScale {
    SP_NO_SCALE,
    SP_SCALE
  };

  SparkleParticleRenderer();
  SparkleParticleRenderer(const SparkleParticleRenderer& copy);
  explicit SparkleParticleRenderer(const LColor& center,
                                   const LColor& edge,
                                   PN_stdfloat birth_radius,
                                   PN_stdfloat death_radius,
                                   SparkleParticleLifeScale life_scale,
                                   ParticleRendererAlphaMode alpha_mode);

  virtual ~SparkleParticleRenderer();

public:
  virtual BaseParticleRenderer *make_copy();

PUBLISHED:
  INLINE void set_center_color(const LColor& c);
  INLINE void set_edge_color(const LColor& c);
  INLINE void set_birth_radius(PN_stdfloat radius);
  INLINE void set_death_radius(PN_stdfloat radius);
  INLINE void set_life_scale(SparkleParticleLifeScale);

  INLINE const LColor& get_center_color() const;
  INLINE const LColor& get_edge_color() const;
  INLINE PN_stdfloat get_birth_radius() const;
  INLINE PN_stdfloat get_death_radius() const;
  INLINE SparkleParticleLifeScale get_life_scale() const;

  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out, int indent_level = 0) const;

private:
  LColor _center_color;
  LColor _edge_color;

  PN_stdfloat _birth_radius;
  PN_stdfloat _death_radius;

  PT(Geom) _line_primitive;
  PT(GeomLines) _lines;
  PT(GeomVertexData) _vdata;

  int _max_pool_size;

  SparkleParticleLifeScale _life_scale;
  LPoint3 _aabb_min;
  LPoint3 _aabb_max;

  INLINE PN_stdfloat get_radius(BaseParticle *bp);

  virtual void birth_particle(int index);
  virtual void kill_particle(int index);
  virtual void init_geoms();
  virtual void render(pvector< PT(PhysicsObject) >& po_vector,
                      int ttl_particles);
  virtual void resize_pool(int new_size);

  static PStatCollector _render_collector;
};

#include "sparkleParticleRenderer.I"

#endif // SPARKLEPARTICLERENDERER_H
