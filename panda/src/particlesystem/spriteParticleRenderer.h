// Filename: spriteParticleRenderer.h
// Created by:  charles (13Jul00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef SPRITEPARTICLERENDERER_H
#define SPRITEPARTICLERENDERER_H

#include "pandabase.h"
#include "texture.h"
#include "pointerTo.h"
#include "pointerToArray.h"
#include "pta_float.h"
#include "geom.h"
#include "geomSprite.h"

#include "baseParticleRenderer.h"
#include "baseParticle.h"

class NodePath;

////////////////////////////////////////////////////////////////////
//       Class : SpriteParticleRenderer
// Description : Renders a particle system with high-speed nasty
//               trick sprites.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS SpriteParticleRenderer : public BaseParticleRenderer {
PUBLISHED:
  // This enumerated type indicates the source of the sprite texture:
  // whether it came from an explicit call to set_texture(), or
  // whether from a call to set_from_node().
  enum SourceType {
    ST_texture,
    ST_from_node,
  };

  SpriteParticleRenderer(Texture *tex = (Texture *) NULL);
  SpriteParticleRenderer(const SpriteParticleRenderer &copy);
  virtual ~SpriteParticleRenderer();

  virtual BaseParticleRenderer *make_copy();

  INLINE SourceType get_source_type() const;

  void set_from_node(const NodePath &node_path);

  INLINE void set_texture(Texture *tex);
  INLINE void set_ll_uv(const TexCoordf &ll_uv);
  INLINE void set_ur_uv(const TexCoordf &ur_uv);
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

  INLINE Texture *get_texture() const;
  INLINE const TexCoordf &get_ll_uv() const;
  INLINE const TexCoordf &get_ur_uv() const;
  INLINE Colorf get_color() const;
  INLINE bool get_x_scale_flag() const;
  INLINE bool get_y_scale_flag() const;
  INLINE bool get_anim_angle_flag() const;
  INLINE float get_initial_x_scale() const;
  INLINE float get_final_x_scale() const;
  INLINE float get_initial_y_scale() const;
  INLINE float get_final_y_scale() const;
  INLINE float get_nonanimated_theta() const;
  INLINE ParticleRendererBlendMethod get_alpha_blend_method() const;
  INLINE bool get_alpha_disable() const;

  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent=0) const;

private:
  PT(GeomSprite) _sprite_primitive;
  PTA_Vertexf _vertex_array;
  PTA_Colorf _color_array;
  PTA_float _x_texel_array;
  PTA_float _y_texel_array;
  PTA_float _theta_array;

  Colorf _color;

  float _initial_x_texel_ratio;
  float _final_x_texel_ratio;
  float _initial_y_texel_ratio;
  float _final_y_texel_ratio;
  float _theta;

  bool _animate_x_ratio;
  bool _animate_y_ratio;
  bool _animate_theta;

  ParticleRendererBlendMethod _blend_method;

  Vertexf _aabb_min;
  Vertexf _aabb_max;

  int _pool_size;
  SourceType _source_type;

  virtual void birth_particle(int index);
  virtual void kill_particle(int index);
  virtual void init_geoms();
  virtual void render(pvector< PT(PhysicsObject) > &po_vector,
                      int ttl_particles);
  virtual void resize_pool(int new_size);
};

#include "spriteParticleRenderer.I"

#endif // SPRITEPARTICLERENDERER_H
