// Filename: spriteParticleRenderer.h
// Created by:  charles (13Jul00)
// 
////////////////////////////////////////////////////////////////////

#ifndef SPRITEPARTICLERENDERER_H
#define SPRITEPARTICLERENDERER_H

#include <pandabase.h>
#include <texture.h>
#include <pointerTo.h>
#include <pointerToArray.h>
#include <pta_float.h>
#include <geom.h>
#include <geomSprite.h>

#include "baseParticleRenderer.h"
#include "baseParticle.h"

////////////////////////////////////////////////////////////////////
//       Class : SpriteParticleRenderer
// Description : Renders a particle system with high-speed nasty
//               trick sprites.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS SpriteParticleRenderer : public BaseParticleRenderer {
private:
  PT(GeomSprite) _sprite_primitive;
  PTA_Vertexf _vertex_array;
  PTA_Colorf _color_array;
  PTA_float _x_texel_array;
  PTA_float _y_texel_array;
  PTA_float _theta_array;

  Colorf _color;

  float _initial_x_texel_ratio, _final_x_texel_ratio;
  float _initial_y_texel_ratio, _final_y_texel_ratio;
  float _theta;

  bool _animate_x_ratio, _animate_y_ratio;
  bool _animate_theta;

  ParticleRendererBlendMethod _blend_method;

  Vertexf _aabb_min, _aabb_max;

  int _pool_size;

  virtual void birth_particle(int index);
  virtual void kill_particle(int index);
  virtual void init_geoms(void);
  virtual void render(vector< PT(PhysicsObject) > &po_vector,
		      int ttl_particles);
  virtual void resize_pool(int new_size);

public:
  SpriteParticleRenderer(Texture *tex = (Texture *) NULL);
  SpriteParticleRenderer(const SpriteParticleRenderer &copy);
  virtual ~SpriteParticleRenderer(void);

  virtual BaseParticleRenderer *make_copy(void);

  INLINE void set_texture(Texture *tex);
  INLINE Texture *get_texture(void) const;

  INLINE void set_color(const Colorf &color);

  // this is the main interface for whether or not you want the sizes
  // to change over the course of the particle's lifetime.
  INLINE void set_animation_flags(bool animate_x_ratio,
				  bool animate_y_ratio,
				  bool animate_theta);

  INLINE void set_x_ratios(float initial_x_texel_ratio,
			   float final_x_texel_ratio = 0.0f);
  INLINE void set_y_ratios(float initial_y_texel_ratio,
			   float final_y_texel_ratio = 0.0f);
  INLINE void set_nonanimated_theta(float theta);

  // alpha
  INLINE void set_blend_type(ParticleRendererBlendMethod bm);

  INLINE void set_alpha_disable(bool ad);
  INLINE bool get_alpha_disable(void) const;
};

#include "spriteParticleRenderer.I"

#endif // SPRITEPARTICLERENDERER_H
