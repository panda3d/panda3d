// Filename: sparkleParticleRenderer.h
// Created by:  charles (27Jun00)
// 
////////////////////////////////////////////////////////////////////

#ifndef SPARKLEPARTICLERENDERER_H
#define SPARKLEPARTICLERENDERER_H

#include "baseParticle.h"
#include "baseParticleRenderer.h"

#include <pointerTo.h>
#include <pointerToArray.h>
#include <geom.h>
#include <geomLine.h>

enum SparkleParticleLifeScale {
  SP_NO_SCALE,
  SP_SCALE
};

////////////////////////////////////////////////////////////////////
//       Class : SparkleParticleRenderer
// Description : pretty sparkly things.
////////////////////////////////////////////////////////////////////

class EXPCL_PANDAPHYSICS SparkleParticleRenderer : public BaseParticleRenderer {
private:

  Colorf _center_color;
  Colorf _edge_color;

  float _birth_mag;
  float _death_mag;

  PT(GeomLine) _line_primitive;

  PTA_Vertexf _vertex_array;
  PTA_Colorf _color_array;

  int _max_pool_size;

  SparkleParticleLifeScale _life_scale;
  LPoint3f _aabb_min, _aabb_max;

  INLINE float get_magnitude(BaseParticle *bp);

  virtual void birth_particle(int index);
  virtual void kill_particle(int index);
  virtual void init_geoms(void);
  virtual void render(vector< PT(PhysicsObject) >& po_vector,
		      int ttl_particles);
  virtual void resize_pool(int new_size);

public:

  SparkleParticleRenderer(void);
  SparkleParticleRenderer(const SparkleParticleRenderer& copy);
  SparkleParticleRenderer(const Colorf& center,
			  const Colorf& edge,
			  float birth_mag,
			  float death_mag,
			  SparkleParticleLifeScale life_scale,
			  ParticleRendererAlphaDecay alpha_decay);

  virtual ~SparkleParticleRenderer(void);

  virtual BaseParticleRenderer *make_copy(void);

  INLINE void set_center_color(const Colorf& c);
  INLINE void set_edge_color(const Colorf& c);
  INLINE void set_life_scale(SparkleParticleLifeScale);
  INLINE void set_birth_mag(float mag);
  INLINE void set_death_mag(float mag);

  INLINE const Colorf& get_center_color(void) const;
  INLINE const Colorf& get_edge_color(void) const;
  INLINE SparkleParticleLifeScale get_life_scale(void) const;
  INLINE float get_birth_mag(void) const;
  INLINE float get_death_mag(void) const;
};

#include "sparkleParticleRenderer.I"

#endif // SPARKLEPARTICLERENDERER_H
