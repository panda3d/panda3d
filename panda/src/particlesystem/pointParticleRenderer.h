// Filename: pointParticleRenderer.h
// Created by:  charles (20Jun00)
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

#ifndef POINTPARTICLERENDERER_H
#define POINTPARTICLERENDERER_H

#include "baseParticleRenderer.h"
#include "baseParticle.h"

#include "pointerTo.h"
#include "pointerToArray.h"
#include "luse.h"
#include "geom.h"
#include "geomPoint.h"

////////////////////////////////////////////////////////////////////
//       Class : PointParticleRenderer
// Description : Simple point/point particle renderer.  Does NOT
//               handle billboards- use BillboardParticleRenderer
//               for that.
////////////////////////////////////////////////////////////////////

class EXPCL_PANDAPHYSICS PointParticleRenderer : public BaseParticleRenderer {
PUBLISHED:
  enum PointParticleBlendType {
    PP_ONE_COLOR,
    PP_BLEND_LIFE,
    PP_BLEND_VEL
  };

  PointParticleRenderer(const PointParticleRenderer& copy);
  PointParticleRenderer(ParticleRendererAlphaMode ad = PR_ALPHA_NONE,
                        float point_size = 1.0f,
                        PointParticleBlendType bt = PP_ONE_COLOR,
                        ParticleRendererBlendMethod bm = PP_NO_BLEND,
                        const Colorf& sc = Colorf(1.0f, 1.0f, 1.0f, 1.0f),
                        const Colorf& ec = Colorf(1.0f, 1.0f, 1.0f, 1.0f));

  virtual ~PointParticleRenderer();

  virtual BaseParticleRenderer *make_copy();

  INLINE void set_point_size(float point_size);
  INLINE void set_start_color(const Colorf& sc);
  INLINE void set_end_color(const Colorf& ec);
  INLINE void set_blend_type(PointParticleBlendType bt);
  INLINE void set_blend_method(ParticleRendererBlendMethod bm);

  INLINE float get_point_size() const;
  INLINE const Colorf& get_start_color() const;
  INLINE const Colorf& get_end_color() const;
  INLINE PointParticleBlendType get_blend_type() const;
  INLINE ParticleRendererBlendMethod get_blend_method() const;

  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent=0) const;

private:
  Colorf _start_color;
  Colorf _end_color;
  float _point_size;

  PT(GeomPoint) _point_primitive;

  PTA_Vertexf _vertex_array;
  PTA_Colorf _color_array;

  int _max_pool_size;

  PointParticleBlendType _blend_type;
  ParticleRendererBlendMethod _blend_method;

  LPoint3f _aabb_min;
  LPoint3f _aabb_max;

  Colorf create_color(const BaseParticle *p);

  virtual void birth_particle(int index);
  virtual void kill_particle(int index);
  virtual void init_geoms();
  virtual void render(pvector< PT(PhysicsObject) >& po_vector,
                      int ttl_particles);
  virtual void resize_pool(int new_size);
};

#include "pointParticleRenderer.I"

#endif // POINTPARTICLERENDERER_H
