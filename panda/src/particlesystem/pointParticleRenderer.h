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

enum PointParticleBlendType {
  PP_ONE_COLOR,
  PP_BLEND_LIFE,
  PP_BLEND_VEL
};

////////////////////////////////////////////////////////////////////
//       Class : PointParticleRenderer
// Description : Simple point/point particle renderer.  Does NOT
//               handle billboards- use BillboardParticleRenderer
//               for that.
////////////////////////////////////////////////////////////////////

class EXPCL_PANDAPHYSICS PointParticleRenderer : public BaseParticleRenderer {
private:

  Colorf _color1, _color2;
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

public:

  PointParticleRenderer(const PointParticleRenderer& copy);
  PointParticleRenderer(ParticleRendererAlphaDecay ad = PR_NO_ALPHA,
			float point_size = 1.0f,
			PointParticleBlendType bt = PP_ONE_COLOR,
			ParticleRendererBlendMethod bm = PP_NO_BLEND,
			const Colorf& c1 = Colorf(1.0f, 1.0f, 1.0f, 1.0f),
			const Colorf& c2 = Colorf(1.0f, 1.0f, 1.0f, 1.0f));

  virtual ~PointParticleRenderer(void);

  virtual BaseParticleRenderer *make_copy(void);

  INLINE void set_point_size(float point_size);
  INLINE void set_color1(const Colorf& c1);
  INLINE void set_color2(const Colorf& c2);
  INLINE void set_blend_type(PointParticleBlendType bt);
  INLINE void set_blend_method(ParticleRendererBlendMethod bm);

  INLINE float get_point_size(void) const;
  INLINE const Colorf& get_color1(void) const;
  INLINE const Colorf& get_color2(void) const;
  INLINE PointParticleBlendType get_blend_type(void) const;
  INLINE ParticleRendererBlendMethod get_blend_method(void) const;
};

#include "pointParticleRenderer.I"

#endif // POINTPARTICLERENDERER_H
