// Filename: spriteParticleRenderer.h
// Created by:  charles (13Jul00)
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

#ifndef SPRITEPARTICLERENDERER_H
#define SPRITEPARTICLERENDERER_H

#include "pandabase.h"
#include "pvector.h"
#include "baseParticleRenderer.h"
#include "baseParticle.h"
#include "texture.h"
#include "pointerTo.h"
#include "geom.h"
#include "geomVertexData.h"
#include "geomPoints.h"
#include "colorInterpolationManager.h"
#include "geomVertexWriter.h"
#include "textureCollection.h"
#include "nodePathCollection.h"

class NodePath;

class SpriteWriter {
public:
  SpriteWriter(GeomVertexWriter v,
               GeomVertexWriter c,
               GeomVertexWriter r,
               GeomVertexWriter s,
               GeomVertexWriter a):
    vertex(v),
    color(c),
    rotate(r),
    size(s),
    aspect_ratio(a){
    };

  SpriteWriter(const SpriteWriter &copy):
    vertex(copy.vertex),
    color(copy.color),
    rotate(copy.rotate),
    size(copy.size),
    aspect_ratio(copy.aspect_ratio) {
  };

  GeomVertexWriter vertex;
  GeomVertexWriter color;
  GeomVertexWriter rotate;
  GeomVertexWriter size;
  GeomVertexWriter aspect_ratio;
};

class SpriteAnim : public ReferenceCount{
PUBLISHED:
  enum SourceType {
    ST_texture,
    ST_from_node,
  };

  void set_source_info(const string &tex) {
    _source_type = ST_texture;
    _source_tex = tex;
  }

  void set_source_info(const string &model, const string &node) {
    _source_type = ST_from_node;
    _source_model = model;
    _source_node = node;
  }

  SourceType get_source_type() const {
    return _source_type;
  }

  string get_tex_source() const {
    return _source_tex;
  }

  string get_model_source() const {
    return _source_model;
  }

  string get_node_source() const {
    return _source_node;
  }

  int get_num_frames(void) const {
    return textures.size();
  }

public:
  SpriteAnim(Texture* t, TexCoordf ll, TexCoordf ur) {
    textures.push_back(t);
    this->ll.push_back(ll);
    this->ur.push_back(ur);
  };

  SpriteAnim(const TextureCollection &t, const pvector< TexCoordf > &lls, const pvector< TexCoordf > &urs) :
    ll(lls),
    ur(urs) {
    for (int i = 0; i < t.get_num_textures(); ++i) {
      textures.push_back(t.get_texture(i));
    }
  };
  
  void set_ll(const int n, TexCoordf c) {
    ll[n] = c;
  }

  void set_ur(const int n, TexCoordf c) {
    ur[n] = c;
  }

  Texture *get_frame(const int n) const {
    return textures[n];
  };

  TexCoordf get_ll(const int n) const {
    return ll[n];
  }

  TexCoordf get_ur(const int n) const {
    return ur[n];
  }

private:
  pvector< PT(Texture) > textures;
  pvector< TexCoordf > ll,ur;
  SourceType _source_type;
  string _source_tex,_source_model,_source_node;
};

////////////////////////////////////////////////////////////////////
//       Class : SpriteParticleRenderer
// Description : Renders a particle system with high-speed nasty
//               trick sprites.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS SpriteParticleRenderer : public BaseParticleRenderer {
PUBLISHED:
  SpriteParticleRenderer(Texture *tex = (Texture *) NULL);
  SpriteParticleRenderer(const SpriteParticleRenderer &copy);
  virtual ~SpriteParticleRenderer();

  virtual BaseParticleRenderer *make_copy();

  void set_from_node(const NodePath &node_path, bool size_from_texels = true);
  void set_from_node(const NodePath &node_path, const string &model, const string &node, bool size_from_texels = true);
  void add_from_node(const NodePath &node_path, bool size_from_texels = true, bool resize = false);
  void add_from_node(const NodePath &node_path, const string &model, const string &node, bool size_from_texels = true, bool resize = false);

  INLINE void set_texture(Texture *tex, float texels_per_unit = 1.0f);
  INLINE void set_texture(Texture *tex, const string &tex_path, float texels_per_unit = 1.0f);
  INLINE void add_texture(Texture *tex, float texels_per_unit = 1.0f, bool resize = false);
  INLINE void add_texture(Texture *tex, const string &tex_path, float texels_per_unit = 1.0f, bool resize = false);
  INLINE void remove_animation(const int n);
  INLINE void set_ll_uv(const TexCoordf &ll_uv);
  INLINE void set_ll_uv(const TexCoordf &ll_uv, const int anim, const int frame);
  INLINE void set_ur_uv(const TexCoordf &ur_uv);
  INLINE void set_ur_uv(const TexCoordf &ur_uv, const int anim, const int frame);
  INLINE void set_size(float width, float height);
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
  INLINE void set_animate_frames_enable(bool an);
  INLINE void set_animate_frames_rate(float r);
  INLINE void set_animate_frames_index(int i);

  INLINE Texture *get_texture() const;
  INLINE Texture *get_texture(const int anim, const int frame) const;
  INLINE int get_num_anims() const;
  INLINE SpriteAnim *get_anim(const int n) const;
  INLINE SpriteAnim *get_last_anim() const;
  INLINE ColorInterpolationManager* get_color_interpolation_manager() const;
  INLINE TexCoordf get_ll_uv() const;
  INLINE TexCoordf get_ll_uv(const int anim, const int frame) const;
  INLINE TexCoordf get_ur_uv() const;
  INLINE TexCoordf get_ur_uv(const int anim, const int frame) const;
  INLINE float get_width() const;
  INLINE float get_height() const;
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
  INLINE bool get_animate_frames_enable() const;  
  INLINE float get_animate_frames_rate() const;
  INLINE int get_animate_frames_index() const;

  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent_level = 0) const;

private:
  pvector< pvector< PT(Geom) > > _sprite_primitive;
  pvector< pvector< PT(GeomPoints) > > _sprites;
  pvector< pvector< SpriteWriter > > _sprite_writer;
  pvector< pvector< PT(GeomVertexData) > > _vdata;

  pvector< PT(SpriteAnim) > _anims;            // Stores texture references and UV info for each geom.

  Colorf _color;

  float _height;
  float _width;
  float _initial_x_scale;
  float _final_x_scale;
  float _initial_y_scale;
  float _final_y_scale;
  float _theta;
  float _base_y_scale;
  float _aspect_ratio;
  float _animate_frames_rate;
  int _animate_frames_index;

  bool _animate_x_ratio;
  bool _animate_y_ratio;
  bool _animate_theta;
  bool _alpha_disable;
  bool _animate_frames;
  bool _animation_removed;

  ParticleRendererBlendMethod _blend_method;
  PT(ColorInterpolationManager) _color_interpolation_manager;

  Vertexf _aabb_min;
  Vertexf _aabb_max;

  int _pool_size;

  virtual void birth_particle(int index);
  virtual void kill_particle(int index);
  virtual void init_geoms();
  virtual void render(pvector< PT(PhysicsObject) > &po_vector,
                      int ttl_particles);
  virtual void resize_pool(int new_size);
  int extract_textures_from_node(const NodePath &node_path, NodePathCollection &np_col, TextureCollection &tex_col);

  pvector<int> _anim_size;   // Holds the number of frames in each animation.
  pvector<int*> _ttl_count;  // _ttl_count[i][j] holds the number of particles attached to animation 'i' at frame 'j'.
  pvector<int> _birth_list;  // Holds the list of particles that need a new random animation to start on.

};

#include "spriteParticleRenderer.I"

#endif // SPRITEPARTICLERENDERER_H
