// Filename: pointParticleRenderer.h
// Created by:  charles (20Jun00)
// 
////////////////////////////////////////////////////////////////////

#ifndef POINTPARTICLERENDERER_H
#define POINTPARTICLERENDERER_H

#include "baseParticleRenderer.h"
#include "baseParticle.h"

#include <pointerTo.h>
#include <pointerToArray.h>
#include <luse.h>
#include <geom.h>
#include <geomPoint.h>

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

  virtual ~PointParticleRenderer(void);

  virtual BaseParticleRenderer *make_copy(void);

  INLINE void set_point_size(float point_size);
  INLINE void set_start_color(const Colorf& sc);
  INLINE void set_end_color(const Colorf& ec);
  INLINE void set_blend_type(PointParticleBlendType bt);
  INLINE void set_blend_method(ParticleRendererBlendMethod bm);

  INLINE float get_point_size(void) const;
  INLINE const Colorf& get_start_color(void) const;
  INLINE const Colorf& get_end_color(void) const;
  INLINE PointParticleBlendType get_blend_type(void) const;
  INLINE ParticleRendererBlendMethod get_blend_method(void) const;

private:
  Colorf _start_color, _end_color;
  float _point_size;

  PT(GeomPoint) _point_primitive;

  PTA_Vertexf _vertex_array;
  PTA_Colorf _color_array;

  int _max_pool_size;

  PointParticleBlendType _blend_type;
  ParticleRendererBlendMethod _blend_method;

  LPoint3f _aabb_min, _aabb_max;

  Colorf create_color(const BaseParticle *p);

  virtual void birth_particle(int index);
  virtual void kill_particle(int index);
  virtual void init_geoms(void);
  virtual void render(vector< PT(PhysicsObject) >& po_vector, 
                      int ttl_particles);
  virtual void resize_pool(int new_size);
};

#include "pointParticleRenderer.I"

#endif // POINTPARTICLERENDERER_H
