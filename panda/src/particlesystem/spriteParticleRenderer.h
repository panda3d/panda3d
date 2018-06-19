/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spriteParticleRenderer.h
 * @author charles
 * @date 2000-07-13
 */

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
#include "vector_int.h"
#include "pStatCollector.h"

class NodePath;

/**
 * Helper class used by SpriteParticleRenderer to keep track of the various
 * GeomVertexWriters associated with each geom created in
 * SpriteParticleRenderer::init_geoms().
 */
class SpriteWriter {
public:
  SpriteWriter() {
  }
  SpriteWriter(const SpriteWriter &copy):
    vertex(copy.vertex),
    color(copy.color),
    rotate(copy.rotate),
    size(copy.size),
    aspect_ratio(copy.aspect_ratio) {
  };

  void clear() {
    vertex.clear();
    color.clear();
    rotate.clear();
    size.clear();
    aspect_ratio.clear();
  }

  GeomVertexWriter vertex;
  GeomVertexWriter color;
  GeomVertexWriter rotate;
  GeomVertexWriter size;
  GeomVertexWriter aspect_ratio;
};

/**
 * Helper class used by SpriteParticleRenderer to keep track of its textures
 * and their respective UVs and source types.
 */
class SpriteAnim : public ReferenceCount{
PUBLISHED:
  enum SourceType {
    ST_texture,
    ST_from_node,
  };

  void set_source_info(const std::string &tex) {
    _source_type = ST_texture;
    _source_tex = tex;
  }

  void set_source_info(const std::string &model, const std::string &node) {
    _source_type = ST_from_node;
    _source_model = model;
    _source_node = node;
  }

  SourceType get_source_type() const {
    return _source_type;
  }

  std::string get_tex_source() const {
    return _source_tex;
  }

  std::string get_model_source() const {
    return _source_model;
  }

  std::string get_node_source() const {
    return _source_node;
  }

  int get_num_frames() const {
    return textures.size();
  }

public:
  SpriteAnim(Texture* t, LTexCoord ll, LTexCoord ur) {
    textures.push_back(t);
    this->ll.push_back(ll);
    this->ur.push_back(ur);
  };

  SpriteAnim(const TextureCollection &t, const pvector< LTexCoord > &lls, const pvector< LTexCoord > &urs) :
    ll(lls),
    ur(urs) {
    for (int i = 0; i < t.get_num_textures(); ++i) {
      textures.push_back(t.get_texture(i));
    }
  };

  void set_ll(const int n, LTexCoord c) {
    ll[n] = c;
  }

  void set_ur(const int n, LTexCoord c) {
    ur[n] = c;
  }

  Texture *get_frame(const int n) const {
    return textures[n];
  };

  LTexCoord get_ll(const int n) const {
    return ll[n];
  }

  LTexCoord get_ur(const int n) const {
    return ur[n];
  }

private:
  pvector< PT(Texture) > textures;
  pvector< LTexCoord > ll,ur;
  SourceType _source_type;
  std::string _source_tex,_source_model,_source_node;
};

/**
 * Renders a particle system with high-speed nasty trick sprites.
 */
class EXPCL_PANDA_PARTICLESYSTEM SpriteParticleRenderer : public BaseParticleRenderer {
PUBLISHED:
  explicit SpriteParticleRenderer(Texture *tex = nullptr);
  SpriteParticleRenderer(const SpriteParticleRenderer &copy);
  virtual ~SpriteParticleRenderer();

public:
  virtual BaseParticleRenderer *make_copy();

PUBLISHED:
  void set_from_node(const NodePath &node_path, bool size_from_texels = false);
  void set_from_node(const NodePath &node_path, const std::string &model, const std::string &node, bool size_from_texels = false);
  void add_from_node(const NodePath &node_path, bool size_from_texels = false, bool resize = false);
  void add_from_node(const NodePath &node_path, const std::string &model, const std::string &node, bool size_from_texels = false, bool resize = false);

  INLINE void set_texture(Texture *tex, PN_stdfloat texels_per_unit = 1.0f);
  INLINE void add_texture(Texture *tex, PN_stdfloat texels_per_unit = 1.0f, bool resize = false);
  INLINE void remove_animation(const int n);
  INLINE void set_ll_uv(const LTexCoord &ll_uv);
  INLINE void set_ll_uv(const LTexCoord &ll_uv, const int anim, const int frame);
  INLINE void set_ur_uv(const LTexCoord &ur_uv);
  INLINE void set_ur_uv(const LTexCoord &ur_uv, const int anim, const int frame);
  INLINE void set_size(PN_stdfloat width, PN_stdfloat height);
  INLINE void set_color(const LColor &color);
  INLINE void set_x_scale_flag(bool animate_x_ratio);
  INLINE void set_y_scale_flag(bool animate_y_ratio);
  INLINE void set_anim_angle_flag(bool animate_theta);
  INLINE void set_initial_x_scale(PN_stdfloat initial_x_scale);
  INLINE void set_final_x_scale(PN_stdfloat final_x_scale);
  INLINE void set_initial_y_scale(PN_stdfloat initial_y_scale);
  INLINE void set_final_y_scale(PN_stdfloat final_y_scale);
  INLINE void set_nonanimated_theta(PN_stdfloat theta);
  INLINE void set_alpha_blend_method(ParticleRendererBlendMethod bm);
  INLINE void set_alpha_disable(bool ad);
  INLINE void set_animate_frames_enable(bool an);
  INLINE void set_animate_frames_rate(PN_stdfloat r);
  INLINE void set_animate_frames_index(int i);

  INLINE Texture *get_texture() const;
  INLINE Texture *get_texture(const int anim, const int frame) const;
  INLINE int get_num_anims() const;
  INLINE SpriteAnim *get_anim(const int n) const;
  MAKE_SEQ(get_anims, get_num_anims, get_anim);
  INLINE SpriteAnim *get_last_anim() const;
  INLINE ColorInterpolationManager* get_color_interpolation_manager() const;
  INLINE LTexCoord get_ll_uv() const;
  INLINE LTexCoord get_ll_uv(const int anim, const int frame) const;
  INLINE LTexCoord get_ur_uv() const;
  INLINE LTexCoord get_ur_uv(const int anim, const int frame) const;
  INLINE PN_stdfloat get_width() const;
  INLINE PN_stdfloat get_height() const;
  INLINE LColor get_color() const;
  INLINE bool get_x_scale_flag() const;
  INLINE bool get_y_scale_flag() const;
  INLINE bool get_anim_angle_flag() const;
  INLINE PN_stdfloat get_initial_x_scale() const;
  INLINE PN_stdfloat get_final_x_scale() const;
  INLINE PN_stdfloat get_initial_y_scale() const;
  INLINE PN_stdfloat get_final_y_scale() const;
  INLINE PN_stdfloat get_nonanimated_theta() const;
  INLINE ParticleRendererBlendMethod get_alpha_blend_method() const;
  INLINE bool get_alpha_disable() const;
  INLINE bool get_animate_frames_enable() const;
  INLINE PN_stdfloat get_animate_frames_rate() const;
  INLINE int get_animate_frames_index() const;

  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out, int indent_level = 0) const;

private:
  pvector< pvector< PT(Geom) > > _sprite_primitive;
  pvector< pvector< PT(GeomPoints) > > _sprites;
  pvector< pvector< SpriteWriter > > _sprite_writer;
  pvector< pvector< PT(GeomVertexData) > > _vdata;

  pvector< PT(SpriteAnim) > _anims;            // Stores texture references and UV info for each geom.

  LColor _color;

  PN_stdfloat _height;
  PN_stdfloat _width;
  PN_stdfloat _initial_x_scale;
  PN_stdfloat _final_x_scale;
  PN_stdfloat _initial_y_scale;
  PN_stdfloat _final_y_scale;
  PN_stdfloat _theta;
  PN_stdfloat _base_y_scale;
  PN_stdfloat _aspect_ratio;
  PN_stdfloat _animate_frames_rate;
  int _animate_frames_index;

  bool _animate_x_ratio;
  bool _animate_y_ratio;
  bool _animate_theta;
  bool _alpha_disable;
  bool _animate_frames;
  bool _animation_removed;

  ParticleRendererBlendMethod _blend_method;
  PT(ColorInterpolationManager) _color_interpolation_manager;

  LVertex _aabb_min;
  LVertex _aabb_max;

  int _pool_size;

  virtual void birth_particle(int index);
  virtual void kill_particle(int index);
  virtual void init_geoms();
  virtual void render(pvector< PT(PhysicsObject) > &po_vector,
                      int ttl_particles);
  virtual void resize_pool(int new_size);
  int extract_textures_from_node(const NodePath &node_path, NodePathCollection &np_col, TextureCollection &tex_col);

  vector_int _anim_size;   // Holds the number of frames in each animation.
  pvector<int*> _ttl_count;  // _ttl_count[i][j] holds the number of particles attached to animation 'i' at frame 'j'.
  vector_int _birth_list;  // Holds the list of particles that need a new random animation to start on.

  static PStatCollector _render_collector;
};

#include "spriteParticleRenderer.I"

#endif // SPRITEPARTICLERENDERER_H
