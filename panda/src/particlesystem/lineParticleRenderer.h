/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lineParticleRenderer.h
 * @author darren
 * @date 2000-10-06
 */

#ifndef LINEPARTICLERENDERER_H
#define LINEPARTICLERENDERER_H

#include "baseParticle.h"
#include "baseParticleRenderer.h"
#include "pointerTo.h"
#include "pointerToArray.h"
#include "geom.h"
#include "geomVertexData.h"
#include "geomLines.h"
#include "pStatCollector.h"

/**
 * renders a line from last position to current position -- good for rain,
 * sparks, etc.
 */

class EXPCL_PANDA_PARTICLESYSTEM LineParticleRenderer : public BaseParticleRenderer {
PUBLISHED:
  LineParticleRenderer();
  LineParticleRenderer(const LineParticleRenderer& copy);
  explicit LineParticleRenderer(const LColor& head,
                                const LColor& tail,
                                ParticleRendererAlphaMode alpha_mode);

  virtual ~LineParticleRenderer();

public:
  virtual BaseParticleRenderer *make_copy();

PUBLISHED:
  INLINE void set_head_color(const LColor& c);
  INLINE void set_tail_color(const LColor& c);

  INLINE const LColor& get_head_color() const;
  INLINE const LColor& get_tail_color() const;

  INLINE void set_line_scale_factor(PN_stdfloat sf);
  INLINE PN_stdfloat get_line_scale_factor() const;

  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out, int indent_level = 0) const;

private:
  LColor _head_color;
  LColor _tail_color;

  PT(Geom) _line_primitive;
  PT(GeomLines) _lines;
  PT(GeomVertexData) _vdata;

  int _max_pool_size;

  LPoint3 _aabb_min;
  LPoint3 _aabb_max;

  PN_stdfloat _line_scale_factor;

  virtual void birth_particle(int index);
  virtual void kill_particle(int index);
  virtual void init_geoms();
  virtual void render(pvector< PT(PhysicsObject) >& po_vector,
                      int ttl_particles);
  virtual void resize_pool(int new_size);

  static PStatCollector _render_collector;
};

#include "lineParticleRenderer.I"

#endif // LINEPARTICLERENDERER_H
