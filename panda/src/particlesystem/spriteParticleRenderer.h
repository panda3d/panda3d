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

PUBLISHED:
  SpriteParticleRenderer(Texture *tex = (Texture *) NULL);
  SpriteParticleRenderer(const SpriteParticleRenderer &copy);
  virtual ~SpriteParticleRenderer(void);

  virtual BaseParticleRenderer *make_copy(void);

  INLINE void set_texture(Texture *tex);
  INLINE void set_color(const Colorf &color);
  INLINE void set_x_scale_flag(bool animate_x_ratio);
  INLINE void set_y_scale_flag(bool animate_y_ratio);
  INLINE void set_anim_angle_flag(bool animate_theta);
  INLINE void set_initial_x_scale(float initial_x_scale);
  INLINE void set_final_x_scale(float final_x_scale);
  INLINE void set_initial_y_scale(float initial_y_scale);
  INLINE void set_final_y_scale(float final_y_scale);
  INLINE void set_nonanimated_theta(float theta);
  INLINE void set_alpha_blend_method(ParticleRendererBlendMethod bm);
  INLINE void set_alpha_disable(bool ad);

  INLINE Texture *get_texture(void) const;
  INLINE Colorf get_color(void) const;
  INLINE bool get_x_scale_flag(void) const;
  INLINE bool get_y_scale_flag(void) const;
  INLINE bool get_anim_angle_flag(void) const;
  INLINE float get_initial_x_scale(void) const;
  INLINE float get_final_x_scale(void) const;
  INLINE float get_initial_y_scale(void) const;
  INLINE float get_final_y_scale(void) const;
  INLINE float get_nonanimated_theta(void) const;
  INLINE ParticleRendererBlendMethod get_alpha_blend_method(void) const;
  INLINE bool get_alpha_disable(void) const;
};

#include "spriteParticleRenderer.I"

#endif // SPRITEPARTICLERENDERER_H
